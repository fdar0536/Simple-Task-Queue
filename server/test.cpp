#include <iostream>
#include <new>

#include "logger.hpp"
#ifdef _WIN32
#include "model/process/winprocess.hpp"
#define sleep(x) Sleep(x * 1000)
#else
#include "model/process/nixprocess.hpp"
#endif

#ifndef MAX_PATH
#define MAX_PATH 16384
#endif

uint8_t getWorkDir(char *buf, size_t bufLen);

int main(int argc, char **argv)
{
    if (argc == 1)
    {
        std::cerr << "no input" << std::endl;
        return 1;
    }

    AbstractProcess *proc(nullptr);
#ifdef _WIN32
    proc = new (std::nothrow) WinProcess;
#else
    proc = new (std::nothrow) NixProcess;
#endif

    if (!proc)
    {
        std::cerr << "Fail to allocate process" << std::endl;
        return 1;
    }

    Logger logger;
    std::string logPath("debugLog.log");
    logger.setSavePath(logPath);
    int ret(0);
    char workDir[MAX_PATH] = { 0 };
    std::string tmpString;
    std::vector<std::string> args;
    args.reserve(argc - 1);

    char outBuf[4096];
    size_t outBufLen(4096);
    int childExitCode(0);
    AbstractProcess::ExitState exitState(AbstractProcess::ExitState::NormalExit);

    if (proc->init(proc, &logger))
    {
        std::cerr << "Fail to initialize process." << std::endl;
        ret = 1;
        goto error;
    }

    if (getWorkDir(workDir, MAX_PATH))
    {
        std::cerr << "Fail to initialize process." << std::endl;
        ret = 1;
        goto error;
    }

    std::cout << "workDir is: " << workDir << std::endl;

    // build args for child process
    if (argc > 2)
    {
        for (int i = 2; i < argc; ++i)
        {
            tmpString = argv[i];
            std::cout << "tmpString is: " << tmpString << std::endl;
            args.push_back(tmpString);
        }
    }

    if (proc->start(argv[1], workDir, args))
    {
        std::cerr << "Fail to start process." << std::endl;
        ret = 1;
        goto error;
    }

#ifdef _WIN32
    if (proc->readStdOut(outBuf, &outBufLen))
    {
        std::cerr << "Fail to read stdout." << std::endl;
        ret = 1;
        goto error;
    }

    outBuf[outBufLen] = '\0';
    std::cout << "outBuf is: " << outBuf << std::endl;
#else
    for(int i = 0; i < 5 && proc->isRunning(); ++i)
    {
        if (proc->readStdOut(outBuf, &outBufLen))
        {
            std::cerr << "Fail to read stdout." << std::endl;
            ret = 1;
            goto error;
        }

        if (!outBufLen)
        {
            std::cout << "No output" << std::endl;
        }
        else
        {
            outBuf[outBufLen] = '\0';
            std::cout << "outBuf is: " << outBuf << std::endl;
        }

        outBufLen = 4096;
        std::cout << "Sleep for 1 second" << std::endl;
        sleep(1);
    }
#endif

    proc->stop();
    sleep(1);
    exitState = proc->exitCode(&childExitCode);
    if (exitState == AbstractProcess::ExitState::Failed)
    {
        std::cerr << "Fail to get exit code." << std::endl;
        ret = 1;
        goto error;
    }

    if (exitState == AbstractProcess::ExitState::NonNormalExit)
    {
        std::cout << "Child process may be crashed or signaled" << std::endl;
        goto error;
    }

    std::cout << "Child process' exit code is: " << childExitCode << std::endl;

error:
    delete proc;
    return ret;
}

uint8_t getWorkDir(char *buf, size_t bufLen)
{
    if (!buf || !bufLen) return 1;

#ifdef _WIN32
    if (GetModuleFileNameA(NULL, buf, bufLen) == bufLen)
    {
        return 1;
    }

    std::string::size_type pos = std::string(buf).find_last_of("\\/");
    buf[pos] = '\0';
#else
    if (!getcwd(buf, bufLen))
    {
        return 1;
    }
#endif
    return 0;
}
