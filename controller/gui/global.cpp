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

#include "global.hpp"

#ifndef STQ_MOBILE
#include <fstream>
#endif

namespace Controller
{

namespace GUI
{

Global::Global(QObject *parent) :
    QObject(parent),
    m_backendMode(SQLITE)
{
    m_isInit.store(false, std::memory_order_relaxed);
    m_logBuf.reserve(1024);
}

Global::~Global()
{}

uint_fast8_t Global::init()
{
    if (m_isInit.load(std::memory_order_relaxed))
    {
        spdlog::warn("{}:{} Global is initialized", __FILE__, __LINE__);
        return 0;
    }

    return 0;
}

bool Global::isLocalAvailable() const
{
    return Controller::Global::sqliteQueueList != nullptr;
}

void Global::setBackendMode(BackendMode in)
{
    m_backendMode = in;
    m_grpcConnect = nullptr;
}

Global::BackendMode Global::backendMode() const
{
    return m_backendMode;
}

void
Global::setConnectToken(BackendMode mode,
                        std::shared_ptr<Model::DAO::IConnect> &in)
{
    switch(mode)
    {
    case GRPC:
    {
        m_grpcConnect = in;
        break;
    }
    case SQLITE:
    {
        m_sqliteConnect = in;
        break;
    }
    }
}

std::shared_ptr<Model::DAO::IConnect>
Global::connectToken() const
{
    switch(m_backendMode)
    {
    case GRPC:
    {
        return m_grpcConnect;
    }
    case SQLITE:
    {
        return m_sqliteConnect;
    }
    }
}

void
Global::setQueueList(BackendMode mode,
                     std::shared_ptr<Model::DAO::IQueueList> &in)
{
    switch(mode)
    {
    case GRPC:
    {
        m_grpcQueueList = in;
        break;
    }
    case SQLITE:
    {
        Controller::Global::sqliteQueueList = in;
        break;
    }
    }
}

std::shared_ptr<Model::DAO::IQueueList>
Global::queueList() const
{
    switch(m_backendMode)
    {
    case GRPC:
    {
        return m_grpcQueueList;
    }
    case SQLITE:
    {
        return Controller::Global::sqliteQueueList;
    }
    }
}

void Global::setQueue(BackendMode mode, std::shared_ptr<Model::DAO::IQueue> &in)
{
    switch(mode)
    {
    case GRPC:
    {
        m_grpcQueue = in;
        break;
    }
    case SQLITE:
    {
        m_sqliteQueue = in;
        break;
    }
    }
}

std::shared_ptr<Model::DAO::IQueue>
Global::queue() const
{
    switch(m_backendMode)
    {
    case GRPC:
    {
        return m_grpcQueue;
    }
    case SQLITE:
    {
        return m_sqliteQueue;
    }
    }
}

void Global::getLog(QList<QString> &out)
{
    std::unique_lock<std::mutex> lock(m_logMutex);
    out = m_logBuf;
    m_logBuf.clear();
}

void Global::onSpdlogLog(const QString &in)
{
    std::unique_lock<std::mutex> lock(m_logMutex);
    m_logBuf.push_back(in);
}

} // namespace GUI

} // end namespace Controller
