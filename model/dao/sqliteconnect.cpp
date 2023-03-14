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

#include <filesystem>

#include "spdlog/spdlog.h"

#include "sqliteconnect.hpp"
#include "dirutils.hpp"

namespace Model
{

namespace DAO
{

SQLiteToken::~SQLiteToken()
{
    std::unique_lock<std::mutex> lock(mutex);

    if (stmt)
    {
        static_cast<void>(sqlite3_finalize(stmt));
        stmt = nullptr;
    }

    if (db)
    {
        static_cast<void>(sqlite3_close(db));
        db = nullptr;
    }
}

SQLiteConnect::SQLiteConnect()
{}

SQLiteConnect::~SQLiteConnect()
{}

uint_fast8_t SQLiteConnect::init()
{
    m_connectToken = nullptr;
    return 0;
}

uint_fast8_t SQLiteConnect::startConnect(const std::string &target,
                                    const int_fast32_t port)
{
    static_cast<void>(port);
    if (target.empty())
    {
        spdlog::error("{}:{} target is empty.", __FILE__, __LINE__);
        return 1;
    }

    std::string basePath = target;
    DirUtils::convertPath(basePath);
    if (basePath.at(basePath.length() - 1) == '/')
    {
        basePath = basePath.substr(0, basePath.length() - 1);
    }

    if (DirUtils::verifyDir(basePath))
    {
        spdlog::error("{}:{} Fail to verify basePath.", __FILE__, __LINE__);
        return 1;
    }

    m_targetPath = basePath;
    return 0;
}

} // end namespace DAO

} // end namespace Model
