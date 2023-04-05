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

Global::Global() :
#ifdef STQ_MOBILE
    QObject(nullptr),
#else
    QWidget(nullptr),
#endif
    m_isNotMobile(true),
    m_settings("STQGuiSettings", QSettings::NativeFormat),
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

#ifndef STQ_MOBILE
void Global::setBackendMode(BackendMode in)
{
    m_backendMode = in;
}

Global::BackendMode Global::backendMode() const
{
    return m_backendMode;
}

void
Global::setSqliteQueueList(std::shared_ptr<Model::DAO::IQueueList<Model::DAO::SQLiteToken>> &in)
{
    m_sqliteQueueList = in;
}

std::shared_ptr<Model::DAO::IQueueList<Model::DAO::SQLiteToken>>
Global::sqliteQueueList() const
{
    return m_sqliteQueueList;
}
#endif // STQ_MOBILE

} // namespace GUI

} // end namespace Controller