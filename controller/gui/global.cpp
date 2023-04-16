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

namespace Controller
{

namespace GUI
{

Global::Global(QObject *parent) :
    QObject(parent),
    m_isNotMobile(true),
    m_settings(nullptr),
#ifndef STQ_MOBILE
    m_backendMode(SQLITE)
#endif
{
    m_isInit.store(false, std::memory_order_relaxed);

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

    m_settings = new (std::nothrow) QSettings("STQGuiSettings",
                                              QSettings::NativeFormat);
    if (!m_settings)
    {
        spdlog::error("{}:{} Failed to allocate memory", __FILE__, __LINE__);
        return 1;
    }

    return 0;
}

bool Global::isNotMobile() const
{
    return m_isNotMobile;
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

QSettings *Global::settings()
{
    return m_settings;
}

void Global::setState(const QString &key, const QJSValue &value)
{
    m_state[key] = value;
}

QJSValue Global::state(const QString &key)
{
    auto it = m_state.find(key);
    if (it == m_state.end())
    {
        return m_engine->newObject();
    }

    return it->second;
}

void Global::notifyClosing()
{
    emit WindowClosing();
}

void Global::notifyAllCleaned()
{
    emit AllCleaned();
}

} // namespace GUI

} // end namespace Controller
