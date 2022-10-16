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

#include "model/grpccommon.hpp"

#include "fakeconsolemodel.hpp"

// public member functions
FakeConsoleModel *FakeConsoleModel::create(QObject *parent)
{
    auto global = Global::instance();
    if (global == nullptr) return nullptr;

    FakeConsoleModel *ret(nullptr);
    try
    {
        ret = new FakeConsoleModel(parent);
    }
    catch (...)
    {
        return nullptr;
    }

    try
    {
        ret->m_stub = stq::Console::NewStub(global->grpcChannel());
        if (ret->m_stub == nullptr)
        {
            delete ret;
            return nullptr;
        }
    }
    catch (...)
    {
        delete ret;
        return nullptr;
    }

    if (GrpcCommon::getQueueName(global, ret->m_queueName))
    {
        delete ret;
        return nullptr;
    }

    return ret;
}

uint8_t FakeConsoleModel::lastError(QString &out)
{
    if (m_runningFlag.load(std::memory_order_relaxed))
    {
        // main loop is running
        return 1;
    }

    out = m_lastError;
    return 0;
}

uint8_t FakeConsoleModel::startConsole()
{
    if (m_runningFlag.load(std::memory_order_relaxed))
    {
        // main loop is running
        return 1;
    }

    m_runningFlag.store(true, std::memory_order_relaxed);
    start();
    return 0;
}

uint8_t FakeConsoleModel::stopConsole()
{
    if (!m_runningFlag.load(std::memory_order_relaxed))
    {
        // main loop is not running
        return 1;
    }

    m_runningFlag.store(false, std::memory_order_relaxed);
    return 0;
}

void FakeConsoleModel::run()
{
    bool flag(false);
    grpc::ClientContext ctx;
    GrpcCommon::setupCtx(ctx);

    auto stub(m_stub->Output(&ctx));
    if (stub == nullptr)
    {
        m_lastError = QString("%1:%2 stub is nullptr.").arg(__FILE__).arg(__LINE__);
        m_runningFlag.store(false, std::memory_order_relaxed);
        emit errorOccurred();
        return;
    }

    stq::Msg msg;
    while(1)
    {
        // initialize
        if (flag)
        {
            if (m_runningFlag.load(std::memory_order_relaxed))
            {
                msg.set_msg("c");
            }
            else
            {
                // have to stop this loop
                msg.set_msg("f");
            }

            stub->Write(msg);
        }
        else
        {
            flag = true;
            msg.set_msg(m_queueName.toLocal8Bit().toStdString());
            stub->Write(msg);
        }

        if (!stub->Read(&msg))
        {
            break;
        }

        emit serverMsg(msg.msg().c_str());
        sleep(3);
    }

    grpc::Status status = stub->Finish();
    if (status.ok())
    {
        m_runningFlag.store(false, std::memory_order_relaxed);
        return;
    }

    GrpcCommon::buildErrMsg(status, m_lastError);
    m_runningFlag.store(false, std::memory_order_relaxed);
    emit errorOccurred();
}

// private memner functions
FakeConsoleModel::FakeConsoleModel(QObject *parent) :
    QThread(parent),
    m_lastError(""),
    m_queueName("")
{
    m_runningFlag.store(false, std::memory_order_relaxed);
}
