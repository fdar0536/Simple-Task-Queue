/*
 * Simple Task Queue
 * Copyright (c) 2023-2024 fdar0536
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

#include <fstream>

#ifdef _WIN32
#include "model/win32-code/getopt.h"
#else
#include "getopt.h"
#endif

#include "inipp.h"

#include "model/dao/dirutils.hpp"
#include "model/utils.hpp"
#include "config.hpp"

namespace Controller
{

namespace GRPCServer
{

Config::Config()
{}

Config::~Config()
{}

uint_fast8_t Config::parse(Config *in, int argc, char **argv)
{
    if (!in)
    {
        spdlog::warn("{}:{} input is nullptr", __FILE__, __LINE__);
        return 1;
    }

    if (argc == 1 || !argv)
    {
        spdlog::warn("{}:{} use default config", __FILE__, __LINE__);
        return 0;
    }

    if (argc == 2 && (( strcmp(argv[1], "-h")) || (strcmp(argv[1], "--help"))))
    {
        printHelp(argv);
        return 1;
    }

    struct option opts[] =
    {
        {"config-file", required_argument, NULL, 'c'},
        {0, 0, 0, 0}
    };

    int c(0);
    std::string configFile("");

    while ((c = getopt_long(argc, argv, "c:", opts, NULL)) != -1)
    {
        switch (c)
        {
        case 'c':
        {
            configFile = optarg;
            if (Model::DAO::DirUtils::verifyFile(optarg))
            {
                spdlog::error("{}:{} Fail to verify config file", __FILE__, __LINE__);
                return 1;
            }
            break;
        }
        default:
        {
            printHelp(argv);
            return 1;
        }
        } // end switch(c)
    }

    if (configFile.empty())
    {
        spdlog::warn("{}:{} no config file", __FILE__, __LINE__);
        return 0;
    }

    if (parse(in, configFile))
    {
        spdlog::error("{}:{} fail to parse config file", __FILE__, __LINE__);
        return 1;
    }

    return 0;
}

template <typename CharT, typename T>
static inline bool get_value(const std::map<std::basic_string<CharT>,
                                            std::basic_string<CharT>> & sec,
                             const std::basic_string<CharT> & key, T & dst)
{
    const auto it = sec.find(key);
    if (it == sec.end()) return false;
    return inipp::extract(it->second, dst);
}

template <typename CharT, typename T>
inline bool get_value(const std::map<std::basic_string<CharT>,
                                     std::basic_string<CharT>>& sec,
                      const CharT* key, T& dst)
{
    return get_value(sec, std::basic_string<CharT>(key), dst);
}

uint_fast8_t Config::parse(Config *obj, const std::string &path)
{
    if (!obj)
    {
        spdlog::warn("{}:{} input is nullptr", __FILE__, __LINE__);
        return 1;
    }

    std::unique_lock<std::mutex> lock(obj->m_mutex);
    std::ifstream i(path.c_str());
    try
    {
        if (i.fail())
        {
            spdlog::warn("{}:{} Fail to open file: {}", __FILE__, __LINE__, path);
            return 1;
        }

        inipp::Ini<char> ini;
        ini.parse(i);
        i.close();

        UNUSED(get_value(ini.sections["Settings"],
                         "db path",
                         obj->m_dbPath));

        UNUSED(get_value(ini.sections["Settings"],
                         "log path",
                         obj->m_logPath));

        Model::DAO::DirUtils::convertPath(obj->m_logPath);
        if (Model::DAO::DirUtils::verifyDir(obj->m_logPath))
        {
            spdlog::error("{}:{} fail to verify log path", __FILE__, __LINE__);
            return 1;
        }

        UNUSED(get_value(ini.sections["Settings"],
                         "port",
                         obj->m_listenPort));

        if (obj->m_listenPort > 65535)
        {
            spdlog::error("{}:{} Invalid port", __FILE__, __LINE__);
            return 1;
        }

        UNUSED(get_value(ini.sections["Settings"],
                         "ip",
                         obj->m_listenIP));

        if (Model::Utils::verifyIP(obj->m_listenIP))
        {
            spdlog::error("{}:{} Invalid ip", __FILE__, __LINE__);
            return 1;
        }

        uint_fast8_t level(0);
        UNUSED(get_value(ini.sections["Settings"],
                         "log level",
                         level));

        obj->m_logLevel = static_cast<spdlog::level::level_enum>(level);
    }
    catch (...)
    {
        i.close();
        spdlog::error("{}:{} error caught", __FILE__, __LINE__);
        return 1;
    }

    return 0;
}

std::string Config::dbPath()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_dbPath;
}

void Config::setDbPath(const std::string &in)
{
    if (in.empty())
    {
        spdlog::error("{}:{} input is empty", __FILE__, __LINE__);
        return;
    }

    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_dbPath = in;
    }
}

std::string Config::logPath()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_logPath;
}

void Config::setLogPath(const std::string &in)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_logPath = in;
}

uint_fast16_t Config::listenPort()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_listenPort;
}

void Config::setListenPort(uint_fast16_t in)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_listenPort = in;
}

std::string Config::listenIP()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_listenIP;
}

uint_fast8_t Config::setListenIP(const std::string &in)
{
    if (Model::Utils::verifyIP(in))
    {
        spdlog::error("{}:{} input is valid ipv4", __FILE__, __LINE__);
        return 1;
    }

    std::unique_lock<std::mutex> lock(m_mutex);
    m_listenIP = in;
    return 0;
}

spdlog::level::level_enum Config::logLevel()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_logLevel;
}

void Config::setLogLevel(spdlog::level::level_enum in)
{
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_logLevel = in;
    }

    spdlog::set_level(in);
}

// private member functions
void Config::printHelp(char **argv)
{
    if (!argv)
    {
        spdlog::error("{}:{} argv is null", __FILE__, __LINE__);
        return;
    }

    Model::Utils::writeConsole(argv[0]);
    Model::Utils::writeConsole(" usage:\n");
    Model::Utils::writeConsole("--config-file <file>, -c <file>: set config file\n");
}

} // end namespace GRPCServer

} // end namespace Model
