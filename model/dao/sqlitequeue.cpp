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

void SQLiteQueue::init(std::shared_ptr<IConnect> &connect,
                       std::shared_ptr<Proc::IProc> &process,
                       const std::string &name,
                       ErrMsg &msg)
{
    static_cast<void>(connect);
    if (process == nullptr)
    {
        msg.setMsg(ErrMsg::INVALID_ARGUMENT, "process is nullptr");
        spdlog::error("{}:{} process is nullptr.", __FILE__, __LINE__);
        return;
    }

    if (name.empty())
    {
        msg.setMsg(ErrMsg::INVALID_ARGUMENT, "name is empty");
        spdlog::error("{}:{} name is empty.", __FILE__, __LINE__);
        return;
    }

    try
    {
        m_token = std::make_shared<SQLiteToken>();
    }
    catch (...)
    {
        m_token = nullptr;
        msg.setMsg(ErrMsg::OS_ERROR, "Fail to allocate memory");
        spdlog::error("{}:{} Fail to allocate memory.", __FILE__, __LINE__);
        return;
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
        msg.setMsg(ErrMsg::OS_ERROR, "Fail to connect to SQLite");
        spdlog::error("{}:{} Fail to connect to SQLite.", __FILE__, __LINE__);
        m_token = nullptr;
        return;
    }

    m_process = process;
    m_isRunning.store(false, std::memory_order_relaxed);
    m_start.store(false, std::memory_order_relaxed);
}

void SQLiteQueue::listPending(std::vector<int> &out, ErrMsg &msg)
{
    std::unique_lock<std::mutex> lock(m_token->mutex);
    listIDInTable("pending", out, msg);
}

void SQLiteQueue::listFinished(std::vector<int> &out, ErrMsg &msg)
{
    std::unique_lock<std::mutex> lock(m_token->mutex);
    listIDInTable("done", out, msg);
}

void SQLiteQueue::pendingDetails(const int_fast32_t id,
                                 Proc::Task &out,
                                 ErrMsg &msg)
{
    std::unique_lock<std::mutex> lock(m_token->mutex);
    taskDetails("pending", id, out, msg);
}

void SQLiteQueue::finishedDetails(const int_fast32_t id,
                                  Proc::Task &out,
                                  ErrMsg &msg)
{
    std::unique_lock<std::mutex> lock(m_token->mutex);
    taskDetails("done", id, out, msg);
}

void SQLiteQueue::clearPending(ErrMsg &msg)
{
    std::unique_lock<std::mutex> lock(m_token->mutex);
    std::unique_lock<std::mutex> lock2(m_currentTaskMutex);
    clearTable("pending", msg);
}

void SQLiteQueue::clearFinished(ErrMsg &msg)
{
    std::unique_lock<std::mutex> lock(m_token->mutex);
    clearTable("done", msg);
}

void SQLiteQueue::currentTask(Proc::Task &out, ErrMsg &msg)
{
    if (!isRunning())
    {
        msg.setMsg(ErrMsg::INVALID_ARGUMENT, "Queue is not running");
        spdlog::error("{}:{} Queue is not running.", __FILE__, __LINE__);
        return;
    }

    std::unique_lock<std::mutex> lock(m_currentTaskMutex);
    out = m_currentTask;
}

void SQLiteQueue::addTask(Proc::Task &in, ErrMsg &msg)
{
    std::unique_lock<std::mutex> lock(m_token->mutex);
    ErrMsg::ErrCode code;
    std::string errMsg;
    getID(in.ID, msg);
    msg.msg(code, errMsg);
    if (code != ErrMsg::OK)
    {
        msg.setMsg(ErrMsg::OS_ERROR, "Fail to get ID");
        spdlog::error("{}:{} Fail to get ID", __FILE__, __LINE__);
        return;
    }

    addTaskToTable("pending", in, msg);
}

void SQLiteQueue::removeTask(const int_fast32_t in, ErrMsg &msg)
{
    std::unique_lock<std::mutex> lock(m_token->mutex);
    removeTaskFromPending(in, true, msg);
}

bool SQLiteQueue::isRunning() const
{
    return m_isRunning.load(std::memory_order_relaxed);
}

