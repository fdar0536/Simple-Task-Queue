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

#include <stdio.h>

#ifdef _WIN32
#include "windows.h"
#include "io.h"
#include "fcntl.h"
#endif

#include "spdlog/spdlog.h"

#include "model/utils.hpp"

#include "init.hpp"

#ifdef STQ_GUI
#include "controller/gui/global.hpp"
#endif

namespace Controller
{

namespace Global
{

Config config;

#ifndef STQ_MOBILE
GRPCServer::Server server;

std::shared_ptr<Model::DAO::IQueueList> sqliteQueueList;
#endif

#ifdef STQ_GUI
Controller::GUI::Global guiGlobal;
#endif

#ifndef STQ_GUI
#ifdef _WIN32
static UINT consoleCP(0);

static UINT consoleOutputCP(0);
#endif
#endif

static uint_fast8_t initConsole();

#ifndef STQ_MOBILE
static uint_fast8_t initSQLiteQueueList();
#endif

uint_fast8_t init(int argc, char **argv)
{
    if (initConsole())
    {
        spdlog::error("{}:{} initConsole failed", __FILE__, __LINE__);
        return 1;
    }

    if (Config::parse(&config, argc, argv))
    {
        spdlog::error("{}:{} parse config failed", __FILE__, __LINE__);
        return 1;
    }

#ifndef STQ_MOBILE
    if (initSQLiteQueueList())
    {
        spdlog::error("{}:{} Fail to initialize sqlite queue list", __FILE__, __LINE__);
        return 1;
    }
#endif

    Model::ErrMsg::init();
    return 0;
}

void fin()
{
#ifndef STQ_GUI
#ifdef _WIN32
    SetConsoleCP(consoleCP);
    SetConsoleOutputCP(consoleOutputCP);
#endif
#endif
}

#ifndef STQ_MOBILE
static uint_fast8_t initSQLiteQueueList()
{
    Model::DAO::SQLiteConnect *conn(nullptr);
    try
    {
        conn = new Model::DAO::SQLiteConnect();
        if (conn == nullptr)
        {
            spdlog::error("{}:{} Fail to allocate memory", __FILE__, __LINE__);
            return 1;
        }
    }
    catch(...)
    {
        spdlog::error("{}:{} Fail to allocate memory", __FILE__, __LINE__);
        return 1;
    }

    Model::ErrMsg msg;
    Model::ErrMsg::ErrCode code;
    conn->startConnect(msg, config.dbPath());
    msg.msg(&code, nullptr);
    if (code != Model::ErrMsg::OK)
    {
        delete conn;
        spdlog::error("{}:{} Fail to initialize sqlite", __FILE__, __LINE__);
        return 1;
    }

    auto connPtr = std::shared_ptr<Model::DAO::IConnect>(conn);
    Model::DAO::SQLiteQueueList *sqlPtr;
    try
    {
        sqlPtr = new Model::DAO::SQLiteQueueList;
    }
    catch (...)
    {
        spdlog::error("{}:{} Fail to allocate memory", __FILE__, __LINE__);
        return 1;
    }

    sqlPtr->init(connPtr, msg);
    msg.msg(&code, nullptr);
    if (code != Model::ErrMsg::OK)
    {
        spdlog::error("{}:{} Fail to initialize sqlite queue list", __FILE__, __LINE__);
        delete sqlPtr;
        return 1;
    }

    sqliteQueueList = std::shared_ptr<Model::DAO::IQueueList>(sqlPtr);
    return 0;
}
#endif

static uint_fast8_t initConsole()
{
    fflush(stdout);
#ifndef STQ_GUI
#ifdef _MSC_VER
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

#ifdef _MSC_VER
#   pragma endregion
#endif
#endif // STQ_GUI
    std::ios::sync_with_stdio(false);
    return 0;
}

} // end namespace Global

} // end namespace Model
