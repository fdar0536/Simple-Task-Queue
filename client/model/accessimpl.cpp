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

#include <new>

#include "grpccommon.hpp"

#include "accessimpl.hpp"

// public member functions
AccessImpl::AccessImpl(QObject *parent) :
    AbstractClient(parent),
    m_stub(nullptr)
{
}

uint8_t AccessImpl::setChannel(std::shared_ptr<::grpc::ChannelInterface> &channel)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    try
    {
        m_stub = stq::Access::NewStub(channel);
        if (m_stub == nullptr) return 1;
    }
    catch (...)
    {
        m_stub = nullptr;
        return 1;
    }

    return 0;
}

uint8_t AccessImpl::echoTest()
{
    if (m_stub == nullptr)
    {
        return 1;
    }

    start();
    return 0;
}

// protected member functions
void AccessImpl::run()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    QString reason("");
    if (m_stub == nullptr)
    {
        reason = "stub is nullptr.";
        emit echoDone(false, reason);
        return;
    }

    stq::Empty req;
    stq::EchoRes res;
    grpc::ClientContext ctx;

    GrpcCommon::setupCtx(ctx);
    grpc::Status status = m_stub->Echo(&ctx, req, &res);
    if (status.ok())
    {
        emit echoDone(true, reason);
        return;
    }

    GrpcCommon::buildErrMsg(status, reason);
    emit echoDone(false, reason);
}
