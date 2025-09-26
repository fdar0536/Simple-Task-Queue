/*
 * Simple Task Queue
 * Copyright (c) 2023-present fdar0536
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

#include "spdlog/spdlog.h"

#include "winproc.hpp"
#include "tlhelp32.h"
#include "model/utils.hpp"

namespace Model
{

namespace Proc
{

WinProc::WinProc() :
    m_childStdoutRead(nullptr),
    m_childStdoutWrite(nullptr),
    m_procInfo(PROCESS_INFORMATION())
{}

WinProc::~WinProc()
{
    stopImpl();
}

u8 WinProc::init()
{
    m_procInfo.hProcess = NULL;
    m_procInfo.hThread = NULL;
    resetHandle();
    m_current_output = "";
    m_current_output.reserve(4096);
    return 0;
}

u8 WinProc::start(const Task &task)
{
    if (isRunning())
    {
        spdlog::error("{}:{} {}", __FILE__, __LINE__, "Process is running");
        return 1;
    }

    resetHandle();
    SECURITY_ATTRIBUTES saAttr;

    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = nullptr;

    if (!CreatePipe(&m_childStdoutRead, &m_childStdoutWrite, &saAttr, 0))
    {
        Utils::writeLastError(__FILE__, __LINE__);
        return 1;
    }

    if (!SetHandleInformation(m_childStdoutRead, HANDLE_FLAG_INHERIT, 0))
    {
        Utils::writeLastError(__FILE__, __LINE__);
        resetHandle();
        return 1;
    }

    // Create the child process.
    if (CreateChildProcess(task))
    {
        spdlog::error("{}:{} {}", __FILE__, __LINE__, "Fail to start process");
        resetHandle();
        return 1;
    }

    CloseHandle(m_childStdoutWrite);
    m_childStdoutWrite = NULL;

    m_exitCode.store(STILL_ACTIVE, std::memory_order_relaxed);
    m_thread = std::jthread(&WinProc::readOutputLoop, this);
    return 0;
}

void WinProc::stop()
{
    stopImpl();
}

bool WinProc::isRunning()
{
    if (m_procInfo.hProcess == NULL)
    {
        return false;
    }

    i32 exitCode = m_exitCode.load(std::memory_order_relaxed);
    if (exitCode != STILL_ACTIVE)
    {
        return false;
    }

    if (GetExitCodeProcess(m_procInfo.hProcess, (LPDWORD)&exitCode) == FALSE)
    {
        Utils::writeLastError(__FILE__, __LINE__);
        return true;
    }

    if (exitCode == STILL_ACTIVE)
    {
        return true;
    }

    if (m_thread.joinable())
    {
        m_thread.join();
    }

    m_exitCode.store(exitCode, std::memory_order_relaxed);
    return false;
}

u8 WinProc::readCurrentOutput(std::string &out)
{
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        out = m_current_output;
        m_current_output.clear();
    }

    return 0;
}

u8 WinProc::exitCode(i32 &out)
{
    if (isRunning())
    {
        spdlog::error("{}:{} {}", __FILE__, __LINE__, "Process is running");
        return 1;
    }

    out = m_exitCode.load(std::memory_order_relaxed);
    return 0;
}

// private member functions
u8 WinProc::CreateChildProcess(const Task &task)
{
    if (task.execName.empty())
    {
        spdlog::error("{}:{} {}", __FILE__, __LINE__, "task.execName is empty");
        return 1;
    }

    std::string cmdLine = "\"" + task.execName + "\"";
    if (task.args.size())
    {
        cmdLine += " ";
        size_t lastIndex = task.args.size() - 1;
        for (size_t i = 0; i <= lastIndex; ++i)
        {
            cmdLine += "\"" + task.args.at(i) + "\"";
            cmdLine += " ";
        }
    }

    char *cmdPtr = new ( std::nothrow )char[cmdLine.length() + 1]();
    if (!cmdPtr)
    {
        spdlog::error("{}:{} {}", __FILE__, __LINE__, "Fail to allocate memory");
        return 1;
    }

    memcpy(cmdPtr, cmdLine.c_str(), cmdLine.length());

    STARTUPINFOA siStartInfo;
    BOOL bSuccess = FALSE;

    // Set up members of the STARTUPINFO structure.
    // This structure specifies the STDIN and STDOUT handles for redirection.
    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdError = m_childStdoutWrite;
    siStartInfo.hStdOutput = m_childStdoutWrite;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    // Create the child process.
    bSuccess = CreateProcessA(NULL,
        cmdPtr,        // command line
        NULL,          // process security attributes
        NULL,          // primary thread security attributes
        TRUE,          // handles are inherited
        0,             // creation flags
        NULL,          // use parent's environment
        task.workDir.c_str(),
        &siStartInfo,  // STARTUPINFO pointer
        &m_procInfo);  // receives PROCESS_INFORMATION

    u8 ret(0);

    // If an error occurs, exit the application.
    if (!bSuccess)
    {
        Utils::writeLastError(__FILE__, __LINE__);
        ret = 1;
    }

    delete[] cmdPtr;
    return ret;
}

void WinProc::resetHandle()
{
    if (m_childStdoutRead)
    {
        CloseHandle(m_childStdoutRead);
        m_childStdoutRead = nullptr;
    }

    if (m_childStdoutWrite)
    {
        CloseHandle(m_childStdoutWrite);
        m_childStdoutWrite = nullptr;
    }

    if (m_procInfo.hProcess)
    {
        CloseHandle(m_procInfo.hProcess);
        m_procInfo.hProcess = NULL;
    }

    if (m_procInfo.hThread)
    {
        CloseHandle(m_procInfo.hThread);
        m_procInfo.hThread = NULL;
    }

    memset(&m_procInfo, 0, sizeof(PROCESS_INFORMATION));
}

void WinProc::stopImpl()
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

    resetHandle();
}

void WinProc::readOutputLoop()
{
    char buf[4096];
    BOOL bSuccess;
    DWORD dwRead;

    while(true)
    {
        bSuccess = ReadFile(m_childStdoutRead, buf, 4096, &dwRead, NULL);
        if (!bSuccess || dwRead == 0)
        {
            // child process is already close the pipe
            break;
        }

        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_current_output.append(buf, dwRead);
        }
    } // end while(true)
}

} // end namespace Proc

} // end namespace Model
