/*
 * Simple Task Queue
 * Copyright (c) 2022 fdar0536
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <cstring>
#include <new>

#include "sys/types.h"
#include "sys/wait.h"

#include "nixprocess.hpp"

#include "global.hpp"

NixProcess::NixProcess() :
    m_pid(0)
{}

NixProcess::~NixProcess()
{
    resetImpl();
}

uint8_t NixProcess::init(AbstractProcess *in)
{
    if (!in) return 1;

    NixProcess *process = reinterpret_cast<NixProcess *>(in);
    memset(process->m_fd, 0, 2);
    return 0;
}

void NixProcess::reset()
{
    resetImpl();
}

bool NixProcess::isRunning()
{
    int state(0);
    return !waitpid(-1, &state, WNOHANG);
}

uint8_t NixProcess::start(const char *name,
                           const char *workDir,
                           std::vector<std::string> &args)
{
    if (!name || !workDir) return 1;

    m_pid = fork();
    if (m_pid == -1)
    {
        // parent process
        sprintf(m_error, "%s:%d %s: %s", __FILE__, __LINE__, "fork", strerror(errno));
        Global::logger.write(Logger::Error, m_error);
        return 1;
    }

    if (pipe(m_fd) == -1)
    {
        if (m_pid == 0)
        {
            // child process
            perror("pipe");
            exit(1);
        }

        sprintf(m_error, "%s:%d %s: %s", __FILE__, __LINE__, "pipe", strerror(errno));
        Global::logger.write(Logger::Error, m_error);
        return 1;
    }

    if (m_pid == 0)
    {
        // child process
        while ((dup2(m_fd[1], STDOUT_FILENO) == -1) && (errno == EINTR)) {}

        close(m_fd[1]);
        close(m_fd[0]);

        if (chdir(workDir) == -1)
        {
            perror("chdir");
            exit(1);
        }

        char **argv(nullptr);
        if (args.size())
        {
            argv = new (std::nothrow) char*[args.size() + 2](); // process name + null for tail
        }
        else
        {
            argv = new (std::nothrow) char*[2]();
        }

        if (!argv)
        {
            fprintf(stderr, "Fail to allocate child process' argv.");
            exit(1);
        }

        size_t nameLen = strlen(name) + 1;
        argv[0] = new (std::nothrow) char[nameLen]();
        if (!argv[0])
        {
            delete[] argv;
            fprintf(stderr, "Fail to allocate child process' argv[0].");
            exit(1);
        }

        memcpy(argv[0], name, nameLen);
        if (args.size())
        {
            size_t index(0);
            for (size_t i = 0; i < args.size(); ++i)
            {
                index = i + 1;
                argv[index] = new (std::nothrow) char[args[i].size() + 1](); // +1 for '\0'
                if (!argv[index])
                {
                    for (size_t j = 0; j < index; ++j)
                    {
                        delete[] argv[j];
                    }

                    delete[] argv;
                    fprintf(stderr, "Fail to allocate child process' argv[%zu].", index);
                    exit(1);
                }

                memcpy(argv[index], args[i].c_str(), args[i].size());
                argv[index][args[i].size()] = '\0';
            }

            argv[args.size() + 1] = NULL;
        }
        else
        {
            argv[1] = NULL;
        }

        // start child
        char *env[] = { NULL };
        execve(name, argv, env);
        perror("execve");   /* execve() returns only on error */
        exit(1);
    }

    close(m_fd[1]);
    if (setpgid(m_pid, 0) == -1)
    {
        sprintf(m_error, "%s:%d %s: %s", __FILE__, __LINE__, "setpgid", strerror(errno));
        Global::logger.write(Logger::Error, m_error);
        kill(m_pid, SIGKILL);
        return 1;
    }

    return 0;
}

uint8_t NixProcess::stop()
{
    return stopImpl();
}

uint8_t NixProcess::readStdOut(char *buf, size_t *bufSize)
{
    if (!buf || !bufSize)
    {
        sprintf(m_error, "%s:%d %s", __FILE__, __LINE__, "Invalid input.");
        Global::logger.write(Logger::Error, m_error);
        return 1;
    }

    if (!(*bufSize))
    {
        sprintf(m_error, "%s:%d %s", __FILE__, __LINE__, "Invalid buffer size.");
        Global::logger.write(Logger::Error, m_error);
        return 1;
    }

    ssize_t count;
    while (1)
    {
        count = read(m_fd[0], buf, *bufSize);
        if (count == -1)
        {
            if (errno == EINTR) continue;
            else
            {
                sprintf(m_error, "%s:%d %s: %s", __FILE__, __LINE__, "read", strerror(errno));
                Global::logger.write(Logger::Error, m_error);
                *bufSize = 0;
                buf[0] = '\0';
                return 1;
            }
        }
        else if (count == 0)
        {
            *bufSize = 0;
            buf[0] = '\0';
        }
        else
        {
            *bufSize = count;
        }

        return 0;
    }
}

AbstractProcess::ExitState NixProcess::exitCode(int *code)
{
    if (!code) return AbstractProcess::ExitState::Failed;
    if (isRunning()) return AbstractProcess::ExitState::Failed;

    int status = 0;
    if (waitpid(m_pid, &status, WNOHANG) == -1)
    {
        sprintf(m_error, "%s:%d %s: %s", __FILE__, __LINE__, "waitpid", strerror(errno));
        Global::logger.write(Logger::Error, m_error);
        return AbstractProcess::ExitState::Failed;
    }

    if (WIFEXITED(status))
    {
        *code = WEXITSTATUS(status);
        return AbstractProcess::ExitState::NormalExit;
    }

    if (WIFSIGNALED(status))
    {
        return AbstractProcess::ExitState::NonNormalExit;
    }

    return AbstractProcess::ExitState::Failed;
}

// private memebr functions
void NixProcess::resetImpl()
{
    stopImpl();
    close(m_fd[0]);
    close(m_fd[1]);
    memset(m_fd, 0, 2);
    m_pid = 0;
}

uint8_t NixProcess::stopImpl()
{
    if (kill(m_pid * -1, SIGKILL) == -1)
    {
        sprintf(m_error, "%s:%d %s: %s", __FILE__, __LINE__, "kill", strerror(errno));
        Global::logger.write(Logger::Error, m_error);
        return 1;
    }

    return 0;
}