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

#include "access.grpc.pb.h"

#include "grpccommon.hpp"
#include "settingsmodel.hpp"

// public member functions
SettingsModel *SettingsModel::create(QObject *parent)
{
    SettingsModel *ret(nullptr);

    try
    {
        ret = new SettingsModel(parent);
    }
    catch (...)
    {
        return nullptr;
    }

    ret->m_global = Global::instance();
    if (ret->m_global == nullptr)
    {
        delete ret;
        return nullptr;
    }

    return ret;
}

SettingsModel::~SettingsModel()
{}

void SettingsModel::startConnect(const QString &ip, int port)
{
    std::shared_ptr<grpc::ChannelInterface> null(nullptr);
    m_global->setGrpcChannel(null);
    m_ip = ip;
    m_port = static_cast<uint16_t>(port);
    m_hasError = false;
    m_lastError = "";

    start();
}

bool SettingsModel::hasError()
{
    return m_hasError;
}

QString SettingsModel::lastError()
{
    return m_lastError;
}

void SettingsModel::run()
{
    m_ip += ":";
    m_ip += QString::number(m_port);

    std::shared_ptr<grpc::ChannelInterface> channel;
    std::unique_ptr<stq::Access::Stub> stub;

    try
    {
        channel = grpc::CreateChannel(m_ip.toStdString(),
                                      grpc::InsecureChannelCredentials());

        if (channel == nullptr)
        {
            m_hasError = true;
            m_lastError = "Fail to create channel.";
            return;
        }

        stub = stq::Access::NewStub(channel);
        if (stub == nullptr)
        {
            m_hasError = true;
            m_lastError = "Fail to create access' stub.";
            return;
        }
    }
    catch (...)
    {
        m_hasError = true;
        m_lastError = "Fail to initialize connection.";
        return;
    }

    stq::Empty req;
    stq::EchoRes res;
    grpc::ClientContext ctx;

    GrpcCommon::setupCtx(ctx);
    grpc::Status status = stub->Echo(&ctx, req, &res);
    if (status.ok())
    {
        m_global->setGrpcChannel(channel);
        emit done();
        return;
    }

    m_hasError = true;
    GrpcCommon::buildErrMsg(status, m_lastError);
    emit done();
}

// private member function
SettingsModel::SettingsModel(QObject *parent) :
    QThread(parent),
    m_global(nullptr),
    m_ip("127.0.0.1"),
    m_port(12345),
    m_hasError(false),
    m_lastError("")
{}
