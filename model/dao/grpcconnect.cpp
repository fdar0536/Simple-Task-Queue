/*
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

#include <new>

#include "spdlog/spdlog.h"


#include "grpcconnect.hpp"
#include "grpcutils.hpp"

namespace Model
{

namespace DAO
{

GRPCToken::GRPCToken() :
    channel(nullptr)
{}

GRPCToken::~GRPCToken()
{}

GRPCConnect::GRPCConnect()
{}

GRPCConnect::~GRPCConnect()
{
    freeConnectToken<GRPCToken>();
}

void GRPCConnect::init(ErrMsg &)
{}

void GRPCConnect::startConnect(ErrMsg &msg,
                               const std::string &target,
                               const int_fast32_t port)
{
    GRPCToken *token = new (std::nothrow) GRPCToken;
    if (!token)
    {
        msg.setMsg(ErrMsg::INVALID_ARGUMENT, "Fail to allocate memory");
        spdlog::error("{}:{} Fail to allocate memory", __FILE__, __LINE__);
        return;
    }

    std::string ip = target;
    ip += ":";
    ip += std::to_string(port);
    std::unique_ptr<stq::Access::Stub> stub;

    try
    {
        token->channel = grpc::CreateChannel(ip,
                                             grpc::InsecureChannelCredentials());

        if (token->channel == nullptr)
        {
            msg.setMsg(ErrMsg::OS_ERROR, "Fail to create channel");
            spdlog::error("{}:{} Fail to create channel", __FILE__, __LINE__);
            delete token;
            return;
        }

        stub = stq::Access::NewStub(token->channel);
        if (stub == nullptr)
        {
            msg.setMsg(ErrMsg::OS_ERROR, "Fail to create access' stub");
            spdlog::error("{}:{} Fail to create access' stub", __FILE__, __LINE__);
            delete token;
            return;
        }
    }
    catch (...)
    {
        msg.setMsg(ErrMsg::OS_ERROR, "Fail to initialize connection");
        spdlog::error("{}:{} Fail to initialize connection", __FILE__, __LINE__);
        delete token;
        return;
    }

    stq::Empty req;
    stq::EchoRes res;
    grpc::ClientContext ctx;

    GRPCUtils::setupCtx(ctx);
    grpc::Status status = stub->Echo(&ctx, req, &res);
    if (status.ok())
    {
        m_connectToken = reinterpret_cast<void *>(token);
        return;
    }

    delete token;
    m_connectToken = nullptr;
    msg.setMsg(ErrMsg::OS_ERROR, status.error_message());
    GRPCUtils::buildErrMsg(__FILE__, __LINE__, status);
}

} // end namespace DAO

} // end namespace Model
