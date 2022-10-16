/*
 * Simple Task Queue
 * Copyright (c) 2022 fdar0536
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

#pragma once

#include <condition_variable>
#include <fstream>
#include <queue>
#include <memory>
#include <mutex>
#include <thread>

#include <cinttypes>

#include "stqtask.hpp"

#include "pending.grpc.pb.h"

#include "config.h"

class AbstractProcess;

class STQQueue
{
public:

    STQQueue();

    ~STQQueue();

    static uint8_t init(std::shared_ptr<STQQueue> &, const std::string &);

    void setName(std::string &);

    uint8_t listPanding(::grpc::ServerWriter<::stq::ListTaskRes> *);

    uint8_t listFinished(::grpc::ServerWriter<::stq::ListTaskRes> *);

    uint8_t pendingDetails(uint32_t, STQTask *);

    uint8_t finishedDetails(uint32_t, STQTask *);

    void clearFinished();

    uint8_t currentTask(STQTask *);

    uint8_t addTask(STQTask *);

    uint8_t removeTask(uint32_t);

    uint8_t readCurrentOutput(char *, size_t);

    void start();

    void stop();

private:

    std::string m_name;

    std::vector<STQTask> m_pending;

    std::deque<STQTask> m_finished;

    uint32_t m_id = 0;

    std::mutex m_queueMutex;

    std::thread m_thread;

    std::mutex m_outMutex;

    char m_out[STQ_SERVER_CHILD_STDOUT_BUFFER_SIZE] = {};

    std::fstream m_file;

    std::mutex m_currentTaskMutex;

    STQTask *m_currentTask = nullptr;

    std::condition_variable m_condition;

    AbstractProcess *m_process;

    std::string m_errLog;

    bool m_terminate = false;

    bool m_stopped = true;

    void mainLoop();

    void toFinishedQueue(STQTask *);
};
