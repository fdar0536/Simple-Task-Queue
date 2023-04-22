/* Simple Task Queue
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

#include "QCoreApplication"
#include "controller/global/init.hpp"
#include "model/utils.hpp"
#include "global.hpp"
#include "clientconfigthread.hpp"

namespace Controller
{

namespace GUI
{

ClientConfigThread::ClientConfigThread(QObject *parent) :
    QThread(parent)
{
    m_isInit.store(false, std::memory_order_relaxed);
    m_mode.store(INIT, std::memory_order_relaxed);
}

ClientConfigThread::~ClientConfigThread()
{}

void ClientConfigThread::init()
{
    m_mode.store(INIT, std::memory_order_relaxed);
    start();
}

void ClientConfigThread::run()
{
    (this->*m_handler[m_mode.load(std::memory_order_relaxed)])();
}

// private member functions
void ClientConfigThread::initImpl()
{
    if (m_isInit.load(std::memory_order_relaxed))
    {
        spdlog::warn("{}:{} Already initialized", __FILE__, __LINE__);
        return;
    }

    // verify settings
    QList<QString> keys = {"name", "ip", "port"};
    QList<int> values = {QMetaType::QString, QMetaType::QString, QMetaType::Int};
    QMap<QString, QVariant> map;
    QSettings settings;
    QList<QVariant> data = settings.value("Server List", QList<QVariant>()).toList();

    if (data.length())
    {
        for (auto i = 0; i < data.length(); ++i)
        {
            if (data.at(i).userType() != QMetaType::QVariantMap)
            {
                data.clear();
                break;
            }

            map = data.at(i).toMap();

            for (int j = 0; j < 3; ++j)
            {
                if (map[keys[j]].userType() != values[j])
                {
                    data.clear();
                    spdlog::warn("{}:{} Type failed", __FILE__, __LINE__);
                    goto typeFailed;
                }

                if (Model::Utils::verifyIP(map["ip"].toString().toUtf8().toStdString()))
                {
                    data.clear();
                    spdlog::warn("{}:{} Type failed", __FILE__, __LINE__);
                    goto typeFailed;
                }
            }
        } // end for (auto i = 0; i < m_data.length(); ++i)
    } // end if (m_data.length())

typeFailed:

    if (!Controller::Global::guiGlobal.isNotMobile())
    {
        m_isInit.store(true, std::memory_order_relaxed);
        emit InitDone(data);
        return;
    }

    uint_fast8_t count(0);
    while (!Controller::Global::guiGlobal.isLocalAvailable())
    {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);
        ++count;
        if (count > 5)
        {
            break;
        }
    }

    m_isInit.store(true, std::memory_order_relaxed);
    emit InitDone(data);
}

void ClientConfigThread::connectToServer()
{}

} // namespace GUI

} // end namespace Controller
