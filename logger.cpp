/*
 * Simple Task Queue
 * Copyright (c) 2020 fdar0536
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

#include <iostream>
#include <new>

#include <cstring>
#include <ctime>

#include "logger.hpp"

Logger::Logger(Level level) :
    m_logLevel(level),
    m_out(nullptr),
    m_lastUpdate(time(nullptr))
{
    updateFileName(m_lastUpdate);
    checkFile(m_lastUpdate);
}

Logger::~Logger()
{
    if (m_out) fclose(m_out);
}

void Logger::setLevel(Level level)
{
    m_logLevel = level;
}

Logger::Level Logger::getLevel() const
{
    return m_logLevel;
}

void Logger::setSavePath(std::string &path)
{
    m_savePath = path;
}

void Logger::write(Level level,
                       const char *in,
                       bool haveToWriteOut,
                       bool haveToWriteFile)
{
    if (!in || m_logLevel == Level::Off || level < m_logLevel) return;
    if (!haveToWriteOut && !haveToWriteFile) return;

    std::lock_guard<std::mutex> lock(m_mutex);

    time_t current = time(nullptr);
    printTimeStamp(current);
    char label[8192];
    label[0] = '\0';
    switch (level)
    {
    case Verbose:
    {
        sprintf(label, "%s Verbose: %s\n", m_log, in);
        break;
    }
    case Debug:
    {
        sprintf(label, "%s Debug: %s\n", m_log, in);
        break;
    }
    case Info:
    {
        sprintf(label, "%s Info: %s\n", m_log, in);
        break;
    }
    case Warning:
    {
        sprintf(label, "%s Warning: %s\n", m_log, in);
        break;
    }
    case Error:
    {
        sprintf(label, "%s Error: %s\n", m_log, in);
        break;
    }
    default:
    {
        break;
    }
    }

    if (haveToWriteOut)
    {
        printf("%s", label);
    }

    if (haveToWriteFile)
    {
        checkFile(current);
        m_out = fopen(m_lastFileName.c_str(), "a");
        if (!m_out)
        {
            printTimeStamp(current);
            sprintf(label, "%s Error: %s\n", m_log, "Fail to open output file.");
            printf("%s", label);
            return;
        }

        fputs(label, m_out);
        fflush(m_out);
        fclose(m_out);
        m_out = nullptr;
    }
}

void Logger::checkFile(time_t in)
{
    time_t now(time(0));
    if (difftime(now, m_lastUpdate) > 86400) // 24hours
    {
        updateFileName(in);
    }
}

void Logger::updateFileName(time_t in)
{
    struct tm *tm = localtime(&in);
    if (!tm) return;
    char tmpString[128];
    sprintf(tmpString,
            "%d%02d%02d.log",
            tm->tm_year + 1900,
            tm->tm_mon + 1,
            tm->tm_mday);
    m_lastFileName = m_savePath + tmpString;
}

void Logger::printTimeStamp(time_t in)
{
    struct tm *tm = localtime(&in);
    if (!tm) return;

    sprintf(m_log,
            "%d-%02d-%02d %02d:%02d:%02d",
            tm->tm_year + 1900,
            tm->tm_mon + 1,
            tm->tm_mday,
            tm->tm_hour,
            tm->tm_min,
            tm->tm_sec);
}
