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
#include "queuelist.hpp"

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

    spdlog::set_level(static_cast<spdlog::level::level_enum>(Global::config.logLevel));
    Global::keepRunning.store(true, std::memory_order_relaxed);
    Global::args.reserve(16);

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
    i32 ret(0);
    std::string prefix = "> ";

    if (Global::config.autoConnect)
    {
        ret = connect();
    }

    while (Global::keepRunning.load(std::memory_order_relaxed))
    {
        Global::getArgs(prefix);

        // only accept "print" "modify" "connect" "exit"
        if (Global::args.at(0) == "print")
        {
            ret = print();
            continue;
        }

        if (Global::args.at(0) == "modify")
        {
            ret = modify();
            continue;
        }

        if (Global::args.at(0) == "connect")
        {
            ret = connect();
            continue;
        }

        if (Global::args.at(0) == "exit")
        {
            Global::keepRunning.store(false, std::memory_order_relaxed);
            Model::Utils::writeConsole("Good bye!\n");
            continue;
        }

        if (Global::args.at(0) == "help")
        {
            Model::Utils::writeConsole("Valid command: print, modify, connect, help, exit\n");
            Model::Utils::writeConsole("Please type \"[print|modify|connect] help\" for more details.\n");
            Model::Utils::writeConsole("Please type \"help\" to show this message.\n");
            Model::Utils::writeConsole("Please type \"exit\" to exit.\n");
            ret = 0;
            continue;
        }

        Model::Utils::writeConsole("Invalid command: " + Global::args[0] + "\n");
        Model::Utils::writeConsole("Please type \"help\" for more info\n");
        ret = 1;
    }

    return ret;
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
            ("L,log-level", "log level for spdlog", cxxopts::value<int>(Global::config.logLevel)->default_value("2"))
            ("a,address", "address to STQ server", cxxopts::value<std::string>(Global::config.address)->default_value("127.0.0.1"))
            ("A,auto-connect", "auto connect to server", cxxopts::value<bool>(Global::config.autoConnect)->default_value("true"))
            ("p,port", "which port is STQ Server listening", cxxopts::value<u16>(Global::config.port)->default_value("12345"))
            ("v,version", "print version")
            ("h,help", "print help")
            ;

        auto result = options.parse(argc, argv);
        if (result.count("help"))
        {
            Model::Utils::writeConsole(options.help() + "\n");
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

    Model::Utils::writeConsole("branch: ");
    Model::Utils::writeConsole(STQ_BRANCH);
    Model::Utils::writeConsole("\n");

    Model::Utils::writeConsole("commit: ");
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

i32 Main::print()
{
    if (Global::args.size() != 2)
    {
        Global::printCMDHelp("print");
        return 1;
    }

    if (Global::args.at(1) == "info")
    {
        printVersion();

        Model::Utils::writeConsole("\n");

        Model::Utils::writeConsole("target host: ");
        Model::Utils::writeConsole(Global::config.address);
        Model::Utils::writeConsole("\n");

        Model::Utils::writeConsole("target port: ");
        Model::Utils::writeConsole(std::to_string(Global::config.port));
        Model::Utils::writeConsole("\n");

        return 0;
    }

    if (Global::args.at(1) == "help")
    {
        Model::Utils::writeConsole("Valid command: \"print [info|help]\"\n");
        Model::Utils::writeConsole("info: print version and host info\n");
        Model::Utils::writeConsole("help: to show this message.\n");
        return 0;
    }

    Model::Utils::writeConsole("invalid argument\n");
    Model::Utils::writeConsole("Please type \"print help\" for more info\n");
    return 1;
}

i32 Main::modify()
{
    if (Global::args.size() > 5)
    {
        Global::printCMDHelp("modify");
        return 1;
    }

    if (Global::args.size() == 2)
    {
        if (Global::args.at(1) != "help")
        {
            Global::printCMDHelp("modify");
            return 1;
        }

        Model::Utils::writeConsole("Valid command: \"modify [address|port|help] <args>\"\n");
        Model::Utils::writeConsole("address <addreess>: server address\n");
        Model::Utils::writeConsole("port <port>: server port\n");
        Model::Utils::writeConsole("help: to show this message.\n");
        Model::Utils::writeConsole("You can modify address and port at same time.\n");

        return 0;
    }

    if (Global::args.size() != 3 && Global::args.size() != 5)
    {
        Global::printCMDHelp("modify");
        return 1;
    }

    // parse argument
    bool isIPSet(false);
    bool isPortSet(false);

    // here Global::args.size() == 3 or Global::args.size() == 5
    size_t baseIndex(1);
    size_t remainingCount(Global::args.size() - 1);
    std::string address("");
    u16 port(0);
    while (remainingCount > 0)
    {
        if (Global::args.at(baseIndex) == "address")
        {
            if (isIPSet)
            {
                Global::printCMDHelp("modify");
                return 1;
            }

            if (Global::args.at(baseIndex + 1).empty())
            {
                Global::printCMDHelp("modify");
                return 1;
            }

            address = Global::args.at(baseIndex + 1);
            isIPSet = true;
        }

        if (Global::args.at(baseIndex) == "port")
        {
            if (isPortSet)
            {
                Global::printCMDHelp("modify");
                return 1;
            }

            if (Global::args.at(baseIndex + 1).empty())
            {
                Global::printCMDHelp("modify");
                return 1;
            }

            try
            {
                port = static_cast<u16>(std::stoi(Global::args.at(baseIndex + 1)));
            }
            catch (...)
            {
                Global::printCMDHelp("modify");
                return 1;
            }

            isPortSet = true;
        }

        remainingCount -= 2;
        baseIndex += 2;
    }

    if (isIPSet)
    {
        Global::config.address = address;
    }

    if (isPortSet)
    {
        Global::config.port = port;
    }

    return 0;
}

i32 Main::connect()
{
    QueueList queueList;
    if (queueList.init())
    {
        Model::Utils::writeConsole("Fail to connect to server");
        return 1;
    }

    return queueList.run();
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
