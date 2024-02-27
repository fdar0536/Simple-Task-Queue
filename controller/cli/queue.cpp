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

#ifdef _WIN32
#include "winsock.h"
#define sleep(x) Sleep(x * 1000)
#else
#include "unistd.h"
#include "time.h"
#endif

#include "spdlog/spdlog.h"

#include "model/utils.hpp"

#include "controller/cli/global.hpp"

#include "queue.hpp"

namespace Controller
{

namespace CLI
{

void Queue::init()
{
    m_funcs["list"] = std::bind(&Queue::list, this);
    m_funcs["details"] = std::bind(&Queue::details, this);
    m_funcs["clear"] = std::bind(&Queue::clear, this);
    m_funcs["remove"] = std::bind(&Queue::remove, this);
    m_funcs["current"] = std::bind(&Queue::current, this);
    m_funcs["add"] = std::bind(&Queue::add, this);
    m_funcs["isRunning"] = std::bind(&Queue::isRunning, this);
    m_funcs["start"] = std::bind(&Queue::start, this);
    m_funcs["stop"] = std::bind(&Queue::stop, this);
    m_funcs["output"] = std::bind(&Queue::output, this);
}

i32 Queue::run(const std::string &prefix,
               std::shared_ptr<Model::DAO::IQueue> &queuePtr)
{
    if (queuePtr == nullptr)
    {
        spdlog::error("{}:{} you should never see this line");
        Model::Utils::writeConsole("queuePtr is nullptr\n");
        return 1;
    }

    m_queue = queuePtr;

    i32 ret(0);
    while (Global::keepRunning.load(std::memory_order_relaxed))
    {
        Global::getArgs(prefix);

        if (Global::args.size() == 1)
        {
            // vaild commands: help exit
            if (Global::args.at(0) == "help")
            {
                Model::Utils::writeConsole("Vaild commands: list details clear ");
                Model::Utils::writeConsole("remove current add isRunning ");
                Model::Utils::writeConsole("start stop output help exit\n");
                Model::Utils::writeConsole("Please type \"<command> help\" for more details.\n");
                Model::Utils::writeConsole("Please type \"help\" to show this message.\n");
                Model::Utils::writeConsole("Please type \"exit\" to exit.\n");

                ret = 0;
                continue;
            }

            if (Global::args.at(0) == "exit")
            {
                break;
            }
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

i32 Queue::list()
{
    if (Global::args.size() != 2)
    {
        Global::printCMDHelp("list");
        return 1;
    }

    if (Global::args.at(1) == "help")
    {
        Model::Utils::writeConsole("Valid command: \"list [finished|pending|help]\"\n");
        Model::Utils::writeConsole("finished: list finished task\n");
        Model::Utils::writeConsole("pending: list pending task\n");
        Model::Utils::writeConsole("help: to show this message.\n");
        return 0;
    }

    u8 ret(0);
    std::vector<int> out;
    if (Global::args.at(1) == "pending")
    {
        ret = m_queue->listPending(out);
    }

    if (Global::args.at(1) == "finished")
    {
        ret = m_queue->listFinished(out);
    }

    if (ret)
    {
        Model::Utils::writeConsole("Fail to list tasks\n");
        return 1;
    }

    if (out.empty())
    {
        Model::Utils::writeConsole("list is empty\n");
        return 0;
    }

    for (auto i = out.begin(); i != out.end(); ++i)
    {
        Model::Utils::writeConsole(std::to_string(*i) + "\n");
    }

    return 0;
}

i32 Queue::details()
{
    if (Global::args.size() == 2)
    {
        if (Global::args.at(1) == "help")
        {
            Model::Utils::writeConsole("Valid command: \"details [finished|pending] <id>\"\n");
            Model::Utils::writeConsole("finished: finished task details\n");
            Model::Utils::writeConsole("pending: pending task details\n");
            Model::Utils::writeConsole("id is task id\n");
            Model::Utils::writeConsole("help: to show this message.\n");
            return 0;
        }

        Global::printCMDHelp("details");
        return 1;
    }

    if (Global::args.size() > 3 || Global::args.size() < 2)
    {
        Global::printCMDHelp("details");
        return 1;
    }

    u8 ret(0);
    i32 id(0);
    try
    {
        id = std::stoi(Global::args.at(2));
    }
    catch(...)
    {
        Global::printCMDHelp("details");
        return 1;
    }

    Model::Proc::Task out;
    if (Global::args.at(1) == "pending")
    {
        ret = m_queue->pendingDetails(id, out);
    }

    if (Global::args.at(1) == "finished")
    {
        ret = m_queue->finishedDetails(id, out);
    }

    if (ret)
    {
        Model::Utils::writeConsole("Fail to get task details\n");
        return 1;
    }

    out.print();
    return 0;
}

i32 Queue::clear()
{
    if (Global::args.size() != 2)
    {
        Global::printCMDHelp("clear");
        return 1;
    }

    if (Global::args.at(1) == "help")
    {
        Model::Utils::writeConsole("Valid command: \"clear [finished|pending|help]\"\n");
        Model::Utils::writeConsole("finished: clear finished task\n");
        Model::Utils::writeConsole("pending: clear pending task\n");
        Model::Utils::writeConsole("help: to show this message.\n");
        return 0;
    }

    u8 ret(0);
    if (Global::args.at(1) == "pending")
    {
        ret = m_queue->clearPending();
    }

    if (Global::args.at(1) == "finished")
    {
        ret = m_queue->clearFinished();
    }

    if (ret)
    {
        Model::Utils::writeConsole("Fail to clear tasks\n");
        return 1;
    }

    Model::Utils::writeConsole("done\n");
    return 0;
}

i32 Queue::current()
{
    if (Global::args.size() == 2)
    {
        if (Global::args.at(1) == "help")
        {
            Model::Utils::writeConsole("Valid command: \"current\"\n");
            Model::Utils::writeConsole("help: to show this message.\n");
            return 0;
        }

        Global::printCMDHelp("current");
        return 1;
    }

    if (Global::args.size() > 3)
    {
        Global::printCMDHelp("current");
        return 1;
    }

    Model::Proc::Task out;
    if (m_queue->currentTask(out))
    {
        Model::Utils::writeConsole("Fail to get current task\n");
        return 1;
    }

    out.print();
    return 0;
}

i32 Queue::add()
{
    if (Global::args.size() == 2)
    {
        if (Global::args.at(1) == "help")
        {
            Model::Utils::writeConsole("Valid command: \"add <work dir> <exec name> <args...>\"\n");
            Model::Utils::writeConsole("help: to show this message.\n");
            return 0;
        }

        Global::printCMDHelp("add");
        return 1;
    }

    if (Global::args.size() < 2)
    {
        Global::printCMDHelp("add");
        return 1;
    }

    Model::Proc::Task in;
    in.workDir = Global::args.at(1);
    in.execName = Global::args.at(2);
    if (Global::args.size() > 3)
    {
        in.args.reserve(Global::args.size() - 3);
        for (size_t i = 3; i < Global::args.size(); ++i)
        {
            in.args.push_back(Global::args.at(i));
        }
    }

    if (m_queue->addTask(in))
    {
        Model::Utils::writeConsole("Fail to add task\n");
        return 1;
    }

    Model::Utils::writeConsole("done\n");
    return 0;
}

i32 Queue::remove()
{
    if (Global::args.size() < 2)
    {
        Global::printCMDHelp("remove");
        return 1;
    }

    if (Global::args.at(1) == "help")
    {
        Model::Utils::writeConsole("Valid command: \"remove <task id ...>\"\n");
        Model::Utils::writeConsole("remove the task from pending list.\n");
        Model::Utils::writeConsole("help: to show this message.\n");
        return 0;
    }

    i32 id(0);
    for (size_t i = 1; i < Global::args.size(); ++i)
    {
        try
        {
            id = std::stoi(Global::args.at(i));
        }
        catch(...)
        {
            Model::Utils::writeConsole("invaild id: " + Global::args.at(i));
            Model::Utils::writeConsole(", ignore");
            continue;
        }

        if (m_queue->removeTask(id))
        {
            Model::Utils::writeConsole("Fail to remove task: " + Global::args.at(i));
            Model::Utils::writeConsole("\n");
        }
    }

    Model::Utils::writeConsole("done\n");
    return 0;
}

i32 Queue::isRunning()
{
    if (Global::args.size() == 2)
    {
        if (Global::args.at(1) == "help")
        {
            Model::Utils::writeConsole("Valid command: \"isRunning\"\n");
            Model::Utils::writeConsole("help: to show this message.\n");
            return 0;
        }

        Global::printCMDHelp("isRunning");
        return 1;
    }

    if (Global::args.size() > 2)
    {
        Global::printCMDHelp("isRunning");
        return 1;
    }

    if (m_queue->isRunning())
    {
        Model::Utils::writeConsole("ture");
    }
    else
    {
        Model::Utils::writeConsole("false");
    }

    return 0;
}

i32 Queue::start()
{
    if (Global::args.size() == 2)
    {
        if (Global::args.at(1) == "help")
        {
            Model::Utils::writeConsole("Valid command: \"start\"\n");
            Model::Utils::writeConsole("help: to show this message.\n");
            return 0;
        }

        Global::printCMDHelp("start");
        return 1;
    }

    if (Global::args.size() > 2)
    {
        Global::printCMDHelp("start");
        return 1;
    }

    if (m_queue->start())
    {
        Model::Utils::writeConsole("Fail to start the queue");
        return 1;
    }

    Model::Utils::writeConsole("done\n");
    return 0;
}

i32 Queue::stop()
{
    if (Global::args.size() == 2)
    {
        if (Global::args.at(1) == "help")
        {
            Model::Utils::writeConsole("Valid command: \"stop\"\n");
            Model::Utils::writeConsole("help: to show this message.\n");
            return 0;
        }

        Global::printCMDHelp("stop");
        return 1;
    }

    if (Global::args.size() > 2)
    {
        Global::printCMDHelp("stop");
        return 1;
    }

    m_queue->stop();
    Model::Utils::writeConsole("done\n");
    return 0;
}

static int kbhit()
{
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 3;
    tv.tv_usec = 0;

    FD_ZERO(&fds);
#ifdef _WIN32
    FD_SET(_fileno(stdin), &fds);
    select(_fileno(stdin) + 1, &fds, NULL, NULL, &tv);
    return FD_ISSET(_fileno(stdin), &fds);
#else
    FD_SET(STDIN_FILENO, &fds);
    select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
#endif
}

i32 Queue::output()
{
    if (Global::args.size() == 2)
    {
        if (Global::args.at(1) == "help")
        {
            Model::Utils::writeConsole("Valid command: \"output\"\n");
            Model::Utils::writeConsole("help: to show this message.\n");
            return 0;
        }

        Global::printCMDHelp("output");
        return 1;
    }

    if (Global::args.size() > 2)
    {
        Global::printCMDHelp("output");
        return 1;
    }

    Model::Utils::writeConsole("press any key to stop\n");
    std::string out;
    while (Global::keepRunning.load(std::memory_order_relaxed))
    {
        if (m_queue->readCurrentOutput(out))
        {
            Model::Utils::writeConsole("Fail to read output\n");
        }
        else
        {
            Model::Utils::writeConsole(out);
        }

        if (kbhit())
        {
            break;
        }
    }

    return 0;
}

} // end namesapce CLI

} // end namespace Controller
