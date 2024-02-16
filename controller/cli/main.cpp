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
#include <sstream>

#include "spdlog/spdlog.h"

#ifdef _WIN32
#include "model/win32-code/getopt.h"
#else
#include "getopt.h"
#endif // _WIN32

#include "controller/global/global.hpp"
#include "global.hpp"

#include "main.hpp"

namespace Controller
{

namespace CLI
{

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

    struct option opts[] =
        {
            {"log-file",  required_argument, NULL, 'l'},
            {"log-level", required_argument, NULL, 'L'},
            {"address",   required_argument, NULL, 'a'},
            {"port",      required_argument, NULL, 'p'},
            {"help",      no_argument,       NULL, 'h'},
            {"version",   no_argument,       NULL, 'v'},
            {0, 0, 0, 0}
        };

    int c(0);
    while ((c = getopt_long(argc, argv, "l:L:a:p:hv", opts, NULL)) != -1)
    {
        switch (c)
        {
        case 'l':
        {
            Global::config.logFile = optarg;
            break;
        }
        case 'L':
        {
            std::stringstream strValue;
            u8 level;
            try
            {
                strValue << optarg;
                strValue >> level;
            }
            catch (...)
            {
                spdlog::error("{}:{} Invaild argument: {}", __FILE__, __LINE__,
                              optarg);
                return 1;
            }

            if (level > 6)
            {
                spdlog::error("{}:{} Invaild level: {}", __FILE__, __LINE__,
                              level);
                return 1;
            }

            Global::config.logLevel =
                static_cast<spdlog::level::level_enum>(level);

            break;
        }
        case 'a':
        {
            Global::config.address = optarg;
            break;
        }
        case 'p':
        {
            try
            {
                std::stringstream strValue;
                strValue << optarg;
                strValue >> Global::config.port;
            }
            catch (...)
            {
                spdlog::error("{}:{} Invaild argument: {}", __FILE__, __LINE__,
                              optarg);
                return 1;
            }
            break;
        }
        case 'v':
        {
            printVersion(argv);
            return 0;
        }
        case 'h':
        {
            printHelp(argv);
            return 2;
        }
        default:
        {
            printHelp(argv);
            return 1;
        }
        } // end switch(c)
    }

    return 0;
}

void Main::printHelp(char **argv)
{
    if (!argv)
    {
        spdlog::error("{}:{} {}", __FILE__, __LINE__, "you should never see this line");
        return;
    }

    std::cout << argv[0] << " " << STQ_VERSION << " usage:" << std::endl;

    std::cout << "-l, --log-file <path to log file>: ";
    std::cout << "file for output log, default is stdout." << std::endl;

    std::cout << "-L, --log-level <level>: ";
    std::cout << "log level for spdlog, default value is 2(info)." << std::endl;

    std::cout << "-a, --address <ip or host name>: address to STQ server, ";
    std::cout << "default is 127.0.0.1" << std::endl;

    std::cout << "-p, --port: which port is STQ Server listening, ";
    std::cout << "default is 12345" << std::endl;

    std::cout << "-v, --version: print version and exit" << std::endl;

    std::cout << "-h, --help: print this help and exit" << std::endl;
}

void Main::printVersion(char **argv)
{
    if (!argv)
    {
        spdlog::error("{}:{} you should never see this line", __FILE__, __LINE__);
        return;
    }

    std::cout << argv[0] << " version info:" << std::endl;

    std::cout << "branch:  " << STQ_BRANCH << std::endl;
    std::cout << "commit:  " << STQ_COMMIT << std::endl;
    std::cout << "version: " << STQ_VERSION << std::endl;
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

} // end namesapce CLI

} // end namespace Controller
