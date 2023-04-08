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

class SQLiteQueue: public IQueue
{
public:

    SQLiteQueue();

    ~SQLiteQueue();

    virtual void init(std::shared_ptr<IConnect> &connect,
                      std::shared_ptr<Proc::IProc> &process,
                      const std::string &name,
                      ErrMsg &msg) override;

    virtual void listPending(std::vector<int> &out, ErrMsg &msg) override;

    virtual void listFinished(std::vector<int> &out, ErrMsg &msg) override;

    virtual void pendingDetails(const int_fast32_t id,
                                Proc::Task &out,
                                ErrMsg &msg) override;

    virtual void finishedDetails(const int_fast32_t id,
                                 Proc::Task &out,
                                 ErrMsg &msg) override;

    virtual void clearPending(ErrMsg &msg) override;

    virtual void clearFinished(ErrMsg &msg) override;

    virtual void currentTask(Proc::Task &out, ErrMsg &msg) override;

    virtual void addTask(Proc::Task &in, ErrMsg &msg) override;

    virtual void removeTask(const int_fast32_t in, ErrMsg &msg) override;

    virtual bool isRunning() const override;

    virtual void readCurrentOutput(std::string &out, ErrMsg &msg) override;

    virtual void start(ErrMsg &msg) override;

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

    void clearTable(const std::string &, ErrMsg &);

    void listIDInTable(const std::string &, std::vector<int> &, ErrMsg &);

    void taskDetails(const std::string &,
                     const int_fast32_t,
                     Proc::Task &,
                     ErrMsg &msg);

    void addTaskToTable(const std::string &, const Proc::Task &, ErrMsg &);

    void removeTaskFromPending(const int_fast32_t, const bool, ErrMsg &);

    void splitString(const std::string &, std::vector<std::string> &);

    std::string concatString(const std::vector<std::string> &);

    void getID(int_fast32_t &, ErrMsg &);

    void mainLoop();

    uint_fast8_t mainLoopInit();

    void mainLoopFin();

    void stopImpl();

}; // end class DirToken

} //end namepsace DAO

} // end namespace Model

#endif // _MODEL_DAO_SQLITEQUEUE_HPP_
