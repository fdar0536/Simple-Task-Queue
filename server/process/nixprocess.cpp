#include <cstring>
#include <new>

#include "sys/types.h"
#include "sys/wait.h"

#include "Nixprocess.hpp"

NixProcess::NixProcess() :
    m_pid(0)
{}

NixProcess::~NixProcess()
{
    reset();
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
    stop(nullptr);
    close(m_fd[0]);
    close(m_fd[1]);
    memset(m_fd, 0, 2);
}

bool NixProcess::isRunning()
{
    int state(0);
    return !waitpid(-1, &state, WNOHANG);
}

uint8_t NixProcess::start(const char *name,
                           const char *workDir,
                           std::vector<std::string> &args,
                           char *errMsg)
{
    if (!name || !workDir) return 1;

    if (pipe(m_fd) == -1)
    {
        if (errMsg)
        {
            sprintf(errMsg, "%s: %s", "pipe", strerror(errno));
        }

        return 1;
    }

    m_pid = fork();
    if (m_pid == -1)
    {
        // parent process
        if (errMsg)
        {
            sprintf(errMsg, "%s: %s", "fork", strerror(errno));
        }

        return 1;
    }
    else if (m_pid == 0)
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
        if (errMsg)
        {
            sprintf(errMsg, "%s: %s", "setpgid", strerror(errno));
        }

        kill(m_pid, SIGKILL);
        return 1;
    }

    return 0;
}

uint8_t NixProcess::stop(char *errMsg)
{
    if (kill(m_pid * -1, SIGKILL) == -1)
    {
        if (errMsg)
        {
            sprintf(errMsg, "%s: %s", "kill", strerror(errno));
        }

        return 1;
    }

    return 0;
}

uint8_t NixProcess::readStdOut(char *buf, size_t *bufSize, char *errMsg)
{
    if (!buf || !bufSize)
    {
        if (errMsg)
        {
            sprintf(errMsg, "%s", "Invalid input");
        }

        return 1;
    }

    if (!(*bufSize))
    {
        if (errMsg)
        {
            sprintf(errMsg, "%s", "Invalid buffer size.");
        }

        return 1;
    }

    if (!isRunning())
    {
        if (errMsg)
        {
            sprintf(errMsg, "%s", "Process is not running.");
        }

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
                if (errMsg)
                {
                    sprintf(errMsg, "%s: %s", "read", strerror(errno));
                }

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

AbstractProcess::ExitState NixProcess::exitCode(int *code, char *errMsg)
{
    if (!code) return AbstractProcess::ExitState::Failed;
    if (isRunning()) return AbstractProcess::ExitState::Failed;

    int status = 0;
    if (waitpid(m_pid, &status, WNOHANG) == -1)
    {
        if (errMsg)
        {
            sprintf(errMsg, "%s: %s", "waitpid", strerror(errno));
        }

        return AbstractProcess::ExitState::Failed;
    }

    if (WIFEXITED(status))
    {
        *code = WEXITSTATUS(status);
        return AbstractProcess::ExitState::NormalExit;
    }

    if (WIFSIGNALED(status))
    {
        return AbstractProcess::ExitState::NonNornalExit;
    }

    return AbstractProcess::ExitState::Failed;
}
