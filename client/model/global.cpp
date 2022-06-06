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
#include <new>

#include "QApplication"
#include "QDir"
#include "QJsonArray"
#include "QJsonDocument"
#include "QJsonObject"
#include "QJSValue"
#include "QMessageBox"
#include "QRegularExpressionValidator"
#include "QStandardPaths"

#include "global.hpp"

Global *Global::m_instance = nullptr;

// public member functions
Global::~Global()
{}

Global *Global::instance(QQmlApplicationEngine *engine)
{
    if (!engine)
    {
        if (m_instance) return m_instance;
        return nullptr;
    }

    m_instance = new (std::nothrow) Global;
    if (!m_instance) return nullptr;

    m_instance->m_engine = engine;
    if (m_instance->initConfigFile())
    {
        delete m_instance;
        m_instance = nullptr;
        return nullptr;
    }

    m_instance->initState();
    return m_instance;
}

QJSValue Global::state(QString key)
{
    auto it = m_stateStore.find(key);
    if (it == m_stateStore.end())
    {
        return m_engine->newObject();
    }

    return it.value();
}

void Global::setState(QString key, QJSValue data)
{
    m_stateStore[key] = data;
}

QJSValue Global::settings()
{
    return m_stateStore["settings"];
}

void Global::saveSettings(QJSValue input)
{
    m_stateStore["settings"] = input;
    QJSValue list = input.property("config");
    qint32 length = list.property("length").toInt();

    if (length == 0) return;

    QString path = input.property("configPath").toString();
    qDebug() << "path is: " << path;
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
        QJSValue map = list.property(i);
        item.insert("alias", map.property("alias").toString());
        item.insert("ip", map.property("ip").toString());
        item.insert("port", map.property("port").toInt());
        arr.append(item);
    }

    json.insert("config", arr);
    QJsonDocument doc(json);
    config.write(doc.toJson());
    config.close();
}

bool Global::isSettingsNotAccepted()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    return (m_channel == nullptr);
}

std::shared_ptr<grpc::ChannelInterface> Global::grpcChannel()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_channel;
}

void Global::setGrpcChannel(std::shared_ptr<grpc::ChannelInterface> &in)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_channel = in;
}

QQmlApplicationEngine *Global::engine() const
{
    return m_engine;
}

void Global::programExit(int exitCode, QString reason)
{
    if (exitCode)
    {
        QMessageBox::critical(this,
                              tr("Error"),
                              reason);
    }

    QApplication::exit(exitCode);
}

// private member functions
Global::Global() :
    QWidget(),
    m_engine(nullptr),
    m_channel(nullptr)
{}

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
    QJSValue output = m_engine->newObject();
    output.setProperty("configPath", QJSValue(configPath));

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
    QJSValue list = m_engine->newArray(arr.size());

    QRegularExpression re("^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.)"
                          "{3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    re.optimize();

    QRegularExpressionValidator regex = QRegularExpressionValidator(re, nullptr);
    int pos = 0;

    // note:
    // array.prototype.push is not working here
    for (int i = 0; i < arr.size(); ++i)
    {
        QJsonObject obj = arr[i].toObject();
        QJSValue out = m_engine->newObject();

        if (!obj["alias"].isString())
        {
            std::cerr << __FILE__ << ":" << __LINE__;
            std::cerr << " Invalid config file." << std::endl;
            list = m_engine->newArray();
            goto exit;
        }

        out.setProperty("alias", QJSValue(obj["alias"].toString()));
        if (!obj["ip"].isString())
        {
            std::cerr << __FILE__ << ":" << __LINE__;
            std::cerr << " Invalid config file." << std::endl;
            list = m_engine->newArray();
            goto exit;
        }

        tmpString = obj["ip"].toString();
        if (regex.validate(tmpString, pos) != QValidator::Acceptable)
        {
            std::cerr << __FILE__ << ":" << __LINE__;
            std::cerr << " Invalid ip." << std::endl;
            list = m_engine->newArray();
            goto exit;
        }

        out.setProperty("ip", QJSValue(tmpString));

        if (!obj["port"].isDouble())
        {
            std::cerr << __FILE__ << ":" << __LINE__;
            std::cerr << " Invalid config file." << std::endl;
            list = m_engine->newArray();
            goto exit;
        }

        pos = obj["port"].toInt();
        if (pos > 65535 || pos < 0)
        {
            std::cerr << __FILE__ << ":" << __LINE__;
            std::cerr << " Invalid port." << std::endl;
            list = m_engine->newArray();
            goto exit;
        }

        out.setProperty("port", QJSValue(pos));
        list.setProperty(i, out);
    }

exit:

    output.setProperty("config", list);
    m_stateStore["settings"] = output;
    return 0;
}

void Global::initState()
{
    m_stateStore["settingsState"] = QJSValue("");
}
