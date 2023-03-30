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

#include <new>
#include <iostream>

#include "model/dao/sqlitequeue.hpp"
#include "model/dao/sqlitequeuelist.hpp"
#include "model/utils.hpp"

#include "queuelist.hpp"
#include "utils.hpp"
#include "main.hpp"

namespace View
{

namespace CLI
{

int_fast32_t help();

int_fast32_t main()
{
    std::string cmd = "";
    int_fast32_t lastExitCode(0);

    // init
    QueueList::init<Model::DAO::SQLiteToken>();
    Queue::init<Model::DAO::SQLiteToken>();

    while (1)
    {
        cmd = Utils::getInput("STQ>");
        if (cmd == "exit")
        {
            return lastExitCode;
        }
        else if (cmd.empty())
        {
            static_cast<void>(help());
            lastExitCode = 1;
            continue;
        }

        auto connPtr = new (std::nothrow) Model::DAO::SQLiteConnect();
        if (!connPtr)
        {
            Model::Utils::writeConsole("Fail to allocate memory\n");
            lastExitCode = 1;
            continue;
        }

        if (connPtr->init())
        {
            Model::Utils::writeConsole("Fail to initialize conn\n");
            lastExitCode = 1;
            continue;
        }

        lastExitCode = connPtr->startConnect(cmd);
        if (lastExitCode)
        {
            delete connPtr;
            Model::Utils::writeConsole("Fail to connect to database\n");
            continue;
        }

        auto conn = std::shared_ptr<Model::DAO::IConnect<Model::DAO::SQLiteToken>>(connPtr);
        auto queueList = Model::DAO::SQLiteQueueList();
        lastExitCode = queueList.init(conn);
        if (lastExitCode)
        {
            Model::Utils::writeConsole("Fail to initialize queue list\n");
            continue;
        }

        lastExitCode = QueueList::run<Model::DAO::SQLiteToken>(queueList);
    }
}

int_fast32_t help()
{
    Model::Utils::writeConsole("Please enter the path of database\n");
    Model::Utils::writeConsole("or type \"exit\" to exit the program\n");
    return 0;
}

} // end namespace CLI

} // end namespace View
