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

#include "spdlog/spdlog.h"
#include "inipp.h"
#include "cxxopts.hpp"

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

u8 Config::parse(Config *in, int argc, char **argv)
{
    if (!in)
    {
        spdlog::warn("{}:{} input is nullptr", __FILE__, __LINE__);
        return 1;
    }

    if (!argv)
    {
        spdlog::error("{}:{} you should never see this line", __FILE__, __LINE__);
        return 1;
    }

    if (argc == 1 || !argv)
    {
        spdlog::warn("{}:{} use default config", __FILE__, __LINE__);
        return 0;
    }

    std::string configFile("");

    try
    {
        cxxopts::Options options("STQSERVER", "STQ Server");
        options.add_options()
            ("c,config-file", "path to config file", cxxopts::value<std::string>(in->logPath)->default_value(""))
            ("d,db-path", "path to config file", cxxopts::value<std::string>(in->dbPath)->default_value(""))
            ("l,log-path", "path for output log", cxxopts::value<std::string>(in->logPath)->default_value(""))
            ("L,log-level", "log level for spdlog", cxxopts::value<int>(in->logLevel)->default_value("2"))
            ("a,address", "which addess will listen", cxxopts::value<std::string>(in->listenIP)->default_value("127.0.0.1"))
            ("p,port", "which port will listen", cxxopts::value<u16>(in->listenPort)->default_value("12345"))
            ("v,version", "print version")
            ("h,help", "print help")
            ;

        auto result = options.parse(argc, argv);
        if (result.count("help"))
        {
            Model::Utils::writeConsole(options.help());
            return 2;
        }

        if (result.count("version"))
        {
            printVersion();
            return 2;
        }

        if (!configFile.empty())
        {
            if (Model::DAO::DirUtils::verifyFile(configFile))
            {
                spdlog::error("{}:{} Fail to verify config file", __FILE__, __LINE__);
                return 1;
            }
        }

        if (!in->logPath.empty())
        {
            Model::DAO::DirUtils::convertPath(in->logPath);
            if (Model::DAO::DirUtils::verifyDir(in->logPath))
            {
                spdlog::error("{}:{} fail to verify log path", __FILE__, __LINE__);
                return 1;
            }
        }

        if (in->listenPort > 65535)
        {
            spdlog::error("{}:{} Invalid port", __FILE__, __LINE__);
            return 1;
        }

        if (Model::Utils::verifyIP(in->listenIP))
        {
            spdlog::error("{}:{} Invalid ip", __FILE__, __LINE__);
            return 1;
        }
    }
    catch(cxxopts::exceptions::exception e)
    {
        spdlog::error("{}:{} {}", __FILE__, __LINE__, e.what());
        return 1;
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
                         obj->dbPath));

        UNUSED(get_value(ini.sections["Settings"],
                         "log path",
                         obj->logPath));

        Model::DAO::DirUtils::convertPath(obj->logPath);
        if (Model::DAO::DirUtils::verifyDir(obj->logPath))
        {
            spdlog::error("{}:{} fail to verify log path", __FILE__, __LINE__);
            return 1;
        }

        UNUSED(get_value(ini.sections["Settings"],
                         "port",
                         obj->listenPort));

        if (obj->listenPort > 65535)
        {
            spdlog::error("{}:{} Invalid port", __FILE__, __LINE__);
            return 1;
        }

        UNUSED(get_value(ini.sections["Settings"],
                         "ip",
                         obj->listenIP));

        if (Model::Utils::verifyIP(obj->listenIP))
        {
            spdlog::error("{}:{} Invalid ip", __FILE__, __LINE__);
            return 1;
        }

        uint_fast8_t level(0);
        UNUSED(get_value(ini.sections["Settings"],
                         "log level",
                         level));

        obj->logLevel = static_cast<spdlog::level::level_enum>(level);
    }
    catch (...)
    {
        i.close();
        spdlog::error("{}:{} error caught", __FILE__, __LINE__);
        return 1;
    }

    return 0;
}

// private member functions
void Config::printVersion()
{
    Model::Utils::writeConsole("STQSERVER version info:\n");

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

} // end namespace GRPCServer

} // end namespace Model
