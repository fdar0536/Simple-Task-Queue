/*
 * Simple Task Queue
 * Copyright (c) 2022-2023 fdar0536
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

#include "main.hpp"

int main()
{
#ifdef _WIN32
    UINT consoleCP(0);
    UINT consoleOutputCP(0);
    if (Main::init(consoleCP, consoleOutputCP))
#else
    if (Main::init())
#endif
    {
        spdlog::error("{}:{} Fail to initialize", __FILE__, __LINE__);
        return 1;
    }

    const int ret(View::CLI::main());

#ifdef _WIN32
    Main::fin(consoleCP, consoleOutputCP);
#else
    Main::fin();
#endif
    return ret;
}

namespace Main
{

#ifdef _WIN32
static uint_fast8_t init(UINT &consoleCP, UINT &consoleOutputCP)
#else
static uint_fast8_t init()
#endif
{
    fflush(stdout);
#if defined _MSC_VER
#   pragma region WIN_UNICODE_SUPPORT_MAIN
#endif
#if defined _WIN32
    // change code page to UTF-8 UNICODE
    if (!IsValidCodePage(CP_UTF8))
    {
        Model::Utils::writeLastError(__FILE__, __LINE__);
        return 1;
    }

    consoleCP = GetConsoleCP();
    consoleOutputCP = GetConsoleOutputCP();

    if (!SetConsoleCP(CP_UTF8))
    {
        Model::Utils::writeLastError(__FILE__, __LINE__);
        return 1;
    }
    if (!SetConsoleOutputCP(CP_UTF8))
    {
        Model::Utils::writeLastError(__FILE__, __LINE__);
        return 1;
    }

    // change file stream translation mode
    if (_setmode(_fileno(stdout), _O_U8TEXT) == -1)
    {
        Model::Utils::writeLastError(__FILE__, __LINE__);
        return 1;
    }

    if (_setmode(_fileno(stderr), _O_U8TEXT) == -1)
    {
        Model::Utils::writeLastError(__FILE__, __LINE__);
        return 1;
    }

    if (_setmode(_fileno(stdin), _O_U16TEXT) == -1)
    {
        Model::Utils::writeLastError(__FILE__, __LINE__);
        return 1;
    }
#endif
#if defined _MSC_VER
#   pragma endregion
#endif
    std::ios::sync_with_stdio(false);
    return 0;
}

#ifdef _WIN32
static void fin(UINT &consoleCP, UINT &consoleOutputCP)
#else
static void fin()
#endif
{
#ifdef _WIN32
    SetConsoleCP(consoleCP);
    SetConsoleOutputCP(consoleOutputCP);
#endif
}

} // end namespace Main
