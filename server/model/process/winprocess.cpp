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

#include <new>

#include "winprocess.hpp"
#include "tlhelp32.h"

#include "global.hpp"

WinProcess::WinProcess() :
    m_childStdInRead(NULL),
    m_childStdInWrite(NULL),
    m_childStdOutRead(NULL),
    m_childStdOutWrite(NULL)
{
    m_error.reserve(4096);
}

WinProcess::~WinProcess()
{
    resetImpl();
}

uint8_t WinProcess::init(AbstractProcess *)
{
    return 0;
}

void WinProcess::reset()
{
    resetImpl();
}

uint8_t WinProcess::start(const char *pathToProcess,
                          const char *workDir,
                          std::vector<std::string> &args)
{
    if (!pathToProcess || !workDir)
    {
        m_error.clear();
        m_error += __FILE__;
        m_error += ":";
        m_error += std::to_string(__LINE__);
        m_error += " Invalid input.";

        Global::logger.write(Logger::Error, m_error.c_str());
        reset();
        return 1;
    }

    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&m_childStdOutRead, &m_childStdOutWrite, &saAttr, 0))
    {
        writeLastError(__FILE__, __LINE__);
        reset();
        return 1;
    }

    if (!SetHandleInformation(m_childStdOutRead, HANDLE_FLAG_INHERIT, 0))
    {
        writeLastError(__FILE__, __LINE__);
        reset();
        return 1;
    }

    if (!CreatePipe(&m_childStdInRead, &m_childStdInWrite, &saAttr, 0))
    {
        writeLastError(__FILE__, __LINE__);
        reset();
        return 1;
    }

    if (!SetHandleInformation(m_childStdInWrite, HANDLE_FLAG_INHERIT, 0))
    {
        writeLastError(__FILE__, __LINE__);
        reset();
        return 1;
    }

    STARTUPINFOA startInfo;
    memset(&m_procInfo, 0, sizeof(PROCESS_INFORMATION));
    memset(&startInfo, 0, sizeof(STARTUPINFOA));

    startInfo.cb = sizeof(STARTUPINFOA);
    startInfo.hStdError = m_childStdInWrite;
    startInfo.hStdOutput = m_childStdOutWrite;
    startInfo.hStdInput = m_childStdInRead;
    startInfo.dwFlags |= STARTF_USESTDHANDLES;

    std::string cmdLine = "";
    BOOL res;
    char *cmdPtr = nullptr;
    if (args.size())
    {
        cmdPtr = new (std::nothrow) char[cmdLine.size() + 1]();
        if (!cmdPtr)
        {
            m_error.clear();
            m_error += __FILE__;
            m_error += ":";
            m_error += std::to_string(__LINE__);
            m_error += " Fail to allocate memory for command line.";

            Global::logger.write(Logger::Error, m_error.c_str());
            reset();
            return 1;
        }

        for (auto it = args.begin(); it != args.end(); ++it)
        {
            cmdLine += (*it);
            cmdLine += " ";
        }

        memcpy(cmdPtr, cmdLine.c_str(), cmdLine.size());
        cmdPtr[cmdLine.size()] = '\0';
    }

    if (!cmdLine.size())
    {
        res = CreateProcessA(pathToProcess,
                             NULL,          // command line
                             NULL,          // process security attributes
                             NULL,          // primary thread security attributes
                             TRUE,          // handles are inherited
                             0,             // creation flags
                             NULL,          // use parent's environment
                             workDir,
                             &startInfo,    // STARTUPINFO pointer
                             &m_procInfo);  // receives PROCESS_INFORMATION
    }
    else
    {
        res = CreateProcessA(pathToProcess,
                             cmdPtr,          // command line
                             NULL,            // process security attributes
                             NULL,            // primary thread security attributes
                             TRUE,            // handles are inherited
                             0,               // creation flags
                             NULL,            // use parent's environment
                             workDir,
                             &startInfo,      // STARTUPINFO pointer
                             &m_procInfo);    // receives PROCESS_INFORMATION
    }

    if (cmdPtr)
    {
        delete[] cmdPtr;
        cmdPtr = nullptr;
    }

    if (!res)
    {
        writeLastError(__FILE__, __LINE__);
        reset();
        return 1;
    }

    CloseHandle(m_childStdOutWrite);
    CloseHandle(m_childStdInRead);
    return 0;
}

