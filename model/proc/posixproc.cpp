/*
 * Simple Task Queue
 * Copyright (c) 2023 fdar0536
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

#include <string.h>

#include "sys/types.h"
#include "sys/wait.h"
#include "fcntl.h"

#include "spdlog/spdlog.h"

#include "posixproc.hpp"

namespace Model
{

namespace Proc
{

PosixProc::PosixProc() :
    m_pid(0)
{}

PosixProc::~PosixProc()
{}

uint_fast8_t PosixProc::init()
{
    memset(m_fd, 0, 2 * sizeof(int));
    m_exitCode.store(0, std::memory_order_relaxed);
    return 0;
}

uint_fast8_t PosixProc::start(const Task &task)
{
    if (isRunning())
    {
        spdlog::error("{}:{} Process is running", __FILE__, __LINE__);
        return 1;
    }

    memset(m_fd, 0, 2 * sizeof(int));
    m_exitCode.store(0, std::memory_order_relaxed);

    if (pipe(m_fd) == -1)
    {
        spdlog::error("{}:{} {}", __FILE__, __LINE__, strerror(errno));
        return 1;
    }

    m_pid = fork();
    if (m_pid == -1)
    {
        // parent process
        spdlog::error("{}:{} {}", __FILE__, __LINE__, strerror(errno));
        return 1;
    }

    if (m_pid == 0)
    {
        // child process
        startChild(task);
    }

    int fileFlag(fcntl(m_fd[0], F_GETFL));
    if (fileFlag == -1)
    {
        spdlog::error("{}:{} {}", __FILE__, __LINE__, strerror(errno));
        kill(m_pid, SIGKILL);
        return 1;
    }

    if (fcntl(m_fd[0], F_SETFL, fileFlag | O_NONBLOCK) == -1)
    {
        spdlog::error("{}:{} {}", __FILE__, __LINE__, strerror(errno));
        kill(m_pid, SIGKILL);
        return 1;
    }

    if (setpgid(m_pid, 0) == -1)
    {
        spdlog::error("{}:{} {}", __FILE__, __LINE__, strerror(errno));
        kill(m_pid, SIGKILL);
        return 1;
    }

    close(m_fd[1]);
    return 0;
}

void PosixProc::stop()
{
    stopImpl();
}

bool PosixProc::isRunning()
{
    int status;
    pid_t ret = waitpid(m_pid, &status, WNOHANG);
    if (ret == -1)
    {
        spdlog::error("{}:{} {}", __FILE__, __LINE__, strerror(errno));
        return false;
    }
    else if (ret == 0)
    {
        return true;
    }
    else
    {
        if (WIFEXITED(status) || WIFSIGNALED(status))
        {
            m_exitCode.store(status, std::memory_order_relaxed);
        }

        return false;
    }
}

uint_fast8_t PosixProc::readCurrentOutput(std::string &out)
{
    ssize_t count(0);
    char buf[4096] = {};
    while (1)
    {
        count = read(m_fd[0], buf, 4095);
        if (count == -1)
        {
            if (errno == EINTR) continue;
            else
            {
                spdlog::error("{}:{} {}", __FILE__, __LINE__, strerror(errno));
                return 1;
            }
        }
        else if (count == 0)
        {
            out = "";
            return 0;
        }
        else
        {
            buf[count] = '\0';
            out = buf;
        }

        return 0;
    }
}

uint_fast8_t PosixProc::exitCode(int_fast32_t &out)
{
    if (isRunning())
    {
        spdlog::error("{}:{} Process is running", __FILE__, __LINE__);
        return 1;
    }

    out = m_exitCode.load(std::memory_order_relaxed);
    return 0;
}

// private member functions
void PosixProc::startChild(const Task &task)
{
    while (( dup2(m_fd[1], STDOUT_FILENO) == -1 ) && ( errno == EINTR )) {}
    while (( dup2(STDOUT_FILENO, STDERR_FILENO) == -1 ) && ( errno == EINTR )) {}
    close(m_fd[0]);

    if (chdir(task.workDir.c_str()) == -1)
    {
        perror("chdir");
        exit(1);
    }

    char **argv = buildChildArgv(task);
    if (!argv)
    {
        exit(1);
    }

    // start child
    char *env[] = { NULL };
    execve(task.execName.c_str(), argv, env);
    perror("execve");   /* execve() returns only on error */
    exit(1);
}

char **PosixProc::buildChildArgv(const Task &task)
{
    char **argv(nullptr);
    if (task.args.size())
    {
        argv = new ( std::nothrow ) char *[task.args.size() + 2](); // process name + null for tail
    }
    else
    {
        argv = new ( std::nothrow ) char *[2]();
    }

    if (!argv)
    {
        fprintf(stderr, "Fail to allocate child process' argv.");
        return nullptr;
    }

    size_t nameLen = task.execName.length() + 1;
    argv[0] = new ( std::nothrow ) char[nameLen]();
    if (!argv[0])
    {
        delete[] argv;
        fprintf(stderr, "Fail to allocate child process' argv[0].");
        return nullptr;
    }

    memcpy(argv[0], task.execName.c_str(), nameLen);
    if (task.args.size())
    {
        size_t index(0);
        for (size_t i = 0; i < task.args.size(); ++i)
        {
            index = i + 1;
            argv[index] = new ( std::nothrow ) char[task.args[i].size() + 1](); // +1 for '\0'
            if (!argv[index])
            {
                for (size_t j = 0; j < index; ++j)
                {
                    delete[] argv[j];
                }

                delete[] argv;
                fprintf(stderr, "Fail to allocate child process' argv[%zu].", index);
                return nullptr;
            }

            memcpy(argv[index], task.args[i].c_str(), task.args[i].size());
            argv[index][task.args[i].size()] = '\0';
        }

        argv[task.args.size() + 1] = NULL;
    }
    else
    {
        argv[1] = NULL;
    }

    return argv;
}

void PosixProc::stopImpl()
{
    if (kill(m_pid * -1, SIGKILL) == -1)
    {
        spdlog::error("{}:{} {}", __FILE__, __LINE__, strerror(errno));
        return;
    }

    sleep(2);
}

} // end namespace Proc

} // end namespace Model
