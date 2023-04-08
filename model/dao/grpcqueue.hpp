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

#ifndef _MODEL_DAO_GRPCQUEUE_HPP_
#define _MODEL_DAO_GRPCQUEUE_HPP_

#include "queue.grpc.pb.h"

#include "model/proc/task.hpp"
#include "grpcconnect.hpp"
#include "iqueue.hpp"

namespace Model
{

namespace DAO
{

class GRPCQueue : public IQueue
{

public:

    GRPCQueue();

    ~GRPCQueue();

    void init(std::shared_ptr<IConnect> &connect,
                      std::shared_ptr<Proc::IProc> &process,
                      const std::string &name,
                      ErrMsg &msg) override;

    void listPending(std::vector<int> &out, ErrMsg &msg) override;

    void listFinished(std::vector<int> &out, ErrMsg &msg) override;

    void pendingDetails(const int_fast32_t id,
                        Proc::Task &out,
                        ErrMsg &msg) override;

    void finishedDetails(const int_fast32_t id,
                         Proc::Task &out,
                         ErrMsg &msg) override;

    void clearPending(ErrMsg &msg) override;

    void clearFinished(ErrMsg &msg) override;

    void currentTask(Proc::Task &out, ErrMsg &msg) override;

    void addTask(Proc::Task &in, ErrMsg &msg) override;

    void removeTask(const int_fast32_t in, ErrMsg &msg) override;

    bool isRunning() const override;

    void readCurrentOutput(std::string &out, ErrMsg &msg) override;

    void start(ErrMsg &msg) override;

    void stop() override;

private:

    std::unique_ptr<stq::Queue::Stub> m_stub;

    std::string m_queueName;

    static void buildTask(stq::TaskDetailsRes &res, Proc::Task &task);

}; // end class GRPCQueue

} // end namespace DAO

} // end namespace Model

#endif // _MODEL_DAO_GRPCQUEUE_HPP_
