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

#include <string>
#include <vector>

#include <cinttypes>

class AbstractProcess
{
public:

    typedef enum class ExitState
    {
        NormalExit, NonNormalExit, Failed
    } ExitState;

    virtual ~AbstractProcess() = 0;

    virtual uint8_t init(AbstractProcess *) = 0;

    virtual void reset() = 0;

    virtual bool isRunning() = 0;

    virtual uint8_t start(const char *name,
                          const char *workDir,
                          std::vector<std::string> &args) = 0;

    virtual uint8_t stop() = 0;

    virtual uint8_t readStdOut(char *buf, size_t *bufSize) = 0;

    virtual ExitState exitCode(int *code) = 0;

protected:

    char m_error[4096] = {};
};