void SQLiteQueue::readCurrentOutput(std::string &out, ErrMsg &msg)
{
    if (!isRunning())
    {
        msg.setMsg(ErrMsg::INVALID_ARGUMENT, "Queue is not running");
        spdlog::error("{}:{} Queue is not running.", __FILE__, __LINE__);
        return;
    }

    out.clear();
    if (m_process->readCurrentOutput(out))
    {
        msg.setMsg(ErrMsg::OS_ERROR, "Failed to read current output");
        spdlog::error("{}:{} Failed to read current output.", __FILE__, __LINE__);
    }
}

void SQLiteQueue::start(ErrMsg &msg)
{
    if (isRunning())
    {
        msg.setMsg(ErrMsg::INVALID_ARGUMENT, "Queue is running");
        spdlog::error("{}:{} Queue is running.", __FILE__, __LINE__);
        return;
    }

    m_isRunning.store(true, std::memory_order_relaxed);
    m_start.store(true, std::memory_order_relaxed);
    m_thread = std::jthread(&SQLiteQueue::mainLoop, this);
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

void SQLiteQueue::clearTable(const std::string &name, ErrMsg &msg)
{
    std::string sql = "";

    sql = "DELETE FROM " + name;
    sql += ";";

    if (sqlite3_prepare_v2(m_token->db,
        sql.c_str(), sql.length(),
        &m_token->stmt, NULL))
    {
        msg.setMsg(ErrMsg::OS_ERROR, "Fail to build prepared statment");
        spdlog::error("{}:{} Fail to build prepared statment: {}", __FILE__, __LINE__,
            sqlite3_errmsg(m_token->db));
        goto exit;
    }

    if (sqlite3_step(m_token->stmt) != SQLITE_DONE)
    {
        msg.setMsg(ErrMsg::OS_ERROR, "Fail to clear table");
        spdlog::error("{}:{} Fail to clear table: {}", __FILE__, __LINE__,
            name);
    }

exit:

    UNUSED(sqlite3_finalize(m_token->stmt));
    m_token->stmt = nullptr;
}

void SQLiteQueue::listIDInTable(const std::string &name,
                                std::vector<int> &out,
                                ErrMsg &msg)
{
    out.clear();
    out.reserve(128);

    int_fast32_t rc(0);
    std::string sql = "SELECT ID FROM " + name + ";";

    if (sqlite3_prepare_v2(m_token->db,
        sql.c_str(), sql.length(),
        &m_token->stmt, NULL))
    {
        msg.setMsg(ErrMsg::OS_ERROR, "Fail to build prepared statment");
        spdlog::error("{}:{} Fail to build prepared statment: {}", __FILE__, __LINE__,
            sqlite3_errmsg(m_token->db));
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
            msg.setMsg(ErrMsg::OS_ERROR, "Fail to execute sql");
            spdlog::error("{}:{} Fail to execute sql: {}", __FILE__, __LINE__,
                sqlite3_errmsg(m_token->db));
            goto exit;
        }
    }

exit:

    UNUSED(sqlite3_finalize(m_token->stmt));
    m_token->stmt = nullptr;
}

void SQLiteQueue::taskDetails(const std::string &name,
                              const int_fast32_t id,
                              Proc::Task &out,
                              ErrMsg &msg)
{
    int_fast32_t rc(0);
    int_fast32_t rowCount(0);
    std::string sql = "SELECT * FROM " + name + " WHERE ID=?;";


    if (sqlite3_prepare_v2(m_token->db,
        sql.c_str(), sql.length(),
        &m_token->stmt, NULL))
    {
        msg.setMsg(ErrMsg::OS_ERROR, "Fail to build prepared statment");
        spdlog::error("{}:{} Fail to build prepared statment: {}", __FILE__, __LINE__,
            sqlite3_errmsg(m_token->db));
        goto exit;
    }

    if (sqlite3_bind_int(m_token->stmt, 1, id))
    {
        msg.setMsg(ErrMsg::OS_ERROR, "Fail to build prepared statment");
        spdlog::error("{}:{} Fail to build prepared statment: {}", __FILE__, __LINE__,
            sqlite3_errmsg(m_token->db));
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
            msg.setMsg(ErrMsg::OS_ERROR, "Fail to execute sql");
            spdlog::error("{}:{} Fail to execute sql: {}", __FILE__, __LINE__,
                sqlite3_errmsg(m_token->db));
            goto exit;
        }
    }

    if (rowCount == 0)
    {
        msg.setMsg(ErrMsg::NOT_FOUND, "No such ID in table");
        spdlog::error("{}:{} No such ID in table {}: {}", __FILE__, __LINE__,
            name, id);
    }

