/*
 * Simple Task Queue
 * Copyright (c) 2024 fdar0536
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

#include <csignal>

#include "spdlog/spdlog.h"
#include "cxxopts.hpp"


#ifdef _WIN32
#include "windows.h"
#endif

#include "controller/global/global.hpp"
#include "model/utils.hpp"
#include "global.hpp"

#include "main.hpp"

namespace Controller
{

namespace CLI
{

static void sighandler(int signum);

#ifdef _WIN32
static BOOL eventHandler(DWORD dwCtrlType);
#endif

Main::Main()
{}

Main::~Main()
{
    Controller::Global::consoleFin();
}

// public member functions
i32 Main::init(int argc, char **argv)
{
    if (!argv)
    {
        spdlog::error("{}:{} {}", __FILE__, __LINE__, "you should never see this line");
        return 1;
    }

    if (Controller::Global::isAdmin())
    {
#ifdef _WIN32
        spdlog::error("{}:{} Refuse to run as administrator", __FILE__, __LINE__);
#else
        spdlog::error("{}:{} Refuse to run as super user", __FILE__, __LINE__);
#endif
        return 1;
    }

    i32 ret(parseArgs(argc, argv));
    if (ret)
    {
        spdlog::error("{}:{} Fail to parse args.", __FILE__, __LINE__);
        return ret;
    }

    if (spdlogInit())
    {
        spdlog::error("{}:{} spdlog init failed", __FILE__, __LINE__);
        return 1;
    }

    spdlog::set_level(Global::config.logLevel);
    Global::keepRunning.store(true, std::memory_order_relaxed);

    signal(SIGABRT, sighandler);
    signal(SIGFPE,  sighandler);
    signal(SIGILL,  sighandler);
    signal(SIGINT,  sighandler);
    signal(SIGSEGV, sighandler);
    signal(SIGTERM, sighandler);

#ifdef _WIN32
    SetConsoleCtrlHandler(eventHandler, TRUE);
#endif

    return 0;
}

i32 Main::run()
{
    return 0;
}

// private menber functions
i32 Main::parseArgs(int argc, char **argv)
{
    if (!argv)
    {
        spdlog::error("{}:{} {}", __FILE__, __LINE__, "you should never see this line");
        return 1;
    }

    try
    {
        cxxopts::Options options("STQCLI", "STQ CLI Client");
        options.add_options()
            ("l,log-file", "file for output log", cxxopts::value<std::string>(Global::config.logFile)->default_value(""))
            ("L,log-level", "log level for spdlog", cxxopts::value<spdlog::level::level_enum>(Global::config.logLevel)->default_value("2"))
            ("a,address", "address to STQ server", cxxopts::value<std::string>(Global::config.address)->default_value("127.0.0.1"))
            ("p,port", "which port is STQ Server listening", cxxopts::value<u16>(Global::config.port)->default_value("12345"))
            ("v,version", "print version")
            ("h,help", "print help")
            ;

        auto result = options.parse(argc, argv);
        if (result.count("help"))
        {
            std::cout << options.help() << std::endl;
            return 2;
        }

        if (result.count("version"))
        {
            printVersion();
            return 2;
        }
    }
    catch(cxxopts::exceptions::exception e)
    {
        spdlog::error("{}:{} {}", __FILE__, __LINE__, e.what());
        return 1;
    }

    return 0;
}

void Main::printVersion()
{
    Model::Utils::writeConsole("STQCLI version info:\n");

    Model::Utils::writeConsole("branch:  ");
    Model::Utils::writeConsole(STQ_BRANCH);
    Model::Utils::writeConsole("\n");

    Model::Utils::writeConsole("commit:  ");
    Model::Utils::writeConsole(STQ_COMMIT);
    Model::Utils::writeConsole("\n");

    Model::Utils::writeConsole("version: ");
    Model::Utils::writeConsole(STQ_VERSION);
    Model::Utils::writeConsole("\n");
}

u8 Main::spdlogInit()
{
    if (Global::config.logFile.empty())
    {
        return 0;
    }

    if (Controller::Global::spdlogInit(Global::config.logFile))
    {
        spdlog::error("{}:{} spdlog init failed", __FILE__, __LINE__);
        return 1;
    }

    return 0;
}

static void sighandler(int signum)
{
    spdlog::info("{}:{} Signaled: {}", __FILE__, __LINE__, signum);
    spdlog::info("{}:{} Good Bye!", __FILE__, __LINE__);
    Global::keepRunning.store(false, std::memory_order_relaxed);
}

#ifdef _WIN32
static BOOL eventHandler(DWORD dwCtrlType)
{
    sighandler(dwCtrlType);
    return TRUE;
}
#endif

} // end namesapce CLI

} // end namespace Controller
