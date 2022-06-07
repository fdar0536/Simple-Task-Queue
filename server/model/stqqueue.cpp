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

#include <new>

#include <cstring>

#include "../global.hpp"
#include "stqqueue.hpp"

STQQueue::STQQueue() :
    m_id(0),
    m_process(nullptr),
    m_terminate(false),
    m_stopped(true)
{
}

STQQueue::~STQQueue()
{
    if (!m_stopped)
    {
        stop();
    }

    if (m_process) delete m_process;
}

uint8_t STQQueue::init(std::shared_ptr<STQQueue> &in, const std::string &name)
{
#ifdef _WIN32
    in->m_process = new (std::nothrow) WinProcess();
#else
    in->m_process = new (std::nothrow) NixProcess();
#endif

    if (!in->m_process)
    {
        in->m_errLog.clear();
        in->m_errLog += __FILE__;
        in->m_errLog += ":";
        in->m_errLog += __LINE__;
        in->m_errLog += " Fail to allocate memory.";
        Global::logger.write(Logger::Error, in->m_errLog.c_str());
        return 1;
    }

    if (in->m_process->init(in->m_process))
    {
        in->m_errLog.clear();
        in->m_errLog += __FILE__;
        in->m_errLog += ":";
        in->m_errLog += __LINE__;
        in->m_errLog += " Fail to initialize process object.";
        Global::logger.write(Logger::Error, in->m_errLog.c_str());

        delete in->m_process;
        in->m_process = nullptr;
        return 1;
    }

    in->m_name = name;
    return 0;
}

void STQQueue::setName(std::string &name)
{
    m_name = name;
}

uint8_t STQQueue::listPanding(::stq::ListTaskRes *out)
{
    return listID(m_pending, out);
}

uint8_t STQQueue::listFinished(::stq::ListTaskRes *out)
{
    return listID(m_finished, out);
}

uint8_t STQQueue::pendingDetails(uint32_t id, STQTask *out)
{
    return taskDetails(m_pending, id, out);
}

uint8_t STQQueue::finishedDetails(uint32_t id, STQTask *out)
{
    return taskDetails(m_finished, id, out);
}

void STQQueue::clearFinished()
{
    std::unique_lock<std::mutex> lock(m_queueMutex);
    m_finished.clear();
}

uint8_t STQQueue::currentTask(STQTask *out)
{
    if (!out) return 1;

    std::unique_lock<std::mutex> lock(m_currentTaskMutex);
    if (!m_currentTask) return 1;

    (*out) = *m_currentTask;
    return 0;
}

uint8_t STQQueue::addTask(STQTask *in)
{
    if (!in) return 1;

    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        (*in).id = m_id;
        ++m_id;
        m_pending.push_back(*in);
    }

    m_condition.notify_one();
    return 0;
}

uint8_t STQQueue::removeTask(uint32_t id)
{
    uint8_t ret(1);
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        for (auto it = m_pending.begin(); it != m_pending.end(); ++it)
        {
            if (it->id == id)
            {
                m_pending.erase(it);
                ret = 0;
                break;
            }
        }
    }

    m_condition.notify_one();
    return ret;
}

void STQQueue::start()
{
    if (!m_stopped) return;

    m_stopped = false;
    m_thread = std::thread([this]()
    {
        mainLoop();
    });
}

void STQQueue::stop()
{
    if (m_stopped) return;

    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        m_terminate = true;
    }

    m_condition.notify_all();
    m_process->stop();
    m_thread.join();
    m_stopped = true;
}

// private member functions
uint8_t STQQueue::listID(std::deque<STQTask> &queue, ::stq::ListTaskRes *out)
{
    if (!out) return 1;
    std::unique_lock<std::mutex> lock(m_queueMutex);

    if (queue.empty())
    {
        return 0;
    }

    for (auto it = queue.begin(); it != queue.end(); ++it)
    {
        out->add_list(it->id);
    }

    return 0;
}

uint8_t STQQueue::taskDetails(std::deque<STQTask> &queue, uint32_t id, STQTask *out)
{
    if (!out) return 1;
    std::unique_lock<std::mutex> lock(m_queueMutex);

    STQTask *task(nullptr);
    for (auto it = queue.begin(); it != queue.end(); ++it)
    {
        if (it->id == id)
        {
            task = &(*it);
            (*out) = *task;
            return 0;
        }
    }

    return 1;
}

void STQQueue::mainLoop()
{
    if (!m_process)
    {
        m_stopped = true;
        return;
    }

    STQTask task;
    {
        std::unique_lock<std::mutex> lock(m_currentTaskMutex);
        m_currentTask = nullptr;
    }

    std::string errorLog;
    errorLog.reserve(4096);
    size_t bufSize(4096);
    std::string fileName;

    while (1)
    {
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_condition.wait(lock, [this]()
            {
                return !m_pending.empty() || m_terminate;
            });

            if (m_terminate)
            {
                m_terminate = false;
                break;
            }

            task = m_pending.front();
            m_pending.pop_front();
        }

        {
            std::unique_lock<std::mutex> lock(m_currentTaskMutex);
            m_currentTask = &task;
        }

        if (m_process->start(task.execName.c_str(), task.workDir.c_str(), task.args))
        {
            // failed
            errorLog.clear();
            errorLog += __FILE__;
            errorLog += ":";
            errorLog += std::to_string(__LINE__);
            errorLog += " Fail to start process. Task details:\n";

            // print task details
            errorLog += "name: ";
            errorLog += task.execName;
            errorLog += "\n";

            errorLog += "working dir: ";
            errorLog += task.workDir;
            errorLog += "\n";

            errorLog += "args: ";
            if (task.args.size())
            {
                for (auto it = task.args.begin(); it != task.args.end(); ++it)
                {
                    errorLog += (*it);
                    errorLog += " ";
                }
            }

            Global::logger.write(Logger::Error, errorLog.c_str());
            task.isSuccess = false;
            toFinishedQueue(&task);
            continue;
        }

        fileName = m_name + "-" + std::to_string(task.id) + ".log";
        fileName = Global::outFilePath + fileName;
        FILE *f;
        while (m_process->isRunning())
        {
            {
                std::unique_lock<std::mutex> lock(m_outMutex);
                if (m_process->readStdOut(m_out, &bufSize))
                {
                    bufSize = 4096;
                    errorLog.clear();
                    errorLog += __FILE__;
                    errorLog += ":";
                    errorLog += std::to_string(__LINE__);
                    errorLog += " Fail to read child process' stdout.";
                    Global::logger.write(Logger::Error, errorLog.c_str());
                    continue;
                }

                f = fopen(fileName.c_str(), "a");
                if (!f)
                {
                    bufSize = 4096;
                    errorLog.clear();
                    errorLog += __FILE__;
                    errorLog += ":";
                    errorLog += std::to_string(__LINE__);
                    errorLog += " Fail to open log file.";
                    Global::logger.write(Logger::Error, errorLog.c_str());
                    continue;
                }

                fwrite(m_out, 1, bufSize, f);
                fclose(f);
                f = nullptr;
            }
        } // end while (m_process->isRunning())

        toFinishedQueue(&task);
    } // end while(1)
}

void STQQueue::toFinishedQueue(STQTask *in)
{
    {
        std::unique_lock<std::mutex> lock(m_currentTaskMutex);
        m_currentTask = nullptr;
    }

    if (!in) return;

    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        m_finished.push_back(*in);
    }
}
