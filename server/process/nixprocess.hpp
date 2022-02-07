#pragma once

#include "unistd.h"

#include "abstractprocess.hpp"

class NixProcess : public AbstractProcess
{
public:

    NixProcess();

    ~NixProcess();

    uint8_t init(AbstractProcess *) override;

    void reset() override;

    bool isRunning() override;

    uint8_t start(const char *name,
                  const char *workDir,
                  std::vector<std::string> &args,
                  char *errMsg) override;

    uint8_t stop(char *errMsg) override;

    uint8_t readStdOut(char *buf, size_t *bufSize, char *errMsg) override;

    AbstractProcess::ExitState exitCode(int *code, char *errMsg) override;

private:

    pid_t m_pid;

    int m_fd[2];
};
