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
#include "clientconfig.hpp"

namespace Controller
{

namespace GUI
{

ClientConfig::ClientConfig(QObject *parent) :
    QAbstractItemModel(parent)
{
    m_isInit.store(false, std::memory_order_relaxed);
}

ClientConfig::~ClientConfig()
{
    if (m_thread) delete m_thread;
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

    m_thread = new (std::nothrow) ClientConfigThread;
    if (!m_thread)
    {
        spdlog::error("{}:{} Fail to allocate memory", __FILE__, __LINE__);
        return false;
    }

    connect(m_thread,
            &ClientConfigThread::InitDone,
            this,
            &ClientConfig::onThreadInitDone);

    connect(m_thread,
            &ClientConfigThread::ServerConnected,
            this,
            &ClientConfig::ServerConnected);

    m_thread->init();
    m_isInit.store(true, std::memory_order_relaxed);
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

int ClientConfig::rowCount(const QModelIndex &) const
{
    return m_data.length();
}

int ClientConfig::columnCount(const QModelIndex &) const
{
    return 2;
}

QVariant ClientConfig::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 ||
        index.row() >= m_data.length() ||
        index.column() < 0 ||
        index.column() > 2)
        return QVariant();

    if (role == IdRole)
        return index.row();
    else if (role == NameRole)
        return m_data.at(index.row()).toMap()["name"];
    return QVariant();
}

QModelIndex ClientConfig::index(int, int, const QModelIndex &) const
{
    return QModelIndex();
}


QModelIndex ClientConfig::parent(const QModelIndex &) const
{
    return QModelIndex();
}

// private slots
void ClientConfig::onThreadInitDone(const QList<QVariant> &config)
{
    m_data = config;
    emit InitDone();
}

} // namespace GUI

} // end namespace Controller
