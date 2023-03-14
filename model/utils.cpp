/*
 * Simple Task Queue
 * Copyright (c) 2023 fdar0536
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

#ifdef _WIN32
#include "windows.h"
#endif

#include "spdlog/spdlog.h"

#include "utils.hpp"

namespace Model
{

namespace Utils
{

uint_fast8_t utf8ToUtf16(const std::string &in, wchar_t **out)
{
#ifdef _WIN32
    return utf8ToUtf16(in.c_str(), in.length(), out);
#else
    static_cast<void>(in);
    static_cast<void>(out);
    return 0;
#endif
}

uint_fast8_t utf8ToUtf16(const char *in, size_t inSize, wchar_t **out)
{
#ifdef _WIN32

    if (!in || !inSize || !out)
    {
        spdlog::error("{}:{} Invalid input", __FILE__, __LINE__);
        return 1;
    }

    int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, in, inSize, NULL, 0);
    if (sizeNeeded <= 0)
    {
        writeLastError(__FILE__, __LINE__);
        return 1;
    }

    *out = new (std::nothrow) wchar_t[sizeNeeded + 1]();
    if (!(*out))
    {
        spdlog::error("{}:{} Fail to allocate memory", __FILE__, __LINE__);
        return 1;
    }

    sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, in, inSize, *out, sizeNeeded);

    if (sizeNeeded <= 0)
    {
        writeLastError(__FILE__, __LINE__);
        delete[](*out);
        (*out) = nullptr;
        return 1;
    }

    (*out)[sizeNeeded] = L'\0';
    return 0;

#else
    static_cast<void>(in);
    static_cast<void>(inSize);
    static_cast<void>(out);
    return 0;
#endif
}

uint_fast8_t utf16ToUtf8(const std::wstring &in, char **out)
{
#ifdef _WIN32
    return utf16ToUtf8(in.c_str(), in.length(), out);
#else
    static_cast<void>(in);
    return 0;
#endif
}

uint_fast8_t utf16ToUtf8(const wchar_t *in, size_t inSize, char **out)
{
#ifdef _WIN32

    if (!in || !inSize || !out)
    {
        spdlog::error("{}:{} Invalid input", __FILE__, __LINE__);
        return 1;
    }

    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0,
        in, inSize, NULL, 0, NULL, NULL);
    if (sizeNeeded <= 0)
    {
        writeLastError(__FILE__, __LINE__);
        return 1;
    }

    *out = new (std::nothrow) char[sizeNeeded + 1]();
    if (!(*out))
    {
        spdlog::error("{}:{} Fail to allocate memory", __FILE__, __LINE__);
        return 1;
    }

    sizeNeeded = WideCharToMultiByte(CP_UTF8, 0,
        in, inSize, *out, sizeNeeded, NULL, NULL);

    if (sizeNeeded <= 0)
    {
        writeLastError(__FILE__, __LINE__);
        delete[](*out);
        (*out) = nullptr;
        return 1;
    }

    (*out)[sizeNeeded] = '\0';
    return 0;

#else
    static_cast<void>(in);
    static_cast<void>(inSize);
    static_cast<void>(out);
    return 0;
#endif
}

void writeLastError(const char *file, int line)
{
#ifdef _WIN32
    if (!file)
    {
        return;
    }

    LPSTR msgBuf = nullptr;
    DWORD errID = GetLastError();
    if (!errID)
    {
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

    spdlog::error("{}:{} {}", file, line, std::string(msgBuf, errSize));
    LocalFree(msgBuf);
#else
    static_cast<void>(file);
    static_cast<void>(line);
#endif
}

} // end namespace Utils

} // end namespace Model
