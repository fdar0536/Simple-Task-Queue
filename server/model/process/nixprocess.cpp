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

#include <new>

#include <cstdio>
#include <cstring>

#include "sys/types.h"
#include "sys/wait.h"
#include "fcntl.h"

#include "nixprocess.hpp"

NixProcess::NixProcess() :
    m_pid(0),
    m_isStop(false),
    m_exitCode(0)
{
    m_logger = nullptr;
}

NixProcess::~NixProcess()
{
    resetImpl();
}

uint8_t NixProcess::init(AbstractProcess *in, Logger *logger)
{
    if (!in || !logger) return 1;

    NixProcess *process = reinterpret_cast<NixProcess *>(in);
    memset(process->m_fd, 0, 2 * sizeof(int));
    process->m_logger = logger;
    return 0;
}

void NixProcess::reset()
{
    resetImpl();
}

bool NixProcess::isRunning()
{
    int state(0);

    pid_t pid = waitpid(m_pid, &state, WNOHANG);
    if (pid)
    {
        // failed
        m_isStop = !(errno == EAGAIN);
        if (m_isStop) m_exitCode = state;

        return !m_isStop;
    }

    return true;
}

uint8_t NixProcess::start(const char *name,
                           const char *workDir,
                           std::vector<std::string> &args)
{
    if (!name || !workDir) return 1;

    if (pipe(m_fd) == -1)
    {
        m_error.clear();
        m_error = __FILE__":" + std::to_string(__LINE__);
        m_error += " pipe: ";
        m_error += strerror(errno);
        m_logger->write(Logger::Error, m_error.c_str());
        return 1;
    }

    m_pid = fork();
    if (m_pid == -1)
    {
        // parent process
        m_error.clear();
        m_error = __FILE__":" + std::to_string(__LINE__);
        m_error += " fork: ";
        m_error += strerror(errno);

        m_logger->write(Logger::Error, m_error.c_str());
        return 1;
    }

    if (m_pid == 0)
    {
        // child process
        while ((dup2(m_fd[1], STDOUT_FILENO) == -1) && (errno == EINTR)) {}
        while ((dup2(STDOUT_FILENO, STDERR_FILENO) == -1) && (errno == EINTR)) {}
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

    if (setpgid(m_pid, 0) == -1)
    {
        m_error.clear();
        m_error = __FILE__":" + std::to_string(__LINE__);
        m_error += " setpgid: ";
        m_error += strerror(errno);
        m_logger->write(Logger::Error, m_error.c_str());
        kill(m_pid, SIGKILL);
        return 1;
    }

    close(m_fd[1]);
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
        m_error.clear();
        m_error = __FILE__":" + std::to_string(__LINE__);
        m_error += " Invalid input.";
        m_logger->write(Logger::Error, m_error.c_str());
        return 1;
    }

    if (!(*bufSize))
    {
        m_error.clear();
        m_error = __FILE__":" + std::to_string(__LINE__);
        m_error += " Invalid buffer size.";
        m_logger->write(Logger::Error, m_error.c_str());
        return 1;
    }

    ssize_t count(0);
    while (1)
    {
        count = read(m_fd[0], buf, *bufSize);
        if (count == -1)
        {
            if (errno == EINTR) continue;
            else
            {
                m_error.clear();
                m_error = __FILE__":" + std::to_string(__LINE__);
                m_error += " read: ";
                m_error += strerror(errno);
                m_logger->write(Logger::Error, m_error.c_str());

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

    if (WIFEXITED(m_exitCode))
    {
        *code = WEXITSTATUS(m_exitCode);
        return AbstractProcess::ExitState::NormalExit;
    }

    if (WIFSIGNALED(m_exitCode))
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
    memset(m_fd, 0, 2 * sizeof(int));
    m_pid = 0;
    m_isStop = 0;
    m_exitCode = 0;
}

uint8_t NixProcess::stopImpl()
{
    if (kill(m_pid * -1, SIGKILL) == -1)
    {
        m_error.clear();
        m_error = __FILE__":" + std::to_string(__LINE__);
        m_error += " kill: ";
        m_error += strerror(errno);
        m_logger->write(Logger::Error, m_error.c_str());
        return 1;
    }

    sleep(1);
    return 0;
}
