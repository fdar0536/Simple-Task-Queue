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
#include "QSettings"
#include "controller/global/init.hpp"
#include "model/utils.hpp"
#include "global.hpp"
#include "clientconfig.hpp"

namespace Controller
{

namespace GUI
{

static QList<QVariant> m_data;

ClientConfig::ClientConfig(QObject *parent) :
    QThread(parent)
{
    m_isInit.store(false, std::memory_order_relaxed);
    m_isRunning.store(false, std::memory_order_relaxed);
}

ClientConfig::~ClientConfig()
{
    if (m_data.length())
    {
        QSettings s;
        s.setValue("Server List", m_data);
    }
}

int ClientConfig::dataCount() const
{
    return m_data.length();
}

bool ClientConfig::init()
{
    if (m_isInit.load(std::memory_order_relaxed))
    {
        spdlog::warn("{}:{} Already initialized", __FILE__, __LINE__);
        return true;
    }

    m_mode.store(INIT, std::memory_order_relaxed);
    start();
    return true;
}

int ClientConfig::logLevel() const
{
    return static_cast<int>(Controller::Global::config.logLevel());
}

int ClientConfig::setLogLevel(int in)
{
    if (in < 0 || in > 6)
    {
        spdlog::error(
            "{}:{} Invalid level, use default value", __FILE__, __LINE__);
        in = 2;
    }

    Controller::Global::config.setLogLevel(
        static_cast<spdlog::level::level_enum>(in));

    return in;
}

QString ClientConfig::name(int in)
{
    if (in < 0 || in > (m_data.length() - 1))
    {
        return "";
    }

    return m_data.at(in).toMap()["name"].toString();
}

QString ClientConfig::ip(int in)
{
    if (in < 0 || in > (m_data.length() - 1))
    {
        return "";
    }

    return m_data.at(in).toMap()["ip"].toString();
}

int ClientConfig::port(int in)
{
    if (in < 0 || in > (m_data.length() - 1))
    {
        return 0;
    }

    return m_data.at(in).toMap()["port"].toInt();
}

bool ClientConfig::saveSetting(const QString &name,
                               const QString &ip,
                               const int port)
{
    if (Model::Utils::verifyIP(ip.toUtf8().toStdString()))
    {
        return false;
    }

    if (port < 0 || port > 65535)
    {
        return false;
    }

    QMap<QString, QVariant> map;
    map["name"] = name;
    map["ip"] = ip;
    map["port"] = port;
    m_data.push_back(map);

    return true;
}

QList<QVariant> *ClientConfig::data()
{
    return &m_data;
}

void ClientConfig::run()
{
    m_isRunning.store(true, std::memory_order_relaxed);
    (this->*m_handler[m_mode])();
    m_isRunning.store(false, std::memory_order_relaxed);
}

// private member functions
void ClientConfig::initImpl()
{
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
        emit InitDone();
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
    emit InitDone();
}

void ClientConfig::connectToServerImpl()
{}

} // namespace GUI

} // end namespace Controller
