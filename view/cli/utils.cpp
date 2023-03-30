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

#include <iostream>

#include "spdlog/spdlog.h"

#include "model/utils.hpp"
#include "utils.hpp"

namespace View
{

namespace CLI
{

namespace Utils
{

std::string getInput(const std::string &prefix)
{
    std::string out = "";
#ifdef _WIN32
    wchar_t buf[4096] = {};
    if (Model::Utils::utf8ToUtf16(prefix, buf, 4096))
    {
        spdlog::error("{}:{} Fail to convert prefix", __FILE__, __LINE__);
        return "";
    }

    std::wcout << buf << L" ";

    std::wstring wout;
    std::getline(std::wcin, wout);

    char outBuf[4096] = {};
    if (Model::Utils::utf16ToUtf8(wout, outBuf, 4096))
    {
        spdlog::error("{}:{} Fail to convert output", __FILE__, __LINE__);
        return "";
    }

    out = std::string(outBuf);
#else
    std::cout << prefix << " ";
    std::getline(std::cin, out);
#endif
    return out;
}

} // end namespace Utils

} // end namespace CLI

} // end namespace View
