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
#include <new>

#include "spdlog/spdlog.h"

#include "sqlitequeue.hpp"
#include "sqlitequeuelist.hpp"
#include "dirutils.hpp"

#ifdef _WIN32
#include "model/proc/winproc.hpp"
#else
#include "model/proc/posixproc.hpp"
#endif

namespace Model
{

namespace DAO
{

SQLiteQueueList::SQLiteQueueList()
{}

SQLiteQueueList::~SQLiteQueueList()
{}

void
SQLiteQueueList::init(std::shared_ptr<IConnect> &connect, ErrMsg &msg)
{
    if (connect == nullptr)
    {
        msg.setMsg(ErrMsg::INVALID_ARGUMENT, "\"connect\" is nullptr");
        spdlog::error("{}:{} \"connect\" is nullptr.", __FILE__, __LINE__);
        return;
    }

    if (DirUtils::verifyDir(connect->targetPath()))
    {
        msg.setMsg(ErrMsg::INVALID_ARGUMENT, "Fail to verify target path");
        spdlog::error("{}:{} Fail to verify target path.", __FILE__, __LINE__);
        return;
    }

    // create queue
    std::error_code ec;
    std::string fileName;
    m_queueList.clear();
    m_conn = connect;
    ErrMsg::ErrCode code;
    for (const auto& entry : std::filesystem::directory_iterator(connect->targetPath()))
    {
        if (std::filesystem::is_directory(entry))
        {
            spdlog::warn("{}:{} {} is directory, ignore...", __FILE__, __LINE__,
                         entry.path().string());
            continue;
        }

        if (!std::filesystem::is_regular_file(entry))
        {
            spdlog::warn("{}:{} {} is not regular file, ignore...", __FILE__, __LINE__,
                entry.path().string());
            continue;
        }

        // regular file
        fileName = entry.path().string();
        DirUtils::convertPath(fileName);
        size_t index = fileName.find_last_of("/");
        std::string name = fileName.substr(index + 1);
        index = name.find_last_of(".");
        if (name.substr(index + 1) != "db")
        {
            spdlog::warn("{}:{} {} is not database, ignore...", __FILE__, __LINE__,
                fileName);
            continue;
        }

        name = name.substr(0, index);
        createQueue(name, msg);
        msg.msg(&code, nullptr);
        if (code != ErrMsg::OK)
        {
            msg.setMsg(ErrMsg::OS_ERROR, "Fail to create queue");
            spdlog::error("{}:{} Fail to create queue: {}", __FILE__, __LINE__,
                          name);
            m_conn = nullptr;
            m_queueList.clear();
            return;
        }
    }
}

void SQLiteQueueList::createQueue(const std::string &name, ErrMsg &msg)
{
#ifdef _WIN32
    Proc::WinProc *proc = new (std::nothrow) Proc::WinProc();
#else
    PosixProc *proc = new (std::nothrow) Proc::PosixProc();
#endif
    if (!proc)
    {
        msg.setMsg(ErrMsg::OS_ERROR, "Fail to allocate memory");
        spdlog::error("{}:{} Fail to allocate memory", __FILE__, __LINE__);
        return;
    }

    if (proc->init())
    {
        delete proc;
        msg.setMsg(ErrMsg::OS_ERROR, "Fail to initialize process");
        spdlog::error("{}:{} Fail to initialize process", __FILE__, __LINE__);
        return;
    }

    SQLiteQueue *queue = new (std::nothrow) SQLiteQueue();
    if (!queue)
    {
        delete proc;
        msg.setMsg(ErrMsg::OS_ERROR, "Fail to allocate memory");
        spdlog::error("{}:{} Fail to allocate memory", __FILE__, __LINE__);
        return;
    }

    std::shared_ptr<Proc::IProc> procPtr = std::shared_ptr<Proc::IProc>(proc);
    ErrMsg::ErrCode code;
    queue->init(m_conn, procPtr, name, msg);
    msg.msg(&code, nullptr);
    if (code != ErrMsg::OK)
    {
        delete queue;
        msg.setMsg(ErrMsg::OS_ERROR, "Fail to initialize queue");
        spdlog::error("{}:{} Fail to initialize queue", __FILE__, __LINE__);
        return;
    }

    m_queueList[name] = std::shared_ptr<IQueue>(queue);
}

void SQLiteQueueList::listQueue(std::vector<std::string> &out, ErrMsg &msg)
{
    out.clear();
    out.reserve(m_queueList.size());
    for (auto it = m_queueList.begin();
         it != m_queueList.end();
         ++it)
    {
        out.push_back(it->first);
    }

    if (!out.size())
    {
        msg.setMsg(ErrMsg::NOT_FOUND, "queue list is empty");
        spdlog::error("{}:{} queue list is empty", __FILE__, __LINE__);
        return;
    }
}

void SQLiteQueueList::deleteQueue(const std::string &name, ErrMsg &msg)
{
    if (!m_queueList.erase(name))
    {
        msg.setMsg(ErrMsg::NOT_FOUND, "No such queue");
        spdlog::error("{}:{} No such queue: {}", __FILE__, __LINE__,
            name);
    }
}

void
SQLiteQueueList::renameQueue(const std::string &oldName,
                             const std::string &newName,
                             ErrMsg &msg)
{
    for (auto &it : m_queueList)
    {
        if (it.first == oldName)
        {
            m_queueList[newName] = it.second;
            m_queueList.erase(oldName);
            return;
        }
    }

    msg.setMsg(ErrMsg::NOT_FOUND, "No such queue");
    spdlog::error("{}:{} No such queue: {}", __FILE__, __LINE__,
        oldName);
}

std::shared_ptr<IQueue>
SQLiteQueueList::getQueue(const std::string &name)
{
    return m_queueList[name];
}

} // end namespace DAO

} // end namespace Model
