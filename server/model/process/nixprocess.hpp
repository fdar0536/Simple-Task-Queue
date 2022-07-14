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

#pragma once

#include "unistd.h"

#include "abstractprocess.hpp"

class NixProcess : public AbstractProcess
{
public:

    NixProcess();

    ~NixProcess();

    uint8_t init(AbstractProcess *, Logger *) override;

    void reset() override;

    bool isRunning() override;

    uint8_t start(const char *name,
                  const char *workDir,
                  std::vector<std::string> &args) override;

    uint8_t stop() override;

    uint8_t readStdOut(char *buf, size_t *bufSize) override;

    AbstractProcess::ExitState exitCode(int *code) override;

private:

    pid_t m_pid;

    int m_fd[2];

    bool m_isStop;

    bool m_exitCode;

    void resetImpl();

    uint8_t stopImpl();
};
