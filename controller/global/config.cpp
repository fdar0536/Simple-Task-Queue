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

#include <fstream>

#ifdef _WIN32
#include "model/win32-code/getopt.h"
#else
#include "getopt.h"
#endif

#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"

#include "model/dao/dirutils.hpp"
#include "model/utils.hpp"
#include "config.hpp"

namespace json = rapidjson;

namespace Controller
{

namespace Global
{

Config::Config()
{}

Config::~Config()
{}

uint_fast8_t Config::parse(Config *in, int argc, char **argv)
{
#ifdef STQ_MOBILE
    UNUSED(in);
    UNSUED(argc);
    UNSUED(argv);
    return 0;
#else

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
#ifndef STQ_GUI
    std::string configFile("");
#endif
    while ((c = getopt_long(argc, argv, "c:", opts, NULL)) != -1)
    {
        switch (c)
        {
        case 'c':
        {
#ifdef STQ_GUI
            in->m_configPath = optarg;
#else
            configFile = optarg;
#endif
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

#ifdef STQ_GUI
    if (in->m_configPath.empty())
#else
    if (configFile.empty())
#endif
    {
        spdlog::warn("{}:{} no config file", __FILE__, __LINE__);
        return 0;
    }

#ifdef STQ_GUI
    if (parseJson(in, in->m_configPath))
#else
    if (parseJson(in, configFile))
#endif
    {
        spdlog::error("{}:{} fail to parse config file", __FILE__, __LINE__);
        return 1;
    }

    return 0;
#endif
}

static uint8_t getJSONString(std::string &dst,
                             json::Document &doc,
                             const char *key)
{
    if (!key)
    {
        spdlog::error("{}:{} key is null", __FILE__, __LINE__);
        return 1;
    }

    if (!doc[key].IsString())
    {
        spdlog::error("{}:{} {} is not string", __FILE__, __LINE__, key);
        return 1;
    }

    dst = doc[key].GetString();
    return 0;
}

uint_fast8_t Config::parseJson(Config *obj, const std::string &in)
{
    if (!obj)
    {
        spdlog::warn("{}:{} input is nullptr", __FILE__, __LINE__);
        return 1;
    }

    std::unique_lock<std::mutex> lock(obj->m_mutex);
    try
    {
        std::ifstream i(in.c_str());
        json::IStreamWrapper isw(i);
        json::Document j;
        j.ParseStream(isw);
        i.close();

#if defined(STQ_GUI) && !defined(STQ_MOBILE)
        // auto start server
        if (!j["auto start server"].IsBool())
        {
            spdlog::error("{}:{} \"auto start server\" is not bool", __FILE__, __LINE__);
            return 1;
        }

        obj->m_autoStartServer = j["auto start server"].GetBool();
#endif

        // log path
        if (getJSONString(obj->m_logPath, j, "log path"))
        {
            return 1;
        }

        Model::DAO::DirUtils::convertPath(obj->m_logPath);
        if (Model::DAO::DirUtils::verifyDir(obj->m_logPath))
        {
            spdlog::error("{}:{} fail to verify log path", __FILE__, __LINE__);
            return 1;
        }

        // listen port
        if (!j["port"].IsUint())
        {
            spdlog::error("{}:{} \"port\" is not number", __FILE__, __LINE__);
            return 1;
        }

        obj->m_listenPort = static_cast<uint_fast16_t>(j["port"].GetUint());
        if (obj->m_listenPort > 65535)
        {
            spdlog::error("{}:{} Invalid port", __FILE__, __LINE__);
            return 1;
        }

        // listen ip
        if (getJSONString(obj->m_listenIP, j, "ip"))
        {
            return 1;
        }

        if (Model::Utils::verifyIP(obj->m_listenIP))
        {
            spdlog::error("{}:{} Invalid ip", __FILE__, __LINE__);
            return 1;
        }

        // log level
        if (!j["log level"].IsUint())
        {
            spdlog::error("{}:{} \"log level\" is not number", __FILE__, __LINE__);
            return 1;
        }

        obj->m_logLevel = static_cast<spdlog::level::level_enum>(j["port"].GetUint());
        if (obj->m_listenPort > 65535)
        {
            spdlog::error("{}:{} Invalid log level", __FILE__, __LINE__);
            return 1;
        }
    }
    catch (...)
    {
        spdlog::error("{}:{} error caught", __FILE__, __LINE__);
        return 1;
    }

    return 0;
}

#if defined(STQ_GUI) && !defined(STQ_MOBILE)
void Config::setAutoStartServer(bool in)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_autoStartServer = in;
}

bool Config::autoStartServer()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_autoStartServer;
}

std::string Config::configPath()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_configPath;
}
#endif

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

void Config::setListenIP(const std::string &in)
{
    if (Model::Utils::verifyIP(in))
    {
        spdlog::error("{}:{} input is valid ipv4", __FILE__, __LINE__);
        return;
    }

    std::unique_lock<std::mutex> lock(m_mutex);
    m_listenIP = in;
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

} // end namespace Global

} // end namespace Model
