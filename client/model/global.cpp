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

#include <iostream>

#include "QApplication"
#include "QDir"
#include "QJsonArray"
#include "QJsonDocument"
#include "QJsonObject"
#include "QJSValue"
#include "QMessageBox"
#include "QStandardPaths"

#include "global.hpp"

std::shared_ptr<Global> Global::m_instance = nullptr;

// public member functions
Global::Global() :
    QObject(),
    m_channel(nullptr),
    m_ipRegex(nullptr)
{}

Global::~Global()
{}

std::shared_ptr<Global> Global::instance()
{
    if (m_instance != nullptr) return m_instance;

    try
    {
        m_instance = std::make_shared<Global>();
    }
    catch (...)
    {
        m_instance = nullptr;
        return nullptr;
    }


    if (m_instance->initConfigFile())
    {
        m_instance = nullptr;
        return nullptr;
    }

    return m_instance;
}

uint8_t Global::state(const QString &key, QHash<QString, QVariant> &out)
{
    std::unique_lock<std::mutex> lock(m_stateMutex);
    auto it = m_stateStore.find(key);
    if (it == m_stateStore.end())
    {
        return 1;
    }

    out = it.value();
    return 0;
}

void Global::setState(const QString &key, QHash<QString, QVariant> &data)
{
    std::unique_lock<std::mutex> lock(m_stateMutex);
    m_stateStore[key] = data;
}

QHash<QString, QVariant> Global::settings()
{
    std::unique_lock<std::mutex> lock(m_stateMutex);
    return m_stateStore["settings"];
}

void Global::saveSettings(QHash<QString, QVariant> &input)
{
    {
        std::unique_lock<std::mutex> lock(m_stateMutex);
        m_stateStore["settings"] = input;
    }

    QList<QVariant> list = input["config"].toList();
    qint32 length = list.size();

    if (length == 0) return;

    QString path = input["configPath"].toString();
    QFile config(path);
    if (!config.open(QIODevice::WriteOnly))
    {
        std::cerr << __FILE__ << ":" << __LINE__;
        std::cerr << " Fail to open config file." << std::endl;
        return;
    }

    QJsonObject json;
    QJsonArray arr;

    for (qsizetype i = 0; i < length; ++i)
    {
        QJsonObject item;
        QHash<QString, QVariant> map = list[i].toHash();
        item.insert("alias", map["alias"].toString());
        item.insert("ip", map["ip"].toString());
        item.insert("port", map["port"].toInt());
        arr.append(item);
    }

    json.insert("config", arr);
    QJsonDocument doc(json);
    config.write(doc.toJson());
    config.close();
}

bool Global::isSettingsNotAccepted()
{
    std::unique_lock<std::mutex> lock(m_channelMutex);
    return (m_channel == nullptr);
}

std::shared_ptr<grpc::ChannelInterface> Global::grpcChannel()
{
    std::unique_lock<std::mutex> lock(m_channelMutex);
    return m_channel;
}

void Global::setGrpcChannel(std::shared_ptr<grpc::ChannelInterface> &in)
{
    std::unique_lock<std::mutex> lock(m_channelMutex);
    m_channel = in;
}

std::shared_ptr<QRegularExpressionValidator> Global::ipRegex() const
{
    return m_ipRegex;
}

// private member functions
uint8_t Global::initConfigFile()
{
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    if (configPath.isEmpty())
    {
        std::cerr << __FILE__ << ":" << __LINE__;
        std::cerr << " Fail to determine config location." << std::endl;
        return 1;
    }

    QDir dir;
    if (!dir.mkpath(configPath))
    {
        std::cerr << __FILE__ << ":" << __LINE__;
        std::cerr << " Fail to create config location." << std::endl;
        return 1;
    }

    configPath += "/config.json";
    QHash<QString, QVariant> output;
    output["configPath"] = configPath;

    QFile config(configPath);
    if (!config.open(QIODevice::ReadOnly))
    {
        std::cerr << __FILE__ << ":" << __LINE__;
        std::cerr << " Fail to open config file." << std::endl;
        return 0;
    }

    QByteArray data = config.readAll();
    config.close();
    QJsonObject configObj = QJsonDocument::fromJson(data).object();

    if (!configObj["config"].isArray())
    {
        std::cerr << __FILE__ << ":" << __LINE__;
        std::cerr << " Invalid config file." << std::endl;
        return 0;
    }

    QJsonArray arr = configObj["config"].toArray();
    QString tmpString;
    QList<QVariant> list;
    list.reserve(arr.size());

    QRegularExpression re("^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.)"
                          "{3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    re.optimize();

    try
    {
        m_ipRegex = std::make_shared<QRegularExpressionValidator>(re, this);
    }
    catch (...)
    {
        return 1;
    }

    int pos = 0;

    // note:
    // array.prototype.push is not working here
    for (int i = 0; i < arr.size(); ++i)
    {
        QJsonObject obj = arr[i].toObject();
        QHash<QString, QVariant> out;

        if (!obj["alias"].isString())
        {
            std::cerr << __FILE__ << ":" << __LINE__;
            std::cerr << " Invalid config file." << std::endl;
            list.clear();
            goto exit;
        }

        out["alias"] = obj["alias"].toString();
        if (!obj["ip"].isString())
        {
            std::cerr << __FILE__ << ":" << __LINE__;
            std::cerr << " Invalid config file." << std::endl;
            list.clear();
            goto exit;
        }

        tmpString = obj["ip"].toString();
        if (m_ipRegex->validate(tmpString, pos) != QValidator::Acceptable)
        {
            std::cerr << __FILE__ << ":" << __LINE__;
            std::cerr << " Invalid ip." << std::endl;
            list.clear();
            goto exit;
        }

        out["ip"] = tmpString;

        if (!obj["port"].isDouble())
        {
            std::cerr << __FILE__ << ":" << __LINE__;
            std::cerr << " Invalid config file." << std::endl;
            list.clear();
            goto exit;
        }

        pos = obj["port"].toInt();
        if (pos > 65535 || pos < 0)
        {
            std::cerr << __FILE__ << ":" << __LINE__;
            std::cerr << " Invalid port." << std::endl;
            list.clear();
            goto exit;
        }

        out["port"] = pos;
        list.push_back(out);
    }

exit:

    output["config"] = list;
    m_stateStore["settings"] = output;
    return 0;
}
