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
    m_isNotMobile(true),
#ifndef STQ_MOBILE
    m_backendMode(SQLITE)
#endif
{
    m_isInit.store(false, std::memory_order_relaxed);

    m_logBuf.reserve(128);

#ifdef STQ_MOBILE
    m_isNotMobile = false;
#else
    m_isNotMobile = true;
#endif
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

    if (!m_engine)
    {
        spdlog::warn("{}:{} engine is nullptr", __FILE__, __LINE__);
        return 1;
    }

    return 0;
}

bool Global::isNotMobile() const
{
    return m_isNotMobile;
}

bool Global::isLocalAvailable() const
{
#ifdef STQ_MOBILE
    return false;
#else
    return Controller::Global::sqliteQueueList != nullptr;
#endif
}

void Global::setEngine(QQmlApplicationEngine *in)
{
    m_engine = in;
}

QQmlApplicationEngine *Global::engine() const
{
    return m_engine;
}

void Global::setBackendMode(BackendMode in)
{
#ifdef STQ_MOBILE
    UNUSED(in);
    return;
#else
    m_backendMode = in;
    m_grpcConnect = nullptr;
#endif
}

Global::BackendMode Global::backendMode() const
{
#ifdef STQ_MOBILE
    return GRPC;
#else
    return m_backendMode;
#endif
}

void Global::setBackendModeQml(int in)
{
#ifdef STQ_MOBILE
    UNUSED(in);
    return;
#else
    if (in > 1 || in < 0)
    {
        spdlog::error("{}:{} Invalid mode: {}", __FILE__, __LINE__, in);
        return;
    }

    m_backendMode = static_cast<BackendMode>(in);
    m_grpcConnect = nullptr;
#endif
}

int Global::backendModeQml() const
{
#ifdef STQ_MOBILE
    return static_cast<int>(GRPC);
#else
    return static_cast<int>(m_backendMode);
#endif
}

void
Global::setConnectToken(BackendMode mode,
                        std::shared_ptr<Model::DAO::IConnect> &in)
{
#ifdef STQ_MOBILE
    UNUSED(mode);
    m_grpcConnect = in;
#else
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
#endif
}

std::shared_ptr<Model::DAO::IConnect>
Global::connectToken() const
{
#ifdef STQ_MOBILE
    return m_grpcConnect;
#else
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
#endif
}

void
Global::setQueueList(BackendMode mode,
                     std::shared_ptr<Model::DAO::IQueueList> &in)
{
#ifdef STQ_MOBILE
    UNUSED(mode);
    m_grpcQueueList = in;
#else
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
#endif
}

std::shared_ptr<Model::DAO::IQueueList>
Global::queueList() const
{
#ifdef STQ_MOBILE
    return m_grpcQueueList;
#else
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
#endif
}

void Global::setQueue(BackendMode mode, std::shared_ptr<Model::DAO::IQueue> &in)
{
#ifdef STQ_MOBILE
    UNUSED(mode);
    m_sqliteQueue = in;
#else
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
#endif
}

std::shared_ptr<Model::DAO::IQueue>
Global::queue() const
{
#ifdef STQ_MOBILE
    return m_sqliteQueue;
#else
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
#endif
}

void Global::notifyClosing()
{
    emit WindowClosing();
}

void Global::notifyAllCleaned()
{
    emit AllCleaned();
}

#ifndef STQ_MOBILE
void Global::saveFile(const QString &fileName, const QString &text)
{
    std::fstream f;
    f.open(fileName.toUtf8().toStdString(),
           std::ifstream::out | std::ifstream::trunc);
    if (f.fail())
    {
        spdlog::error("{}:{} Fail to open file", __FILE__, __LINE__);
        return;
    }

    std::string output = text.toUtf8().toStdString();
    f.write(output.c_str(), output.size());
    f.close();
}
#endif

QJSValue Global::getLog()
{
    std::unique_lock<std::mutex> lock(m_logMutex);
    QJSValue ret = m_engine->newArray(m_logBuf.length());
    if (!m_logBuf.length()) return ret;

    for (auto i = 0; i < m_logBuf.length(); ++i)
    {
        ret.setProperty(i, m_logBuf.at(i));
    }

    m_logBuf.clear();
    return ret;
}

void Global::onSpdlogLog(const QString &in)
{
    std::unique_lock<std::mutex> lock(m_logMutex);
    m_logBuf.push_back(in);
}

} // namespace GUI

} // end namespace Controller
