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

#include "spdlog/spdlog.h"

#include "grpcconnect.hpp"
#include "grpcqueue.hpp"
#include "grpcqueuelist.hpp"
#include "grpcutils.hpp"

namespace Model
{

namespace DAO
{

GRPCQueueList::GRPCQueueList() :
    m_stub(nullptr)
{}

GRPCQueueList::~GRPCQueueList()
{}

void GRPCQueueList::init(std::shared_ptr<IConnect> &connect, ErrMsg &msg)
{
    if (connect == nullptr)
    {
        msg.setMsg(ErrMsg::INVALID_ARGUMENT, "connect is nullptr");
        spdlog::error("{}:{} connect is nullptr", __FILE__, __LINE__);
        return;
    }

    if (!connect->connectToken())
    {
        msg.setMsg(ErrMsg::INVALID_ARGUMENT, "connect token is nullptr");
        spdlog::error("{}:{} connect token is nullptr", __FILE__, __LINE__);
        return;
    }

    GRPCToken *token = reinterpret_cast<GRPCToken *>(connect->connectToken());
    try
    {
        m_stub = stq::QueueList::NewStub(token->channel);
        if (m_stub == nullptr)
        {
            msg.setMsg(ErrMsg::OS_ERROR, "Fail to get stub");
            spdlog::error("{}:{} Fail to get stub", __FILE__, __LINE__);
            return;
        }
    }
    catch (...)
    {
        msg.setMsg(ErrMsg::OS_ERROR, "Fail to get stub");
        spdlog::error("{}:{} Fail to get stub", __FILE__, __LINE__);
        return;
    }

    m_conn = connect;
}

void GRPCQueueList::createQueue(const std::string &name, ErrMsg &msg)
{
    stq::QueueReq req;
    req.set_name(name);

    stq::Empty res;
    grpc::ClientContext ctx;

    GRPCUtils::setupCtx(ctx);
    grpc::Status status = m_stub->Create(&ctx, req, &res);
    if (status.ok())
    {
        return;
    }

    msg.setMsg(ErrMsg::OS_ERROR, status.error_message());
    GRPCUtils::buildErrMsg(__FILE__, __LINE__, status);
}

void GRPCQueueList::listQueue(std::vector<std::string> &out, ErrMsg &msg)
{
    out.clear();
    out.reserve(100);

    stq::Empty req;
    stq::ListQueueRes res;
    grpc::ClientContext ctx;

    GRPCUtils::setupCtx(ctx);
    auto reader = m_stub->List(&ctx, req);
    if (reader == nullptr)
    {
        msg.setMsg(ErrMsg::OS_ERROR, "reader is nullptr");
        spdlog::error("{}:{} reader is nullptr", __FILE__, __LINE__);
        return;
    }

    while (reader->Read(&res))
    {
        out.push_back(res.name());
    }

    grpc::Status status = reader->Finish();
    if (status.ok())
    {
        return;
    }

    msg.setMsg(ErrMsg::OS_ERROR, status.error_message());
    GRPCUtils::buildErrMsg(__FILE__, __LINE__, status);
}

void GRPCQueueList::deleteQueue(const std::string &name, ErrMsg &msg)
{
    stq::QueueReq req;
    req.set_name(name);

    stq::Empty res;
    grpc::ClientContext ctx;

    GRPCUtils::setupCtx(ctx);
    grpc::Status status = m_stub->Delete(&ctx, req, &res);
    if (status.ok())
    {
        return;
    }

    msg.setMsg(ErrMsg::OS_ERROR, status.error_message());
    GRPCUtils::buildErrMsg(__FILE__, __LINE__, status);
}

void GRPCQueueList::renameQueue(const std::string &oldName,
                                const std::string &newName,
                                ErrMsg &msg)
{
    stq::RenameQueueReq req;
    req.set_oldname(oldName);
    req.set_newname(newName);

    stq::Empty res;
    grpc::ClientContext ctx;

    GRPCUtils::setupCtx(ctx);
    grpc::Status status = m_stub->Rename(&ctx, req, &res);
    if (status.ok())
    {
        return;
    }

    msg.setMsg(ErrMsg::OS_ERROR, status.error_message());
    GRPCUtils::buildErrMsg(__FILE__, __LINE__, status);
}

std::shared_ptr<IQueue> GRPCQueueList::getQueue(const std::string &name)
{
    stq::QueueReq req;
    req.set_name(name);

    stq::Empty res;
    grpc::ClientContext ctx;

    GRPCUtils::setupCtx(ctx);
    grpc::Status status = m_stub->GetQueue(&ctx, req, &res);
    if (status.ok())
    {
        GRPCQueue *queue = new (std::nothrow) GRPCQueue;
        if (!queue)
        {
            spdlog::error("{}:{} Fail to allocate memory", __FILE__, __LINE__);
            return nullptr;
        }

        std::shared_ptr<Proc::IProc> proc = std::shared_ptr<Proc::IProc>(nullptr);
        ErrMsg::ErrCode code;
        ErrMsg msg;
        queue->init(m_conn, proc, name, msg);
        msg.msg(&code, nullptr);
        if (code != ErrMsg::OK)
        {
            spdlog::error("{}:{} Fail to initialize queue", __FILE__, __LINE__);
            delete queue;
            return nullptr;
        }

        return std::shared_ptr<IQueue>(queue);
    }

    GRPCUtils::buildErrMsg(__FILE__, __LINE__, status);
    return nullptr;
}

} // end namespace DAO

} // end namespace Model