exit:

    UNUSED(sqlite3_finalize(m_token->stmt));
    m_token->stmt = nullptr;
}

void SQLiteQueue::addTaskToTable(const std::string &name,
                                 const Proc::Task &in,
                                 ErrMsg &msg)
{
    std::string args = "";
    std::string sql = "insert into " + name + " ";
    sql += "values(?,?,?,?,?,?);";

    if (sqlite3_prepare_v2(m_token->db,
        sql.c_str(), sql.length(),
        &m_token->stmt, NULL))
    {
        msg.setMsg(ErrMsg::OS_ERROR, "Fail to build prepared statment");
        spdlog::error("{}:{} Fail to build prepared statment: {}", __FILE__, __LINE__,
            sqlite3_errmsg(m_token->db));
        goto exit;
    }

    if (sqlite3_bind_text(m_token->stmt, 1, in.execName.c_str(), in.execName.length(), NULL))
    {
        msg.setMsg(ErrMsg::OS_ERROR, "Fail to build prepared statment");
        spdlog::error("{}:{} Fail to build prepared statment: {}", __FILE__, __LINE__,
            sqlite3_errmsg(m_token->db));
        goto exit;
    }

    args = concatString(in.args);
    if (sqlite3_bind_text(m_token->stmt, 2, args.c_str(), args.length(), NULL))
    {
        msg.setMsg(ErrMsg::OS_ERROR, "Fail to build prepared statment");
        spdlog::error("{}:{} Fail to build prepared statment: {}", __FILE__, __LINE__,
            sqlite3_errmsg(m_token->db));
        goto exit;
    }

    if (sqlite3_bind_text(m_token->stmt, 3, in.workDir.c_str(), in.workDir.length(), NULL))
    {
        msg.setMsg(ErrMsg::OS_ERROR, "Fail to build prepared statment");
        spdlog::error("{}:{} Fail to build prepared statment: {}", __FILE__, __LINE__,
            sqlite3_errmsg(m_token->db));
        goto exit;
    }

    if (sqlite3_bind_int(m_token->stmt, 4, in.ID))
    {
        msg.setMsg(ErrMsg::OS_ERROR, "Fail to build prepared statment");
        spdlog::error("{}:{} Fail to build prepared statment: {}", __FILE__, __LINE__,
            sqlite3_errmsg(m_token->db));
        goto exit;
    }

    if (sqlite3_bind_int(m_token->stmt, 5, in.exitCode))
    {
        msg.setMsg(ErrMsg::OS_ERROR, "Fail to build prepared statment");
        spdlog::error("{}:{} Fail to build prepared statment: {}", __FILE__, __LINE__,
            sqlite3_errmsg(m_token->db));
        goto exit;
    }

    if (sqlite3_bind_int(m_token->stmt, 6, in.isSuccess))
    {
        msg.setMsg(ErrMsg::OS_ERROR, "Fail to build prepared statment");
        spdlog::error("{}:{} Fail to build prepared statment: {}", __FILE__, __LINE__,
            sqlite3_errmsg(m_token->db));
        goto exit;
    }

    if (sqlite3_step(m_token->stmt) != SQLITE_DONE)
    {
        msg.setMsg(ErrMsg::OS_ERROR, "Fail to insert task to table");
        spdlog::error("{}:{} Fail to insert task to table {}", __FILE__, __LINE__,
            name);
    }

exit:

    UNUSED(sqlite3_finalize(m_token->stmt));
    m_token->stmt = nullptr;
}

