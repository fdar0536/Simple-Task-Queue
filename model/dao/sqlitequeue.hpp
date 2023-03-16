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

#ifndef _MODEL_DAO_SQLITEQUEUE_HPP_
#define _MODEL_DAO_SQLITEQUEUE_HPP_

#include <atomic>
#include <mutex>
#include <thread>

#include "sqliteconnect.hpp"
#include "iqueue.hpp"

namespace Model
{

namespace DAO
{

class SQLiteQueue: public IQueue<SQLiteToken>
{
public:

    SQLiteQueue();

    ~SQLiteQueue();

    virtual uint_fast8_t init(std::shared_ptr<IConnect<SQLiteToken>> &connect,
                         std::shared_ptr<Proc::IProc> &process,
                         const std::string &name) override;

    virtual uint_fast8_t listPending(std::vector<int> &out) override;

    virtual uint_fast8_t listFinished(std::vector<int> &out) override;

    virtual uint_fast8_t pendingDetails(const int_fast32_t id, Proc::Task &out) override;

    virtual uint_fast8_t finishedDetails(const int_fast32_t id, Proc::Task &out) override;

    virtual uint_fast8_t clearPending() override;

    virtual uint_fast8_t clearFinished() override;

    virtual uint_fast8_t currentTask(Proc::Task &out) override;

    virtual uint_fast8_t addTask(Proc::Task &in) override;

    virtual uint_fast8_t removeTask(const int_fast32_t in) override;

    virtual bool isRunning() const override;

    virtual uint_fast8_t readCurrentOutput(std::string &out) override;

    virtual uint_fast8_t start() override;

    virtual void stop() override;

private:

    std::shared_ptr<SQLiteToken> m_token;

    std::mutex m_currentTaskMutex;

    Proc::Task m_currentTask;

    std::atomic<bool> m_isRunning;

    std::atomic<bool> m_start;

    std::jthread m_thread;

    uint_fast8_t connectToDB(const std::string &);

    uint_fast8_t createTable(const std::string &);

    uint_fast8_t verifyTable(const std::string &);

    uint_fast8_t verifyID();

    uint_fast8_t clearTable(const std::string &);

    uint_fast8_t listIDInTable(const std::string &, std::vector<int> &);

    uint_fast8_t taskDetails(const std::string &, const int_fast32_t, Proc::Task &);

    uint_fast8_t addTaskToTable(const std::string &, const Proc::Task &);

    uint_fast8_t removeTaskFromPending(const int_fast32_t, const bool);

    void splitString(const std::string &, std::vector<std::string> &);

    std::string concatString(const std::vector<std::string> &);

    uint_fast8_t getID(int_fast32_t &);

    void mainLoop();

    uint_fast8_t mainLoopInit();

    void mainLoopFin();

    void stopImpl();

}; // end class DirToken

} //end namepsace DAO

} // end namespace Model

#endif // _MODEL_DAO_SQLITEQUEUE_HPP_
