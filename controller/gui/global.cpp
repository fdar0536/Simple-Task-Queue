/*
 * Simple Task Queue
 * Copyright (c) 2025-present fdar0536
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

#include <fstream>

#include "fmt/format.h"
#include "QDebug"
#include "QMutexLocker"
#include "QStandardPaths"
#include "yaml-cpp/yaml.h"

#include "global.hpp"

namespace Controller
{

namespace GUI
{

static Global *global = nullptr;

// public member functions
Global *Global::instance()
{
    if (global)
    {
        return global;
    }

    global = new (std::nothrow) Global;
    if (!global)
    {
        qCritical(fmt::format("{}:{} Fail to allocate memory",
                              __FILE__, __LINE__).c_str());
        return nullptr;
    }

    global->parseConfig();
    return global;
}

void Global::destroy()
{
    if (global)
    {
        delete global;
        global = nullptr;
    }
}

QQmlApplicationEngine *Global::engine()
{
    return &m_engine;
}

QString Global::getStatus()
{
    QMutexLocker lock(&m_mutex);
    return m_status;
}

void Global::setStatus(const QString &in)
{
    {
        QMutexLocker lock(&m_mutex);
        m_status = in;
    }

    emit statusChanged(in);
}

// private member functions
Global::Global(QObject *parent):
    QObject{parent}
{}

Global::~Global()
{
    saveConfig();
}

void Global::parseConfig()
{
    hostList.reserve(16);
    getDefaultConfig();

    QString path;
    if (getConfigPath(path))
    {
        qWarning(fmt::format("{}:{} getConfigPath failed",
                             __FILE__, __LINE__).c_str());
        return;
    }

    QJSValue obj;
    try
    {
        YAML::Node config = YAML::LoadFile(path.toUtf8().toStdString());

        // last host
        obj = m_engine.newObject();
        auto lastHostConfig = config["last host"];
        obj.setProperty("name",
                        QString::fromStdString(
                            lastHostConfig["name"].as<std::string>()));

        obj.setProperty("host",
                        QString::fromStdString(
                            lastHostConfig["host"].as<std::string>()));

        obj.setProperty("port",
                        lastHostConfig["port"].as<u16>());

        obj.setProperty("embedded",
                        lastHostConfig["embedded"].as<bool>());

        lastHost = QSharedPointer<QJSValue>::create(obj);
        hostList.push_back(lastHost);

        auto hostListNodes = config["host list"];
        hostList.reserve(hostListNodes.size() + 2);
        for (size_t i = 0; i < hostListNodes.size(); ++i)
        {
            obj = m_engine.newObject();
            auto node = hostListNodes[i];

            obj.setProperty("name",
                            QString::fromStdString(
                                node["name"].as<std::string>()));

            obj.setProperty("host",
                            QString::fromStdString(
                                node["host"].as<std::string>()));

            obj.setProperty("port",
                            node["port"].as<u16>());

            obj.setProperty("embedded",
                            node["embedded"].as<bool>());

            hostList.push_back(QSharedPointer<QJSValue>::create(obj));
        }
    }
    catch (...)
    {
        qWarning(fmt::format("{}:{} Fail to parse config, fallback to default",
                             __FILE__, __LINE__).c_str());
        getDefaultConfig();
    }
}

void Global::saveConfig()
{
    QString path;
    if (getConfigPath(path))
    {
        qWarning(fmt::format("{}:{} getConfigPath failed",
                             __FILE__, __LINE__).c_str());
        return;
    }

    YAML::Node config;

    // last host
    YAML::Node lastHostNode;
    lastHostNode["name"] = lastHost->property("name")
                               .toString().toUtf8().toStdString();
    lastHostNode["host"] = lastHost->property("host")
                               .toString().toUtf8().toStdString();
    lastHostNode["port"] = lastHost->property("port").toUInt();
    lastHostNode["embedded"] = lastHost->property("embedded").toBool();
    config["last host"] = lastHostNode;

    YAML::Node hostListNodes;
    for (qsizetype i = 0; i < hostList.size(); ++i)
    {
        YAML::Node node;
        node["name"] = hostList.at(i)->property("name")
                                   .toString().toUtf8().toStdString();
        node["host"] = hostList.at(i)->property("host")
                                   .toString().toUtf8().toStdString();
        node["port"] = hostList.at(i)->property("port").toUInt();
        node["embedded"] = hostList.at(i)->property("embedded").toBool();
        hostListNodes.push_back(node);
    }

    config["host list"] = hostListNodes;

    YAML::Emitter emitter;
    emitter << config;

    try
    {
        std::ofstream fout(path.toUtf8().toStdString());
        fout << emitter.c_str();
    }
    catch (...)
    {
        qCritical(fmt::format("{}:{} Fail to write file",
                              __FILE__, __LINE__).c_str());
    }
}

void Global::getDefaultConfig()
{
    hostList.clear();
    QJSValue obj = m_engine.newObject();
    obj.setProperty("embedded", true);
    lastHost = QSharedPointer<QJSValue>::create(obj);
    hostList.push_back(lastHost);
}

u8 Global::getConfigPath(QString &path)
{
    path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    if (path.isEmpty())
    {
        qWarning(fmt::format("{}:{} Fail to get path to store config",
                             __FILE__, __LINE__).c_str());
        return 1;
    }

    path += "/config.yaml";
    return 0;
}

} // namespace GUI

} // namespace Controller
