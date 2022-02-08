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

#include <fstream>
#include <mutex>
#include <string>

#include <ctime>

class Logger
{

public:

    typedef enum Level
    {
        Verbose, Debug, Info, Warning, Error, Off
    } Level;

    Logger(Level = Level::Info);

    ~Logger();

    void setLevel(Level);

    Level getLevel() const;

    void write(Level, const char *, bool = true, bool = true);

private:

    Level m_logLevel;

    FILE *m_out;

    std::mutex m_mutex;

    time_t m_lastUpdate;

    char m_lastFileName[16];

    char m_log[32];

    void checkFile(time_t);

    void updateFileName(time_t);

    void printTimeStamp(time_t);
};