uint8_t WinProcess::stop()
{
    return stopImpl();
}

bool WinProcess::isRunning()
{
    DWORD exitCode;
    if (!GetExitCodeProcess(m_procInfo.hProcess, &exitCode))
    {
        if (GetLastError() == STILL_ACTIVE)
        {
            return true;
        }
    }

    return false;
}

uint8_t WinProcess::readStdOut(char *buffer, size_t *bufferLen)
{
    DWORD readCount(0);
    while (1)
    {
        if (!ReadFile(m_childStdOutRead,
                      buffer,
                      (DWORD)(*bufferLen),
                      &readCount, NULL))
        {
            if (GetLastError() == ERROR_IO_PENDING)
            {
                continue;
            }

            writeLastError(__FILE__, __LINE__);
            return 1;
        }

        break;
    }


    if (readCount)
    {
        *bufferLen = readCount;
    }
    else
    {
        bufferLen = 0;
    }

    return 0;
}

AbstractProcess::ExitState WinProcess::exitCode(int *code)
{
    DWORD exitCode;
    if (!GetExitCodeProcess(m_procInfo.hProcess, &exitCode))
    {
        return AbstractProcess::ExitState::Failed;
    }

    if (exitCode == 0xC0000005)
    {
        return AbstractProcess::ExitState::NonNormalExit;
    }

    *code = exitCode;
    return AbstractProcess::ExitState::NormalExit;
}

// private member functions
void WinProcess::writeLastError(const char *file, int line)
{
    if (!file)
    {
        m_error.clear();
        return;
    }

    LPSTR msgBuf = nullptr;
    DWORD errID = GetLastError();
    if (!errID)
    {
        m_error.clear();
        return;
    }

    size_t errSize = FormatMessageA(
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                errID,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPSTR)&msgBuf, 0, NULL);

    m_error.clear();
    m_error += file;
    m_error += ":";
    m_error += std::to_string(line);
    m_error += " ";
    m_error += std::string(msgBuf, errSize);

    LocalFree(msgBuf);
    Global::logger.write(Logger::Error, m_error.c_str());
}

void WinProcess::resetImpl()
{
    stopImpl();
    CloseHandle(m_procInfo.hProcess);
    CloseHandle(m_procInfo.hThread);
    CloseHandle(m_childStdInWrite);
    CloseHandle(m_childStdInRead);
    CloseHandle(m_childStdOutWrite);
    CloseHandle(m_childStdOutRead);

    m_childStdInWrite = NULL;
    m_childStdInRead = NULL;
    m_childStdOutWrite = NULL;
    m_childStdOutRead = NULL;
    memset(&m_procInfo, 0, sizeof(PROCESS_INFORMATION));
}

uint8_t WinProcess::stopImpl()
{
    PROCESSENTRY32 pe;

    memset(&pe, 0, sizeof(PROCESSENTRY32));
    pe.dwSize = sizeof(PROCESSENTRY32);
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (Process32First(hSnap, &pe))
    {
        BOOL bContinue = TRUE;

        // kill child processes
        while (bContinue)
        {
            // only kill child processes
            if (pe.th32ParentProcessID == m_procInfo.dwProcessId)
            {
                HANDLE hChildProc = OpenProcess(PROCESS_ALL_ACCESS,
                                                FALSE,
                                                pe.th32ProcessID);

                if (hChildProc)
                {
                    TerminateProcess(hChildProc, 1);
                    CloseHandle(hChildProc);
                }
            }

            bContinue = Process32Next(hSnap, &pe);
        }

        // kill the main process
        HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS,
                                   FALSE,
                                   m_procInfo.dwProcessId);

        if (hProc)
        {
            TerminateProcess(hProc, 1);
            CloseHandle(hProc);
        }
    }

    return 0;
}
