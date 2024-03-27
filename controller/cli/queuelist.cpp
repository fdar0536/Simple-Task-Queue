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

#include "spdlog/spdlog.h"

#include "model/dao/grpcconnect.hpp"
#include "model/dao/grpcqueuelist.hpp"
#include "model/utils.hpp"

#include "controller/cli/global.hpp"

#include "queue.hpp"
#include "queuelist.hpp"

namespace Controller
{

namespace CLI
{

QueueList::QueueList() :
    m_queueList(nullptr)
{}

// public member functions
i32 QueueList::init()
{
    Model::DAO::GRPCConnect *conn = new (std::nothrow) Model::DAO::GRPCConnect;
    if (!conn)
    {
        spdlog::error("{}:{} Fail to allocate memory", __FILE__, __LINE__);
        return 1;
    }

    if (conn->init())
    {
        delete conn;
        spdlog::error("{}:{} Fail to initialize conn", __FILE__, __LINE__);
        return 1;
    }

    if (conn->startConnect(Global::config.address, Global::config.port))
    {
        delete conn;
        spdlog::error("{}:{} Fail to connect to server", __FILE__, __LINE__);
        return 1;
    }

    Model::DAO::GRPCQueueList *queueList = new (std::nothrow) Model::DAO::GRPCQueueList;
    if (!queueList)
    {
        delete conn;
        spdlog::error("{}:{} Fail to allocate memory", __FILE__, __LINE__);
        return 1;
    }

    std::shared_ptr<Model::DAO::IConnect> connPtr =
        std::shared_ptr<Model::DAO::IConnect>(conn);
    if (queueList->init(connPtr))
    {
        delete queueList;
        spdlog::error("{}:{} Fail to initialize queue list", __FILE__, __LINE__);
        return 1;
    }

    m_queueList = std::shared_ptr<Model::DAO::IQueueList>
        (reinterpret_cast<Model::DAO::IQueueList *>(queueList));

    m_funcs["delete"] = std::bind(&QueueList::Delete, this);
    m_funcs["create"] = std::bind(&QueueList::create, this);
    m_funcs["rename"] = std::bind(&QueueList::rename, this);

    return 0;
}

i32 QueueList::run()
{
    m_prefix = Global::config.address + ":";
    m_prefix += std::to_string(Global::config.port);
    std::string prefix = m_prefix + "> ";
    i32 ret(0);

    while (Global::keepRunning.load(std::memory_order_relaxed))
    {
        Global::getArgs(prefix);

        // vaild command: create delete list rename help exit
        if (Global::args.size() == 1)
        {
            // help exit "queue name"
            if (Global::args.at(0) == "exit")
            {
                return ret;
            }

            if (Global::args.at(0) == "help")
            {
                Model::Utils::writeConsole("Valid command: create delete list rename help exit\n");
                Model::Utils::writeConsole("Please type \"<command> help\" for more details.\n");
                Model::Utils::writeConsole("Please type \"help\" to show this message.\n");
                Model::Utils::writeConsole("Please type \"exit\" to exit.\n");
                Model::Utils::writeConsole("Or just type queue name which you want to enter\n");
                ret = 0;
                continue;
            }

            if (Global::args.at(0) == "list")
            {
                ret = list();
                continue;
            }

            // cannot interpret
            ret = enter();
            continue;
        }

        try
        {
            ret = m_funcs[Global::args.at(0)]();
        }
        catch(...)
        {
            Model::Utils::writeConsole("Invalid command: " + Global::args[0] + "\n");
            Model::Utils::writeConsole("Please type \"help\" for more info\n");
            ret = 1;
            continue;
        }
    }

    return ret;
}

i32 QueueList::create()
{
    if (Global::args.size() != 2)
    {
        Global::printCMDHelp("create");
        return 1;
    }

    if (Global::args.at(1) == "help")
    {
        Model::Utils::writeConsole("Valid command: \"create [<name>|help]\"\n");
        Model::Utils::writeConsole("<name>: name for queue\n");
        Model::Utils::writeConsole("help: to show this message.\n");
        return 0;
    }

    if (m_queueList->createQueue(Global::args.at(1)))
    {
        Model::Utils::writeConsole("Fail to create queue");
        return 1;
    }

    Model::Utils::writeConsole("done\n");
    return 0;
}

i32 QueueList::Delete()
{
    if (Global::args.size() != 2)
    {
        Global::printCMDHelp("delete");
        return 1;
    }

    if (Global::args.at(1) == "help")
    {
        Model::Utils::writeConsole("Valid command: \"delete [<name>|help]\"\n");
        Model::Utils::writeConsole("<name>: name for queue\n");
        Model::Utils::writeConsole("help: to show this message.\n");
        return 0;
    }

    if (m_queueList->deleteQueue(Global::args.at(1)))
    {
        Model::Utils::writeConsole("Fail to delete queue");
        return 1;
    }

    Model::Utils::writeConsole("done\n");
    return 0;
}

i32 QueueList::list()
{
    if (Global::args.size() > 2)
    {
        Global::printCMDHelp("list");
        return 1;
    }

    if (Global::args.size() == 2)
    {
        if (Global::args.at(1) == "help")
        {
            Model::Utils::writeConsole("Valid command: \"list [help] or list\"\n");
            Model::Utils::writeConsole("help: to show this message.\n");
            return 0;
        }

        Global::printCMDHelp("list");
        return 1;
    }

    std::vector<std::string> out;
    if (m_queueList->listQueue(out))
    {
        Model::Utils::writeConsole("Fail to list queue\n");
        return 1;
    }

    Model::Utils::writeConsole("\n");
    for (size_t i = 0; i < out.size(); ++i)
    {
        Model::Utils::writeConsole(out.at(i) + "\n");
    }

    return 0;
}

i32 QueueList::rename()
{
    if (Global::args.size() > 3)
    {
        Global::printCMDHelp("rename");
        return 1;
    }

    if (Global::args.size() == 2)
    {
        if (Global::args.at(1) == "help")
        {
            Model::Utils::writeConsole("Valid command: \"rename <old name> <new name>\"\n");
            Model::Utils::writeConsole("help: to show this message.\n");
            return 0;
        }

        Global::printCMDHelp("rename");
        return 1;
    }

    if (m_queueList->renameQueue(Global::args.at(1), Global::args.at(2)))
    {
        Model::Utils::writeConsole("Fail to rename queue\n");
        return 1;
    }

    Model::Utils::writeConsole("done\n");
    return 0;
}

i32 QueueList::enter()
{
    auto ptr = m_queueList->getQueue(Global::args.at(0));
    if (ptr == nullptr)
    {
        Model::Utils::writeConsole("Fail to enter the queue\n");
        return 1;
    }

    Queue queue;
    queue.init();
    return queue.run(m_prefix + "/" + Global::args.at(0) + "> ", ptr);
}

} // end namesapce CLI

} // end namespace Controller