void SQLiteQueue::removeTaskFromPending(const int_fast32_t id,
                                        const bool needCheckCurrentTask,
                                        ErrMsg &msg)
{
    if (needCheckCurrentTask)
    {
        std::unique_lock<std::mutex> lock(m_currentTaskMutex);
        if (id == m_currentTask.ID)
        {
            msg.setMsg(ErrMsg::INVALID_ARGUMENT, "Cannot remove current task");
            spdlog::error("{}:{} Cannot remove current task", __FILE__, __LINE__);
            return;
        }
    }

    if (sqlite3_prepare_v2(m_token->db,
        "delete from pending where ID=?;", 31,
        &m_token->stmt, NULL))
    {
        msg.setMsg(ErrMsg::OS_ERROR, "Fail to build prepared statment");
        spdlog::error("{}:{} Fail to build prepared statment: {}", __FILE__, __LINE__,
            sqlite3_errmsg(m_token->db));
        goto exit;
    }

    if (sqlite3_bind_int(m_token->stmt, 1, id))
    {
        msg.setMsg(ErrMsg::OS_ERROR, "Fail to build prepared statment");
        spdlog::error("{}:{} Fail to build prepared statment: {}", __FILE__, __LINE__,
            sqlite3_errmsg(m_token->db));
        goto exit;
    }

    if (sqlite3_step(m_token->stmt) != SQLITE_DONE)
    {
        msg.setMsg(ErrMsg::NOT_FOUND, "Fail to remove task");
        spdlog::error("{}:{} Fail to remove task", __FILE__, __LINE__);
    }

exit:

    UNUSED(sqlite3_finalize(m_token->stmt));
    m_token->stmt = nullptr;
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

void SQLiteQueue::getID(int_fast32_t &out, ErrMsg &msg)
{
    int_fast32_t rc(0);
    int_fast32_t rowCount(0);
    int_fast32_t oldValue(0), newValue(0);
    if (sqlite3_prepare_v2(m_token->db,
        "SELECT * FROM lastID;", 21,
        &m_token->stmt, NULL))
    {
        msg.setMsg(ErrMsg::OS_ERROR, "Fail to build prepared statment");
        spdlog::error("{}:{} Fail to build prepared statment: {}", __FILE__, __LINE__,
            sqlite3_errmsg(m_token->db));
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
            msg.setMsg(ErrMsg::OS_ERROR, "Fail to execute sql");
            spdlog::error("{}:{} Fail to execute sql: {}", __FILE__, __LINE__,
                sqlite3_errmsg(m_token->db));
            goto exit;
        }
    }

    if (rowCount != 1)
    {
        msg.setMsg(ErrMsg::INVALID_ARGUMENT, "Invalid table");
        spdlog::error("{}:{} Invalid table", __FILE__, __LINE__);
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
        msg.setMsg(ErrMsg::OS_ERROR, "Fail to build prepared statment");
        spdlog::error("{}:{} Fail to build prepared statment: {}", __FILE__, __LINE__,
            sqlite3_errmsg(m_token->db));
        goto exit;
    }

    if (sqlite3_bind_int(m_token->stmt, 1, newValue))
    {
        msg.setMsg(ErrMsg::OS_ERROR, "Fail to build prepared statment");
        spdlog::error("{}:{} Fail to build prepared statment: {}", __FILE__, __LINE__,
            sqlite3_errmsg(m_token->db));
        goto exit;
    }

    if (sqlite3_bind_int(m_token->stmt, 2, oldValue))
    {
        msg.setMsg(ErrMsg::OS_ERROR, "Fail to build prepared statment");
        spdlog::error("{}:{} Fail to build prepared statment: {}", __FILE__, __LINE__,
            sqlite3_errmsg(m_token->db));
        goto exit;
    }

    if (sqlite3_step(m_token->stmt) != SQLITE_DONE)
    {
        msg.setMsg(ErrMsg::OS_ERROR, "Fail to update last id");
        spdlog::error("{}:{} Fail to update last id", __FILE__, __LINE__);
    }

exit:

    UNUSED(sqlite3_finalize(m_token->stmt));
    m_token->stmt = nullptr;
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
    ErrMsg msg;
    ErrMsg::ErrCode code;
    std::string errMsg;
    removeTaskFromPending(m_currentTask.ID, false, msg);
    msg.msg(code, errMsg);
    if (code == ErrMsg::INVALID_ARGUMENT ||
        code == ErrMsg::OS_ERROR)
    {
        spdlog::error("{}:{} Fail to remove task from pending", __FILE__, __LINE__);
        m_start.store(false, std::memory_order_relaxed);
        m_currentTask = Proc::Task();
        return;
    }

    addTaskToTable("done", m_currentTask, msg);
    msg.msg(code, errMsg);
    if (code != ErrMsg::OK)
    {
        spdlog::error("{}:{} Fail to add task to done list", __FILE__, __LINE__);
        m_start.store(false, std::memory_order_relaxed);
        m_currentTask = Proc::Task();
        return;
    }

    m_currentTask = Proc::Task();
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
