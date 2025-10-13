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

#include "fmt/format.h"
#include "QDebug"
#include "QStandardPaths"
#include "yaml-cpp/yaml.h"

#include "global.hpp"

namespace Controller
{

namespace GUI
{

static Global *global = nullptr;

// public member functions
Global::~Global()
{}

Global *Global::instance()
{
    if (global)
    {
        return global;
    }

    global = new (std::nothrow) Global;
    if (!global) return nullptr;

    global->parseConfig();
    return global;
}

QQmlApplicationEngine *Global::engine()
{
    return &m_engine;
}

// private member functions
Global::Global(QObject *parent):
    QObject{parent}
{}

void Global::parseConfig()
{
    hostList.reserve(16);
    defaultConfig();

    QString path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    if (path.isEmpty())
    {
        qWarning(fmt::format("{}:{} Fail to get path to store config",
                             __FILE__, __LINE__).c_str());
        return;
    }

    path += "/config.yaml";
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
        defaultConfig();
    }
}

void Global::saveConfig()
{
    return;
}

void Global::defaultConfig()
{
    hostList.clear();
    QJSValue obj = m_engine.newObject();
    obj.setProperty("embedded", true);
    lastHost = QSharedPointer<QJSValue>::create(obj);
    hostList.push_back(lastHost);
}

} // namespace GUI

} // namespace Controller
