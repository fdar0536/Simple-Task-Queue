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

#ifndef _VIEW_CLI_QUEUELIST_HPP_
#define _VIEW_CLI_QUEUELIST_HPP_

#include <functional>
#include <iostream>
#include <unordered_map>

#include "model/dao/iqueuelist.hpp"
#include "model/utils.hpp"
#include "queue.hpp"
#include "utils.hpp"

namespace View
{

namespace CLI
{

namespace QueueList
{

template<class T>
std::unordered_map<std::string,
    std::function<int(Model::DAO::IQueueList<T> &)>> handlers;

template<class T>
void init();

uint_fast8_t exit();

template<class T>
int_fast32_t help(Model::DAO::IQueueList<T> &);

template<class T>
int_fast32_t create(Model::DAO::IQueueList<T> &queueList);

template<class T>
int_fast32_t list(Model::DAO::IQueueList<T> &queueList);

template<class T>
int_fast32_t deleteQueue(Model::DAO::IQueueList<T> &queueList);

template<class T>
int_fast32_t rename(Model::DAO::IQueueList<T> &queueList);

template<class T>
int_fast32_t run(Model::DAO::IQueueList<T> &queueList)
{
    std::string cmd = "";
    int_fast32_t lastExitCode(0);
    std::function<int(Model::DAO::IQueueList<T> &)> func(nullptr);
    while (1)
    {
        cmd = Utils::getInput("STQ/queueList>");
        if (cmd == "exit")
        {
            if (exit())
            {
                return lastExitCode;
            }
        }
        else if (cmd.empty())
        {
            static_cast<void>(help<T>(queueList));
            lastExitCode = 1;
            continue;
        }

        func = handlers<T>[cmd];
        if (func != nullptr)
        {
            lastExitCode = func(queueList);
            continue;
        }

        // cannot interpret
        auto queue = queueList.getQueue(cmd);
        if (queue == nullptr)
        {
            Model::Utils::writeConsole("No such queue: " + cmd + "\n");
            lastExitCode = 1;
            continue;
        }

        lastExitCode = Queue::run<T>(queue, cmd);
        queue = nullptr;
    }
}

template<class T>
void init()
{
    handlers<T>["help"] = help<T>;
    handlers<T>["create"] = create<T>;
    handlers<T>["list"] = list<T>;
    handlers<T>["delete"] = deleteQueue<T>;
    handlers<T>["rename"] = rename<T>;
}

uint_fast8_t exit()
{
    std::string input;
    while (1)
    {
        Model::Utils::writeConsole("Are you sure to exit?\n");
        Model::Utils::writeConsole("Current queue list will be clean up.\n");
        input = Utils::getInput("Your answer (yes/no):");
        if (input == "yes")
        {
            return 1;
        }
        else if (input == "no")
        {
            return 0;
        }

        Model::Utils::writeConsole("Please enter \"yes\" or \"no\"\n");
    }
}

template<class T>
int_fast32_t help(Model::DAO::IQueueList<T> &)
{
    Model::Utils::writeConsole("create: create the new queue\n");
    Model::Utils::writeConsole("list: list all queue(s)\n");
    Model::Utils::writeConsole("delete: delete the queue\n");
    Model::Utils::writeConsole("rename: rename the queue\n");
    Model::Utils::writeConsole("help: print this message\n");
    Model::Utils::writeConsole("exit: exit this loop\n");

    Model::Utils::writeConsole("cannot interpret: enter the queue you typed.\n");

    return 0;
}

template<class T>
int_fast32_t create(Model::DAO::IQueueList<T> &queueList)
{
    std::string cmd = Utils::getInput("Enter the name:");
    if (queueList.createQueue(cmd))
    {
        Model::Utils::writeConsole("Fail to create queue.\n");
        return 1;
    }

    return 0;
}

template<class T>
int_fast32_t list(Model::DAO::IQueueList<T> &queueList)
{
    std::vector<std::string> out;
    if (queueList.listQueue(out))
    {
        Model::Utils::writeConsole("Fail to list queue or queue is empty.\n");
        return 1;
    }

    for (auto &i : out)
    {
        Model::Utils::writeConsole(i + "\n");
    }

    return 0;
}

template<class T>
int_fast32_t deleteQueue(Model::DAO::IQueueList<T> &queueList)
{
    std::string cmd = Utils::getInput("Enter the name:");
    if (queueList.deleteQueue(cmd))
    {
        Model::Utils::writeConsole("Fail to delete queue or no such queue\n");
        return 1;
    }

    return 0;
}

template<class T>
int_fast32_t rename(Model::DAO::IQueueList<T> &queueList)
{
    std::string oldName = Utils::getInput("Enter the queue name:");
    std::string newName = Utils::getInput("Enter the new name:");
    if (queueList.renameQueue(oldName, newName))
    {
        Model::Utils::writeConsole("Fail to rename queue or no such queue\n");
        return 1;
    }

    return 0;
}

} // end namespace QueueList

} // end namespace CLI

} // end namespace View

#endif // _VIEW_CLI_QUEUELIST_HPP_
