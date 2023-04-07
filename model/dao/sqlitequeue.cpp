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
#include <fstream>

#ifdef _WIN32
#include "windows.h"
#define sleep(x) Sleep(x * 1000)
#else
#include "unistd.h"
#endif

#include "spdlog/spdlog.h"

#include "model/utils.hpp"
#include "sqlitequeue.hpp"
#include "dirutils.hpp"

#define UNUSED(x) static_cast<void>(x)

namespace Model
{

namespace DAO
{

static std::unordered_map<std::string, std::string> dbColumnName;

static std::mutex dbColumnNameMutex;

static bool isDBColumnNameInit = false;

SQLiteQueue::SQLiteQueue() :
    m_token(nullptr)
{}

SQLiteQueue::~SQLiteQueue()
{
    stopImpl();
}

uint_fast8_t SQLiteQueue::init(std::shared_ptr<IConnect> &connect,
                          std::shared_ptr<Proc::IProc> &process,
                          const std::string &name)
{
    static_cast<void>(connect);
    if (process == nullptr)
    {
        spdlog::error("{}:{} process is nullptr.", __FILE__, __LINE__);
        return 1;
    }

    if (name.empty())
    {
        spdlog::error("{}:{} name is empty.", __FILE__, __LINE__);
        return 1;
    }

    try
    {
        m_token = std::make_shared<SQLiteToken>();
    }
    catch (...)
    {
        m_token = nullptr;
        spdlog::error("{}:{} Fail to allocate memory.", __FILE__, __LINE__);
        return 1;
    }

    {
        std::unique_lock<std::mutex> lock(dbColumnNameMutex);
        if (!isDBColumnNameInit)
        {
            dbColumnName["execName"] = "TEXT";
            dbColumnName["args"] = "TEXT";
            dbColumnName["workDir"] = "TEXT";
            dbColumnName["ID"] = "INT";
            dbColumnName["exitCode"] = "INT";
            dbColumnName["isSuccess"] = "INT";
            isDBColumnNameInit = true;
        }
    }

    if (connectToDB(connect->targetPath() + "/" + name + ".db"))
    {
        spdlog::debug("{}:{} path is: {}", __FILE__, __LINE__,
            connect->targetPath() +
            "/" + name + ".sql");
        spdlog::error("{}:{} Fail to connect to SQLite.", __FILE__, __LINE__);
        m_token = nullptr;
        return 1;
    }

    m_process = process;
    m_isRunning.store(false, std::memory_order_relaxed);
    m_start.store(false, std::memory_order_relaxed);
    return 0;
}

uint_fast8_t SQLiteQueue::listPending(std::vector<int> &out)
{
    std::unique_lock<std::mutex> lock(m_token->mutex);
    return listIDInTable("pending", out);
}

uint_fast8_t SQLiteQueue::listFinished(std::vector<int> &out)
{
    std::unique_lock<std::mutex> lock(m_token->mutex);
    return listIDInTable("done", out);
}

uint_fast8_t SQLiteQueue::pendingDetails(const int_fast32_t id, Proc::Task &out)
{
    std::unique_lock<std::mutex> lock(m_token->mutex);
    return taskDetails("pending", id, out);
}

uint_fast8_t SQLiteQueue::finishedDetails(const int_fast32_t id, Proc::Task &out)
{
    std::unique_lock<std::mutex> lock(m_token->mutex);
    return taskDetails("done", id, out);
}

uint_fast8_t SQLiteQueue::clearPending()
{
    std::unique_lock<std::mutex> lock(m_token->mutex);
    std::unique_lock<std::mutex> lock2(m_currentTaskMutex);
    return clearTable("pending");
}

uint_fast8_t SQLiteQueue::clearFinished()
{
    std::unique_lock<std::mutex> lock(m_token->mutex);
    return clearTable("done");
}

uint_fast8_t SQLiteQueue::currentTask(Proc::Task &out)
{
    if (!isRunning())
    {
        spdlog::error("{}:{} Queue is not running.", __FILE__, __LINE__);
        return 1;
    }

    std::unique_lock<std::mutex> lock(m_currentTaskMutex);
    out = m_currentTask;
    return 0;
}

uint_fast8_t SQLiteQueue::addTask(Proc::Task &in)
{
    std::unique_lock<std::mutex> lock(m_token->mutex);
    if (getID(in.ID))
    {
        spdlog::error("{}:{} Fail to get ID", __FILE__, __LINE__);
        return 1;
    }

    return addTaskToTable("pending", in);
}

uint_fast8_t SQLiteQueue::removeTask(const int_fast32_t in)
{
    std::unique_lock<std::mutex> lock(m_token->mutex);
    return removeTaskFromPending(in, true);
}

bool SQLiteQueue::isRunning() const
{
    return m_isRunning.load(std::memory_order_relaxed);
}

uint_fast8_t SQLiteQueue::readCurrentOutput(std::string &out)
{
    if (!isRunning())
    {
        spdlog::error("{}:{} Queue is not running.", __FILE__, __LINE__);
        return 1;
    }

    std::string stdOut = "";
    std::string stdErr = "";

    if (m_process->readCurrentOutput(stdOut))
    {
        spdlog::error("{}:{} Failed to read current output.", __FILE__, __LINE__);
        return 1;
    }

    out = stdOut + stdErr;
    return 0;
}

uint_fast8_t SQLiteQueue::start()
{
    if (isRunning())
    {
        spdlog::error("{}:{} Queue is running.", __FILE__, __LINE__);
        return 1;
    }

    m_isRunning.store(true, std::memory_order_relaxed);
    m_start.store(true, std::memory_order_relaxed);
    m_thread = std::jthread(&SQLiteQueue::mainLoop, this);

    return 0;
}

void SQLiteQueue::stop()
{
    stopImpl();
}

// private member functions
uint_fast8_t SQLiteQueue::connectToDB(const std::string &path)
{
    std::unique_lock<std::mutex> lock(m_token->mutex);
    uint_fast8_t rcPending(0), rcDone(0), rcID(0);

    if (sqlite3_open(path.c_str(), &m_token->db))
    {
        spdlog::error("{}:{} Fail to open SQLite: {}", __FILE__, __LINE__,
            sqlite3_errmsg(m_token->db));
        m_token->db = nullptr;
        return 1;
    }

    rcPending = verifyTable("pending");
    if (rcPending == 1)
    {
        spdlog::error("{}:{} Fail to verifyTable: pending", __FILE__, __LINE__);
        UNUSED(sqlite3_close(m_token->db));
        m_token->db = nullptr;
        return 1;
    }

    rcDone = verifyTable("done");
    if (rcPending == 1)
    {
        spdlog::error("{}:{} Fail to verifyTable: done", __FILE__, __LINE__);
        UNUSED(sqlite3_close(m_token->db));
        m_token->db = nullptr;
        return 1;
    }

    rcID = verifyID();
    if (rcPending == 1)
    {
        spdlog::error("{}:{} Fail to verifyTable: ID", __FILE__, __LINE__);
        UNUSED(sqlite3_close(m_token->db));
        m_token->db = nullptr;
        return 1;
    }

    // The all tables MUST not be exist (rcXXX == 2) in same time
    // or all exist (rcXXX == 0) in the same time
    if (rcPending != rcDone ||
        rcPending != rcID ||
        rcDone != rcID)
    {
        spdlog::error("{}:{} Fail to verifyTable", __FILE__, __LINE__);
        UNUSED(sqlite3_close(m_token->db));
        m_token->db = nullptr;
        return 1;
    }

    return 0;
}

uint_fast8_t SQLiteQueue::createTable(const std::string &name)
{
    uint_fast8_t ret(2);
    std::string sql = "create table ";
    sql += name;
    sql += " ("
        "execName text NOT NULL, "
        "args text NOT NULL, "
        "workDir text NOT NULL, "
        "ID INT NOT NULL PRIMARY KEY, "
        "exitCode INT NOT NULL, "
        "isSuccess INT NOT NULL"
        ");";

    if (sqlite3_prepare_v2(m_token->db,
        sql.c_str(),
        sql.length(),
        &m_token->stmt, NULL))
    {
        spdlog::error("{}:{} Fail to build prepared statment: {}", __FILE__, __LINE__,
            sqlite3_errmsg(m_token->db));
        ret = 1;
        goto exit;
    }

    if (sqlite3_step(m_token->stmt) != SQLITE_DONE)
    {
        spdlog::error("{}:{} Fail to create table", __FILE__, __LINE__);
        ret = 1;
    }

exit:

    UNUSED(sqlite3_finalize(m_token->stmt));
    m_token->stmt = nullptr;
    return ret;
}

uint_fast8_t SQLiteQueue::verifyTable(const std::string &name)
{
    uint_fast8_t ret(0);
    int_fast32_t rowCount(0);
    int_fast32_t rc(0);
    std::string colName, colType;
    std::string sql;
    if (sqlite3_prepare_v2(m_token->db,
        "SELECT name FROM sqlite_master WHERE type='table' AND name=?;", 61,
        &m_token->stmt, NULL))
    {
        spdlog::error("{}:{} Fail to build prepared statment: {}", __FILE__, __LINE__,
            sqlite3_errmsg(m_token->db));
        ret = 1;
        goto exit;
    }

    if (sqlite3_bind_text(m_token->stmt, 1, name.c_str(), name.length(), NULL))
    {
        spdlog::error("{}:{} Fail to build prepared statment: {}", __FILE__, __LINE__,
            sqlite3_errmsg(m_token->db));
        ret = 1;
        goto exit;
    }

    while (1)
    {
        rc = sqlite3_step(m_token->stmt);

        if (rc == SQLITE_ROW)
        {
            ++rowCount;
        }
        else if (rc == SQLITE_DONE)
        {
            break;
        }
        else
        {
            // other error
            spdlog::error("{}:{} Fail to execute sql: {}", __FILE__, __LINE__,
                sqlite3_errmsg(m_token->db));
            ret = 1;
            goto exit;
        }
    }

    UNUSED(sqlite3_finalize(m_token->stmt));
    m_token->stmt = nullptr;

    if (!rowCount)
    {
        return createTable(name);
    }
    else if (rowCount > 1)
    {
        spdlog::error("{}:{} Invalid database", __FILE__, __LINE__);
        return 1;
    }

    rowCount = 0;

    sql = "PRAGMA table_info(\"" + name + "\");";
    // table is exist, check column
    if (sqlite3_prepare_v2(m_token->db,
        sql.c_str(), sql.length(),
        &m_token->stmt, NULL))
    {
        spdlog::error("{}:{} Fail to build prepared statment: {}", __FILE__, __LINE__,
            sqlite3_errmsg(m_token->db));
        ret = 1;
        goto exit;
    }

    while (1)
    {
        rc = sqlite3_step(m_token->stmt);

        if (rc == SQLITE_ROW)
        {
            colName = reinterpret_cast<const char *>(sqlite3_column_text(m_token->stmt, 1));
            colType = reinterpret_cast<const char *>(sqlite3_column_text(m_token->stmt, 2));

            if (dbColumnName[colName] != colType)
            {
                spdlog::error("{}:{} Invalid name & type: {} , {}", __FILE__, __LINE__,
                    colName,
                    colType);
                ret = 1;
                goto exit;
            }

            ++rowCount;
        }
        else if (rc == SQLITE_DONE)
        {
            break;
        }
        else
        {
            // other error
            spdlog::error("{}:{} Fail to execute sql: {}", __FILE__, __LINE__,
                sqlite3_errmsg(m_token->db));
            ret = 1;
            goto exit;
        }
    }

    if (rowCount != 6)
    {
        spdlog::error("{}:{} Invalid table", __FILE__, __LINE__);
        ret = 1;
    }

exit:

    UNUSED(sqlite3_finalize(m_token->stmt));
    m_token->stmt = nullptr;
    return ret;
}

uint_fast8_t SQLiteQueue::verifyID()
{
    uint_fast8_t ret(0);
    int_fast32_t rowCount(0);
    int_fast32_t rc(0);
    std::string colName, colType;
    if (sqlite3_prepare_v2(m_token->db,
        "SELECT name FROM sqlite_master WHERE type='table' AND name='lastID';", 68,
        &m_token->stmt, NULL))
    {
        spdlog::error("{}:{} Fail to build prepared statment: {}", __FILE__, __LINE__,
            sqlite3_errmsg(m_token->db));
        ret = 1;
        goto exit;
    }

    while (1)
    {
        rc = sqlite3_step(m_token->stmt);

        if (rc == SQLITE_ROW)
        {
            ++rowCount;
        }
        else if (rc == SQLITE_DONE)
        {
            break;
        }
        else
        {
            // other error
            spdlog::error("{}:{} Fail to execute sql: {}", __FILE__, __LINE__,
                sqlite3_errmsg(m_token->db));
            ret = 1;
            goto exit;
        }
    }

    UNUSED(sqlite3_finalize(m_token->stmt));
    m_token->stmt = nullptr;

    if (!rowCount)
    {
        // create table "lastID"
        if (sqlite3_prepare_v2(m_token->db,
            "create table lastID "
            "("
            "ID INT NOT NULL PRIMARY KEY"
            ");",
            50,
            &m_token->stmt, NULL))
        {
            spdlog::error("{}:{} Fail to build prepared statment: {}", __FILE__, __LINE__,
                sqlite3_errmsg(m_token->db));
            ret = 1;
            goto exit;
        }

        if (sqlite3_step(m_token->stmt) != SQLITE_DONE)
        {
            spdlog::error("{}:{} Fail to create table", __FILE__, __LINE__);
            ret = 1;
        }

        // insert default data
        UNUSED(sqlite3_finalize(m_token->stmt));
        m_token->stmt = nullptr;

        if (sqlite3_prepare_v2(m_token->db,
            "insert into lastID values(0);",
            29,
            &m_token->stmt, NULL))
        {
            spdlog::error("{}:{} Fail to build prepared statment: {}", __FILE__, __LINE__,
                sqlite3_errmsg(m_token->db));
            ret = 1;
            goto exit;
        }

        if (sqlite3_step(m_token->stmt) != SQLITE_DONE)
        {
            spdlog::error("{}:{} Fail to insert default ID", __FILE__, __LINE__);
            ret = 1;
            goto exit;
        }

        ret = 2;
        goto exit;
    }
    else if (rowCount > 1)
    {
        spdlog::error("{}:{} Invalid database", __FILE__, __LINE__);
        return 1;
    }

    rowCount = 0;

    // table is exist, check column
    if (sqlite3_prepare_v2(m_token->db,
        "PRAGMA table_info('lastID');", 28,
        &m_token->stmt, NULL))
    {
        spdlog::error("{}:{} Fail to build prepared statment: {}", __FILE__, __LINE__,
            sqlite3_errmsg(m_token->db));
        ret = 1;
        goto exit;
    }

    while (1)
    {
        rc = sqlite3_step(m_token->stmt);

        if (rc == SQLITE_ROW)
        {
            colName = reinterpret_cast<const char *>(sqlite3_column_text(m_token->stmt, 1));
            colType = reinterpret_cast<const char *>(sqlite3_column_text(m_token->stmt, 2));

            if (colName != "ID")
            {
                spdlog::error("{}:{} Invalid name: {}", __FILE__, __LINE__,
                    colName);
                ret = 1;
                goto exit;
            }

            if (colType != "INT")
            {
                spdlog::error("{}:{} Invalid type: {}", __FILE__, __LINE__,
                    colType);
                ret = 1;
                goto exit;
            }

            ++rowCount;
        }
        else if (rc == SQLITE_DONE)
        {
            break;
        }
        else
        {
            // other error
            spdlog::error("{}:{} Fail to execute sql: {}", __FILE__, __LINE__,
                sqlite3_errmsg(m_token->db));
            ret = 1;
            goto exit;
        }
    }

    if (rowCount != 1)
    {
        spdlog::error("{}:{} Invalid table", __FILE__, __LINE__);
        ret = 1;
        goto exit;
    }

    UNUSED(sqlite3_finalize(m_token->stmt));
    m_token->stmt = nullptr;
    rowCount = 0;

    if (sqlite3_prepare_v2(m_token->db,
        "SELECT * from lastID;", 21,
        &m_token->stmt, NULL))
    {
        spdlog::error("{}:{} Fail to build prepared statment: {}", __FILE__, __LINE__,
            sqlite3_errmsg(m_token->db));
        ret = 1;
        goto exit;
    }

    while (1)
    {
        rc = sqlite3_step(m_token->stmt);

        if (rc == SQLITE_ROW)
        {
            ++rowCount;
        }
        else if (rc == SQLITE_DONE)
        {
            break;
        }
        else
        {
            // other error
            spdlog::error("{}:{} Fail to execute sql: {}", __FILE__, __LINE__,
                sqlite3_errmsg(m_token->db));
            ret = 1;
            goto exit;
        }
    }

    if (rowCount != 1)
    {
        spdlog::error("{}:{} Invalid table", __FILE__, __LINE__);
        ret = 1;
    }

exit:

    UNUSED(sqlite3_finalize(m_token->stmt));
    m_token->stmt = nullptr;
    return ret;
}

uint_fast8_t SQLiteQueue::clearTable(const std::string &name)
{
    uint_fast8_t ret(0);
    std::string sql = "";

    sql = "DELETE FROM " + name;
    sql += ";";

    if (sqlite3_prepare_v2(m_token->db,
        sql.c_str(), sql.length(),
        &m_token->stmt, NULL))
    {
        spdlog::error("{}:{} Fail to build prepared statment: {}", __FILE__, __LINE__,
            sqlite3_errmsg(m_token->db));
        ret = 1;
        goto exit;
    }

    if (sqlite3_step(m_token->stmt) != SQLITE_DONE)
    {
        spdlog::error("{}:{} Fail to clear table: {}", __FILE__, __LINE__,
            name);
        ret = 1;
    }

exit:

    UNUSED(sqlite3_finalize(m_token->stmt));
    m_token->stmt = nullptr;
    return ret;
}

uint_fast8_t SQLiteQueue::listIDInTable(const std::string &name, std::vector<int> &out)
{
    out.clear();
    out.reserve(128);

    uint_fast8_t ret(0);
    int_fast32_t rc(0);
    std::string sql = "SELECT ID FROM " + name + ";";

    if (sqlite3_prepare_v2(m_token->db,
        sql.c_str(), sql.length(),
        &m_token->stmt, NULL))
    {
        spdlog::error("{}:{} Fail to build prepared statment: {}", __FILE__, __LINE__,
            sqlite3_errmsg(m_token->db));
        ret = 1;
        goto exit;
    }

    while (1)
    {
        rc = sqlite3_step(m_token->stmt);

        if (rc == SQLITE_ROW)
        {
            out.push_back(sqlite3_column_int(m_token->stmt, 0));
        }
        else if (rc == SQLITE_DONE)
        {
            break;
        }
        else
        {
            // other error
            spdlog::error("{}:{} Fail to execute sql: {}", __FILE__, __LINE__,
                sqlite3_errmsg(m_token->db));
            ret = 1;
            goto exit;
        }
    }

exit:

    UNUSED(sqlite3_finalize(m_token->stmt));
    m_token->stmt = nullptr;
    return ret;
}

uint_fast8_t SQLiteQueue::taskDetails(const std::string &name, const int_fast32_t id, Proc::Task &out)
{
    uint_fast8_t ret(0);
    int_fast32_t rc(0);
    int_fast32_t rowCount(0);
    std::string sql = "SELECT * FROM " + name + " WHERE ID=?;";


    if (sqlite3_prepare_v2(m_token->db,
        sql.c_str(), sql.length(),
        &m_token->stmt, NULL))
    {
        spdlog::error("{}:{} Fail to build prepared statment: {}", __FILE__, __LINE__,
            sqlite3_errmsg(m_token->db));
        ret = 1;
        goto exit;
    }

    if (sqlite3_bind_int(m_token->stmt, 1, id))
    {
        spdlog::error("{}:{} Fail to build prepared statment: {}", __FILE__, __LINE__,
            sqlite3_errmsg(m_token->db));
        ret = 1;
        goto exit;
    }

    while (1)
    {
        rc = sqlite3_step(m_token->stmt);

        if (rc == SQLITE_ROW)
        {
            out.execName = reinterpret_cast<const char *>(sqlite3_column_text(m_token->stmt, 0));
            splitString(reinterpret_cast<const char *>(sqlite3_column_text(m_token->stmt, 1)),
                out.args);
            out.workDir = reinterpret_cast<const char *>(sqlite3_column_text(m_token->stmt, 2));
            out.ID = sqlite3_column_int(m_token->stmt, 3);
            out.exitCode = sqlite3_column_int(m_token->stmt, 4);
            out.isSuccess = sqlite3_column_int(m_token->stmt, 5);

            ++rowCount;
        }
        else if (rc == SQLITE_DONE)
        {
            break;
        }
        else
        {
            // other error
            spdlog::error("{}:{} Fail to execute sql: {}", __FILE__, __LINE__,
                sqlite3_errmsg(m_token->db));
            ret = 1;
            goto exit;
        }
    }

    if (rowCount == 0)
    {
        spdlog::error("{}:{} No such ID in table {}: {}", __FILE__, __LINE__,
            name, id);
        ret = 1;
    }

exit:

    UNUSED(sqlite3_finalize(m_token->stmt));
    m_token->stmt = nullptr;
    return ret;
}

uint_fast8_t SQLiteQueue::addTaskToTable(const std::string &name, const Proc::Task &in)
{
    uint_fast8_t ret(0);
    std::string args = "";
    std::string sql = "insert into " + name + " ";
    sql += "values(?,?,?,?,?,?);";

    if (sqlite3_prepare_v2(m_token->db,
        sql.c_str(), sql.length(),
        &m_token->stmt, NULL))
    {
        spdlog::error("{}:{} Fail to build prepared statment: {}", __FILE__, __LINE__,
            sqlite3_errmsg(m_token->db));
        ret = 1;
        goto exit;
    }

    if (sqlite3_bind_text(m_token->stmt, 1, in.execName.c_str(), in.execName.length(), NULL))
    {
        spdlog::error("{}:{} Fail to build prepared statment: {}", __FILE__, __LINE__,
            sqlite3_errmsg(m_token->db));
        ret = 1;
        goto exit;
    }

    args = concatString(in.args);
    if (sqlite3_bind_text(m_token->stmt, 2, args.c_str(), args.length(), NULL))
    {
        spdlog::error("{}:{} Fail to build prepared statment: {}", __FILE__, __LINE__,
            sqlite3_errmsg(m_token->db));
        ret = 1;
        goto exit;
    }

    if (sqlite3_bind_text(m_token->stmt, 3, in.workDir.c_str(), in.workDir.length(), NULL))
    {
        spdlog::error("{}:{} Fail to build prepared statment: {}", __FILE__, __LINE__,
            sqlite3_errmsg(m_token->db));
        ret = 1;
        goto exit;
    }

    if (sqlite3_bind_int(m_token->stmt, 4, in.ID))
    {
        spdlog::error("{}:{} Fail to build prepared statment: {}", __FILE__, __LINE__,
            sqlite3_errmsg(m_token->db));
        ret = 1;
        goto exit;
    }

    if (sqlite3_bind_int(m_token->stmt, 5, in.exitCode))
    {
        spdlog::error("{}:{} Fail to build prepared statment: {}", __FILE__, __LINE__,
            sqlite3_errmsg(m_token->db));
        ret = 1;
        goto exit;
    }

    if (sqlite3_bind_int(m_token->stmt, 6, in.isSuccess))
    {
        spdlog::error("{}:{} Fail to build prepared statment: {}", __FILE__, __LINE__,
            sqlite3_errmsg(m_token->db));
        ret = 1;
        goto exit;
    }

    if (sqlite3_step(m_token->stmt) != SQLITE_DONE)
    {
        spdlog::error("{}:{} Fail to insert task to table {}", __FILE__, __LINE__,
            name);
        ret = 1;
    }

exit:

    UNUSED(sqlite3_finalize(m_token->stmt));
    m_token->stmt = nullptr;
    return ret;
}

uint_fast8_t SQLiteQueue::removeTaskFromPending(const int_fast32_t id,
    const bool needCheckCurrentTask)
{
    if (needCheckCurrentTask)
    {
        std::unique_lock<std::mutex> lock(m_currentTaskMutex);
        if (id == m_currentTask.ID)
        {
            spdlog::error("{}:{} Cannot remove current task", __FILE__, __LINE__);
            return 1;
        }
    }

    uint_fast8_t ret(0);
    if (sqlite3_prepare_v2(m_token->db,
        "delete from pending where ID=?;", 31,
        &m_token->stmt, NULL))
    {
        spdlog::error("{}:{} Fail to build prepared statment: {}", __FILE__, __LINE__,
            sqlite3_errmsg(m_token->db));
        ret = 1;
        goto exit;
    }

    if (sqlite3_bind_int(m_token->stmt, 1, id))
    {
        spdlog::error("{}:{} Fail to build prepared statment: {}", __FILE__, __LINE__,
            sqlite3_errmsg(m_token->db));
        ret = 1;
        goto exit;
    }

    if (sqlite3_step(m_token->stmt) != SQLITE_DONE)
    {
        spdlog::error("{}:{} Fail to remove task", __FILE__, __LINE__);
        ret = 2;
    }

exit:

    UNUSED(sqlite3_finalize(m_token->stmt));
    m_token->stmt = nullptr;
    return ret;
}

void SQLiteQueue::splitString(const std::string &in, std::vector<std::string> &out)
{
    std::string s = in;
    std::string delimiter = "__,__";
    out.clear();
    out.reserve(128);

    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos)
    {
        out.push_back(s.substr(0, pos));
        s.erase(0, pos + delimiter.length());
    }
}

std::string SQLiteQueue::concatString(const std::vector<std::string> &in)
{
    std::string out = "";
    size_t last(0);
    if (!in.size())
    {
        goto exit;
    }

    last = in.size() - 2;
    for (size_t i = 0; i < last; ++i)
    {
        out += (in.at(i) + "__,__");
    }

    out += in.at(in.size() - 1);

exit:
    return out;
}

uint_fast8_t SQLiteQueue::getID(int_fast32_t &out)
{
    uint_fast8_t ret(0);
    int_fast32_t rc(0);
    int_fast32_t rowCount(0);
    int_fast32_t oldValue(0), newValue(0);
    if (sqlite3_prepare_v2(m_token->db,
        "SELECT * FROM lastID;", 21,
        &m_token->stmt, NULL))
    {
        spdlog::error("{}:{} Fail to build prepared statment: {}", __FILE__, __LINE__,
            sqlite3_errmsg(m_token->db));
        ret = 1;
        goto exit;
    }

    while (1)
    {
        rc = sqlite3_step(m_token->stmt);

        if (rc == SQLITE_ROW)
        {
            out = sqlite3_column_int(m_token->stmt, 0);
            ++rowCount;
        }
        else if (rc == SQLITE_DONE)
        {
            break;
        }
        else
        {
            // other error
            spdlog::error("{}:{} Fail to execute sql: {}", __FILE__, __LINE__,
                sqlite3_errmsg(m_token->db));
            ret = 1;
            goto exit;
        }
    }

    if (rowCount != 1)
    {
        spdlog::error("{}:{} Invalid table", __FILE__, __LINE__);
        ret = 1;
        goto exit;
    }

    UNUSED(sqlite3_finalize(m_token->stmt));
    m_token->stmt = nullptr;
    oldValue = out;
    newValue = out + 1;

    if (sqlite3_prepare_v2(m_token->db,
        "update lastID set ID=? where ID=?;", 34,
        &m_token->stmt, NULL))
    {
        spdlog::error("{}:{} Fail to build prepared statment: {}", __FILE__, __LINE__,
            sqlite3_errmsg(m_token->db));
        ret = 1;
        goto exit;
    }

    if (sqlite3_bind_int(m_token->stmt, 1, newValue))
    {
        spdlog::error("{}:{} Fail to build prepared statment: {}", __FILE__, __LINE__,
            sqlite3_errmsg(m_token->db));
        ret = 1;
        goto exit;
    }

    if (sqlite3_bind_int(m_token->stmt, 2, oldValue))
    {
        spdlog::error("{}:{} Fail to build prepared statment: {}", __FILE__, __LINE__,
            sqlite3_errmsg(m_token->db));
        ret = 1;
        goto exit;
    }

    if (sqlite3_step(m_token->stmt) != SQLITE_DONE)
    {
        spdlog::error("{}:{} Fail to update last id", __FILE__, __LINE__);
        ret = 1;
    }

exit:

    UNUSED(sqlite3_finalize(m_token->stmt));
    m_token->stmt = nullptr;
    return ret;
}

void SQLiteQueue::mainLoop()
{
    while (m_start.load(std::memory_order_relaxed))
    {
        if (mainLoopInit())
        {
            continue;
        }

        // invoke process
        if (m_process->start(m_currentTask))
        {
            spdlog::error("{}:{} Fail to start process.", __FILE__, __LINE__);
            m_start.store(false, std::memory_order_relaxed);
            continue;
        }

        while(m_process->isRunning())
        {
            sleep(1);
        }

        mainLoopFin();
    } // end while (m_start.load(std::memory_order_relaxed))

    m_isRunning.store(false, std::memory_order_relaxed);
} // end void DirQueue::mainLoop()

uint_fast8_t SQLiteQueue::mainLoopInit()
{
    // find the task in pending list
    std::unique_lock<std::mutex> lock(m_token->mutex);
    uint_fast8_t ret(0);

    if (sqlite3_prepare_v2(m_token->db,
        "SELECT * FROM pending limit 1;", 30,
        &m_token->stmt, NULL))
    {
        spdlog::error("{}:{} Fail to build prepared statment: {}", __FILE__, __LINE__,
            sqlite3_errmsg(m_token->db));
        ret = 1;
        goto exit;
    }

    switch (sqlite3_step(m_token->stmt))
    {
    case SQLITE_ROW:
    {
        std::unique_lock<std::mutex> lock(m_currentTaskMutex);
        m_currentTask.execName = reinterpret_cast<const char *>(sqlite3_column_text(m_token->stmt, 0));
        splitString(reinterpret_cast<const char *>(sqlite3_column_text(m_token->stmt, 1)),
            m_currentTask.args);
        m_currentTask.workDir = reinterpret_cast<const char *>(sqlite3_column_text(m_token->stmt, 2));
        m_currentTask.ID = sqlite3_column_int(m_token->stmt, 3);
        m_currentTask.exitCode = sqlite3_column_int(m_token->stmt, 4);
        m_currentTask.isSuccess = sqlite3_column_int(m_token->stmt, 5);
        break;
    }
    case SQLITE_DONE:
    {
        spdlog::error("{}:{} Pending list is empty", __FILE__, __LINE__);
        m_start.store(false, std::memory_order_relaxed);
        m_isRunning.store(false, std::memory_order_relaxed);
        ret = 1;
        break;
    }
    default:
    {
        // other error
        spdlog::error("{}:{} Fail to execute sql: {}", __FILE__, __LINE__,
            sqlite3_errmsg(m_token->db));
        m_start.store(false, std::memory_order_relaxed);
        m_isRunning.store(false, std::memory_order_relaxed);
        ret = 1;
        break;
    }
    }

exit:

    UNUSED(sqlite3_finalize(m_token->stmt));
    m_token->stmt = nullptr;
    return ret;
}

void SQLiteQueue::mainLoopFin()
{
    {
        std::unique_lock<std::mutex> lock(m_currentTaskMutex);
        if (m_process->exitCode(m_currentTask.exitCode))
        {
            spdlog::error("{}:{} Fail to get exit code.", __FILE__, __LINE__);
            m_start.store(false, std::memory_order_relaxed);
            m_currentTask = Proc::Task();
            return;
        }

        // write task details to done list
        std::unique_lock<std::mutex> dbLock(m_token->mutex);
        if (removeTaskFromPending(m_currentTask.ID, false) == 1)
        {
            // Note that return code is 2 is also acceptable
            // Because that may be removed by clearing the pending list
            spdlog::error("{}:{} Fail to remove task from pending", __FILE__, __LINE__);
            m_start.store(false, std::memory_order_relaxed);
            m_currentTask = Proc::Task();
            return;
        }

        if (addTaskToTable("done", m_currentTask))
        {
            spdlog::error("{}:{} Fail to add task to done list", __FILE__, __LINE__);
            m_start.store(false, std::memory_order_relaxed);
            m_currentTask = Proc::Task();
            return;
        }

        m_currentTask = Proc::Task();
    }
}

void SQLiteQueue::stopImpl()
{
    if (!isRunning())
    {
        spdlog::error("{}:{} Queue is not running.", __FILE__, __LINE__);
        return;
    }

    m_start.store(false, std::memory_order_relaxed);
    m_process->stop();
    m_isRunning.store(false, std::memory_order_relaxed);
}

} // end namespace DAO

} // end namespace Model
