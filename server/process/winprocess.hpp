#pragma once

#include "abstractprocess.hpp"

class WinProcess : public AbstractProcess
{
    WinProcess();

    ~WinProcess();

    uint8_t init(AbstractProcess *) override;

    uint8_t start(const char *pathToProcess,
                  const char *workDir,
                  std::vector<std::string> &args,
                  char *errMsg) override;

    uint8_t stop(char *errMsg) override;

    bool isRunning() override;

    uint8_t readStdOut(char *buffer, size_t *bufferLen, char *errMsg) override;
};
