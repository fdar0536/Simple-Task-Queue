#pragma once

#include <string>
#include <vector>

#include <cinttypes>

class AbstractProcess
{
public:

    typedef enum class ExitState
    {
        NormalExit, NonNornalExit, Failed
    } ExitState;

    virtual ~AbstractProcess() = 0;

    virtual uint8_t init(AbstractProcess *) = 0;

    virtual void reset() = 0;

    virtual bool isRunning() = 0;

    virtual uint8_t start(const char *name,
                          const char *workDir,
                          std::vector<std::string> &args,
                          char *errMsg) = 0;

    virtual uint8_t stop(char *errMsg) = 0;

    virtual uint8_t readStdOut(char *buf, size_t *bufSize, char *errMsg) = 0;

    virtual ExitState exitCode(int *code, char *errMsg) = 0;
};
