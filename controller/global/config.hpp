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

#ifndef _CONTROLLER_GLOBAL_CONFIG_HPP_
#define _CONTROLLER_GLOBAL_CONFIG_HPP_

#include <mutex>
#include <string>
#include <regex>

#include <inttypes.h>

#include "spdlog/spdlog.h"

#include "defines.hpp"

namespace Controller
{

namespace Global
{

class Config
{
public:

    Config();

    ~Config();

    static uint_fast8_t parse(Config *in, int argc, char **argv);

    static uint_fast8_t parse(Config *, const std::string &);

#if defined(STQ_GUI) && !defined(STQ_MOBILE)
    static uint_fast8_t save(Config *, const std::string &);

    bool autoStartServer();

    void setAutoStartServer(bool);

    std::string configPath();

    void setConfigPath(const std::string &);
#endif

#ifndef STQ_MOBILE
    std::string dbPath();

    void setDbPath(const std::string &);
#endif

    std::string logPath();

    void setLogPath(const std::string &);

    uint_fast16_t listenPort();

    void setListenPort(uint_fast16_t);

    std::string listenIP();

    void setListenIP(const std::string &);

    spdlog::level::level_enum logLevel();

    void setLogLevel(spdlog::level::level_enum);

private:

    static void printHelp(char **argv);

#if defined(STQ_GUI) && !defined(STQ_MOBILE)
    std::string m_configPath = "";

    bool m_autoStartServer = false;
#endif

#ifndef STQ_MOBILE
    std::string m_dbPath;
#endif

    std::string m_logPath = "";

    uint_fast16_t m_listenPort = 12345;

    std::string m_listenIP = "127.0.0.1";

    spdlog::level::level_enum m_logLevel = spdlog::level::level_enum::info;

    std::mutex m_mutex = std::mutex();
};

} // end namespace Global

} // end namespace Model

#endif // _CONTROLLER_GLOBAL_CONFIG_HPP_
