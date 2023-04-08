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

#ifndef _MODEL_DAO_IQUEUE_HPP_
#define _MODEL_DAO_IQUEUE_HPP_

#include <vector>

#include "model/proc/iproc.hpp"
#include "model/proc/task.hpp"
#include "model/errmsg.hpp"
#include "iconnect.hpp"

namespace Model
{

namespace DAO
{

class IQueue
{
public:

    virtual ~IQueue() {}

    virtual void init(std::shared_ptr<IConnect> &connect,
                      std::shared_ptr<Proc::IProc> &process,
                      const std::string &name,
                      ErrMsg &msg) = 0;

    virtual void listPending(std::vector<int> &out, ErrMsg &msg) = 0;

    virtual void listFinished(std::vector<int> &out, ErrMsg &msg) = 0;

    virtual void pendingDetails(const int_fast32_t id,
                                Proc::Task &out,
                                ErrMsg &msg) = 0;

    virtual void finishedDetails(const int_fast32_t id,
                                 Proc::Task &out,
                                 ErrMsg &msg) = 0;

    virtual void clearPending(ErrMsg &msg) = 0;

    virtual void clearFinished(ErrMsg &msg) = 0;

    virtual void currentTask(Proc::Task &out, ErrMsg &msg) = 0;

    virtual void addTask(Proc::Task &in, ErrMsg &msg) = 0;

    virtual void removeTask(const int_fast32_t in, ErrMsg &msg) = 0;

    virtual bool isRunning() const = 0;

    virtual void readCurrentOutput(std::string &out, ErrMsg &msg) = 0;

    virtual void start(ErrMsg &msg) = 0;

    virtual void stop() = 0;

protected:

    std::shared_ptr<Model::Proc::IProc> m_process;

}; // end class IQueue

} // end namespace DAO

} // end namespace Model

#endif // _MODEL_DAO_IQUEUE_HPP_
