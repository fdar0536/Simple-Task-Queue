/* This file is modified from spdlog
 * Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
 * Distributed under the MIT License (http://opensource.org/licenses/MIT)
 *
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

#include "spdlog/spdlog.h"
#include "QHash"
#include "QSettings"

#include "controller/global/init.hpp"
#include "controller/gui/global.hpp"
#include "model/utils.hpp"
#include "model/dao/grpcconnect.hpp"
#include "model/dao/grpcqueuelist.hpp"
#include "clientconfigthread.hpp"

namespace Controller
{

namespace GUI
{

ClientConfigThread::ClientConfigThread(QObject *parent) :
    QThread(parent)
{}

ClientConfigThread::~ClientConfigThread()
{}

uint_fast8_t
ClientConfigThread::startParseConfig(QHash<QString, QVariant> *output)
{
    if (!output)
    {
        spdlog::error("{}:{} Invalid input", __FILE__, __LINE__);
        return 1;
    }

    if (m_isRunning.load(std::memory_order_relaxed))
    {
        spdlog::error("{}:{} Thread is running", __FILE__, __LINE__);
        return 1;
    }

    m_mode.store(PARSECONFIG, std::memory_order_relaxed);
    m_data = output;
    start();
    return 0;
}

uint_fast8_t
ClientConfigThread::startConnect(const QString &host, const int_fast32_t port)
{
    if (m_isRunning.load(std::memory_order_relaxed))
    {
        spdlog::error("{}:{} Thread is running", __FILE__, __LINE__);
        return 1;
    }

    m_mode.store(CONNECT, std::memory_order_relaxed);
    m_host = host.toUtf8().toStdString();
    m_port = port;
    start();
    return 0;
}

void ClientConfigThread::run()
{
    m_isRunning.store(true, std::memory_order_relaxed);
    (this->*m_handler[m_mode.load(std::memory_order_relaxed)])();
    m_isRunning.store(false, std::memory_order_relaxed);
}

// private member functions
void ClientConfigThread::parseConfigImpl()
{
    QList<QString> keys = {"ip", "port"};
    QList<int> values = {QMetaType::QString, QMetaType::Int};
    QHash<QString, QVariant> hash;
    QSettings settings;
    QHash<QString, QVariant> data = settings.value("Server List", QHash<QString, QVariant>()).toHash();

    if (data.count())
    {
        for (auto it = data.begin(); it != data.end(); ++it)
        {
            if (it.value().userType() != QMetaType::QVariantHash)
            {
                data.clear();
                break;
            }

            hash = it.value().toHash();
            for (int j = 0; j < 2; ++j)
            {
                if (hash[keys[j]].userType() != values[j])
                {
                    data.clear();
                    spdlog::warn("{}:{} Type failed", __FILE__, __LINE__);
                    goto typeFailed;
                }
            }

            if (Model::Utils::verifyIP(hash["ip"].toString().toUtf8().toStdString()))
            {
                data.clear();
                spdlog::warn("{}:{} Type failed", __FILE__, __LINE__);
                goto typeFailed;
            }
        } // end for (auto i = 0; i < m_data.length(); ++i)
    } // end if (data.count())

    *m_data = data;

typeFailed:

    emit parseConfigDone();
}

void ClientConfigThread::connectImpl()
{
    std::shared_ptr<Model::DAO::IConnect> connectPtr;
    std::shared_ptr<Model::DAO::IQueueList> queueListPtr;

    Model::DAO::GRPCConnect *connect = new (std::nothrow) Model::DAO::GRPCConnect();
    if (!connect)
    {
        spdlog::error("{}:{} {}", __FILE__, __LINE__, "Fail to allocate memory");
        emit connectDone(false);
        return;
    }

    Model::ErrMsg errMsg;
    Model::ErrMsg::ErrCode code;
    connect->startConnect(errMsg, m_host, m_port);
    errMsg.msg(&code, nullptr);

    if (code != Model::ErrMsg::OK)
    {
        delete connect;
        emit connectDone(false);
        return;
    }

    connectPtr.reset(connect);
    Controller::Global::guiGlobal.setConnectToken(Controller::GUI::Global::GRPC,
                                                  connectPtr);

    Model::DAO::GRPCQueueList *queueList = new (std::nothrow) Model::DAO::GRPCQueueList();
    if (!queueList)
    {
        std::shared_ptr<Model::DAO::IConnect> tmp = nullptr;
        Controller::Global::guiGlobal.setConnectToken(Controller::GUI::Global::GRPC,
                                                      tmp);
        emit connectDone(false);
        return;
    }

    queueList->init(connectPtr, errMsg);
    errMsg.msg(&code, nullptr);
    if (code != Model::ErrMsg::OK)
    {
        delete queueList;
        std::shared_ptr<Model::DAO::IConnect> tmp = nullptr;
        Controller::Global::guiGlobal.setConnectToken(Controller::GUI::Global::GRPC,
                                                      tmp);
        emit connectDone(false);;
        return;
    }

    queueListPtr.reset(queueList);
    Controller::Global::guiGlobal.setQueueList(Controller::GUI::Global::GRPC,
                                               queueListPtr);
    emit connectDone(true);
}

} // namespace GUI

} // end namespace Controller
