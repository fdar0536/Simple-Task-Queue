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

#ifndef _VIEW_CLI_QUEUE_HPP_
#define _VIEW_CLI_QUEUE_HPP_

#include <functional>
#include <iostream>
#include <unordered_map>

#include "model/dao/iqueue.hpp"
#include "utils.hpp"

namespace View
{

namespace CLI
{

namespace Queue
{

template<class T>
std::unordered_map<std::string,
    std::function<int(std::shared_ptr<Model::DAO::IQueue<T>> &)>> handlers;

template<class T>
void init();

template<class T>
int_fast32_t help(std::shared_ptr<Model::DAO::IQueue<T>> &);

template<class T>
int_fast32_t listPending(std::shared_ptr<Model::DAO::IQueue<T>> &);

template<class T>
int_fast32_t listFinished(std::shared_ptr<Model::DAO::IQueue<T>> &);

template<class T>
int_fast32_t pendingDetails(std::shared_ptr<Model::DAO::IQueue<T>> &);

template<class T>
int_fast32_t finishedDetails(std::shared_ptr<Model::DAO::IQueue<T>> &);

template<class T>
int_fast32_t clearPending(std::shared_ptr<Model::DAO::IQueue<T>> &);

template<class T>
int_fast32_t clearFinished(std::shared_ptr<Model::DAO::IQueue<T>> &);

template<class T>
int_fast32_t currentTask(std::shared_ptr<Model::DAO::IQueue<T>> &);

template<class T>
int_fast32_t addTask(std::shared_ptr<Model::DAO::IQueue<T>> &);

template<class T>
int_fast32_t removeTask(std::shared_ptr<Model::DAO::IQueue<T>> &);

template<class T>
int_fast32_t isRunning(std::shared_ptr<Model::DAO::IQueue<T>> &);

template<class T>
int_fast32_t readCurrentOutput(std::shared_ptr<Model::DAO::IQueue<T>> &);

template<class T>
int_fast32_t start(std::shared_ptr<Model::DAO::IQueue<T>> &);

template<class T>
int_fast32_t stop(std::shared_ptr<Model::DAO::IQueue<T>> &);

template<class T>
int_fast32_t run(std::shared_ptr<Model::DAO::IQueue<T>> &queue, const std::string &queueName)
{
    std::function<int(std::shared_ptr<Model::DAO::IQueue<T>> &)> func(nullptr);
    std::string cmd = "";
    std::string prefix = "STQ/queuelist/" + queueName + ">";
    int_fast32_t lastExitCode(0);
    while (1)
    {
        cmd = Utils::getInput(prefix);
        if (cmd == "exit")
        {
            return lastExitCode;
        }
        else if (cmd.empty())
        {
            static_cast<void>(help<T>(queue));
            lastExitCode = 1;
            continue;
        }

        func = handlers<T>[cmd];
        if (func == nullptr)
        {
            Utils::writeConsole("Unknown command: " + cmd + "\n");
            lastExitCode = 1;
            continue;
        }

        lastExitCode = func(queue);
    }
}

template<class T>
void init()
{
    handlers<T>["help"] = help<T>;

    handlers<T>["listPending"] = listPending<T>;
    handlers<T>["listFinished"] = listFinished<T>;
    handlers<T>["pendingDetails"] = pendingDetails<T>;
    handlers<T>["finishedDetails"] = finishedDetails<T>;
    handlers<T>["clearPending"] = clearPending<T>;
    handlers<T>["clearFinished"] = clearFinished<T>;
    handlers<T>["currentTask"] = currentTask<T>;
    handlers<T>["addTask"] = addTask<T>;
    handlers<T>["removeTask"] = removeTask<T>;
    handlers<T>["isRunning"] = isRunning<T>;
    handlers<T>["readCurrentOutput"] = readCurrentOutput<T>;
    handlers<T>["start"] = start<T>;
    handlers<T>["stop"] = stop<T>;
}

template<class T>
int_fast32_t help(std::shared_ptr<Model::DAO::IQueue<T>> &)
{
    Utils::writeConsole("listPending: list pending task\n");
    Utils::writeConsole("listFinished: list finished task\n");
    Utils::writeConsole("pendingDetails: task details in pending list\n");
    Utils::writeConsole("finishedDetails: task details in finished list\n");
    Utils::writeConsole("clearPending: clear all tasks in pending list\n");
    Utils::writeConsole("clearFinished: clear all tasks in finished list\n");
    Utils::writeConsole("currentTask: get the current task's details\n");
    Utils::writeConsole("addTask: add task to queue\n");
    Utils::writeConsole("removeTask: remove task from pending list\n");
    Utils::writeConsole("isRunning: output this queue is running or not\n");
    Utils::writeConsole("readCurrentOutput: read the current output of this queue\n");
    Utils::writeConsole("start: start this queue\n");
    Utils::writeConsole("stop: stop this queue\n");

    Utils::writeConsole("help: print this message\n");
    Utils::writeConsole("exit: exit this loop\n");
    return 0;
}

template<class T>
int_fast32_t listPending(std::shared_ptr<Model::DAO::IQueue<T>> &queue)
{
    std::vector<int> out;
    if (queue->listPending(out))
    {
        Utils::writeConsole("Fail to list pending list\n");
        return 1;
    }

    if (!out.size())
    {
        Utils::writeConsole("pending list is empty\n");
        return 0;
    }

    for (auto &it : out)
    {
        Utils::writeConsole(std::to_string(it) + "\n");
    }

    return 0;
}

template<class T>
int_fast32_t listFinished(std::shared_ptr<Model::DAO::IQueue<T>> &queue)
{
    std::vector<int> out;
    if (queue->listFinished(out))
    {
        Utils::writeConsole("Fail to list finished list\n");
        return 1;
    }

    if (!out.size())
    {
        Utils::writeConsole("finished list is empty\n");
        return 0;
    }

    for (auto &it : out)
    {
        Utils::writeConsole(std::to_string(it) + "\n");
    }

    return 0;
}

template<class T>
int_fast32_t pendingDetails(std::shared_ptr<Model::DAO::IQueue<T>> &queue)
{
    Model::Task out = Model::Task();
    std::string input = Utils::getInput("Please enter id:");

    int_fast32_t id(0);
    try
    {
        id = std::stoi(input);
    }
    catch (...)
    {
        Utils::writeConsole("Invalid input: " + input + "\n");
        return 1;
    }

    if (queue->pendingDetails(id, out))
    {
        Utils::writeConsole("Fail to get details\n");
        return 1;
    }

    out.print();
    return 0;
}

template<class T>
int_fast32_t finishedDetails(std::shared_ptr<Model::DAO::IQueue<T>> &queue)
{
    Model::Task out = Model::Task();
    std::string input = Utils::getInput("Please enter id:");

    int_fast32_t id(0);
    try
    {
        id = std::stoi(input);
    }
    catch (...)
    {
        Utils::writeConsole("Invalid input: " + input + "\n");
        return 1;
    }

    if (queue->finishedDetails(id, out))
    {
        Utils::writeConsole("Fail to get details\n");
        return 1;
    }

    out.print();
    return 0;
}

template<class T>
int_fast32_t clearPending(std::shared_ptr<Model::DAO::IQueue<T>> &queue)
{
    if (queue->clearPending())
    {
        Utils::writeConsole("Fail to clear pending list\n");
        return 1;
    }

    return 0;
}

template<class T>
int_fast32_t clearFinished(std::shared_ptr<Model::DAO::IQueue<T>> &queue)
{
    if (queue->clearFinished())
    {
        Utils::writeConsole("Fail to clear finished list\n");
        return 1;
    }

    return 0;
}

template<class T>
int_fast32_t currentTask(std::shared_ptr<Model::DAO::IQueue<T>> &queue)
{
    Model::Task out = Model::Task();
    if (queue->currentTask(out))
    {
        Utils::writeConsole("Fail to get current task\n");
        return 1;
    }

    out.print();
    return 0;
}

template<class T>
int_fast32_t addTask(std::shared_ptr<Model::DAO::IQueue<T>> &queue)
{
    Model::Task in = Model::Task();
    in.execName = Utils::getInput("execName:");
    std::string s = Utils::getInput("args:");

    std::string delimiter = " ";
    in.args.clear();
    in.args.reserve(128);

    size_t pos = 0;
    std::string token;
    while (( pos = s.find(delimiter) ) != std::string::npos)
    {
        in.args.push_back(s.substr(0, pos));
        s.erase(0, pos + delimiter.length());
    }

    in.workDir = Utils::getInput("workDir:");
    in.postHanlder = Utils::getInput("postHanlder:");
    
    Utils::writeConsole("Your input:\n");
    Utils::writeConsole("\n");
    in.print();
    Utils::writeConsole("\n");
    static_cast<void>(Utils::getInput("Press return to continue"));
    Utils::writeConsole("\n");

    if (queue->addTask(in))
    {
        Utils::writeConsole("Fail to add task\n");
        return 1;
    }

    return 0;
}

template<class T>
int_fast32_t removeTask(std::shared_ptr<Model::DAO::IQueue<T>> &queue)
{
    std::string input = Utils::getInput("Please enter id:");

    int_fast32_t id(0);
    try
    {
        id = std::stoi(input);
    }
    catch (...)
    {
        Utils::writeConsole("Invalid input: " + input + "\n");
        return 1;
    }

    if (queue->removeTask(id))
    {
        Utils::writeConsole("Fail to remove task\n");
        return 1;
    }

    Utils::writeConsole("done\n");
    return 0;
}

template<class T>
int_fast32_t isRunning(std::shared_ptr<Model::DAO::IQueue<T>> &queue)
{
    Utils::writeConsole(std::to_string(queue->isRunning()) + "\n");
    return 0;
}

template<class T>
int_fast32_t readCurrentOutput(std::shared_ptr<Model::DAO::IQueue<T>> &queue)
{
    std::string out;
    if (queue->readCurrentOutput(out))
    {
        Utils::writeConsole("Fail to read current output\n");
        return 1;
    }

    Utils::writeConsole(out);
    return 0;
}

template<class T>
int_fast32_t start(std::shared_ptr<Model::DAO::IQueue<T>> &queue)
{
    if (queue->start())
    {
        Utils::writeConsole("Fail to start this queue\n");
        return 1;
    }

    Utils::writeConsole("done\n");
    return 0;
}

template<class T>
int_fast32_t stop(std::shared_ptr<Model::DAO::IQueue<T>> &queue)
{
    queue->stop();
    Utils::writeConsole("done\n");
    return 0;
}

} // end namespace Queue

} // end namespace CLI

} // end namespace View

#endif // end _VIEW_CLI_QUEUE_HPP_
