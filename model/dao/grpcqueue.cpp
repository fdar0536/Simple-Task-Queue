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

#include "model/dao/grpcconnect.hpp"
#include "spdlog/spdlog.h"

#include "controller/global/defines.hpp"
#include "grpcutils.hpp"
#include "grpcqueue.hpp"

namespace Model
{

namespace DAO
{

GRPCQueue::GRPCQueue() :
    m_stub(nullptr),
    m_queueName("")
{}

GRPCQueue::~GRPCQueue()
{}

void
GRPCQueue::init(std::shared_ptr<IConnect> &connect,
                std::shared_ptr<Proc::IProc> &process,
                const std::string &name,
                ErrMsg &msg)
{
    UNUSED(process);
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

    if (name.empty())
    {
        msg.setMsg(ErrMsg::INVALID_ARGUMENT, "name is empty");
        spdlog::error("{}:{} name is empty", __FILE__, __LINE__);
        return;
    }

    GRPCToken *token = reinterpret_cast<GRPCToken *>(connect->connectToken());

    try
    {
        m_stub = stq::Queue::NewStub(token->channel);
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

    m_queueName = name;
}

void GRPCQueue::listPending(std::vector<int> &out, ErrMsg &msg)
{
    out.clear();
    out.reserve(128);

    stq::QueueReq req;
    req.set_name(m_queueName);

    grpc::ClientContext ctx;
    stq::ListTaskRes res;

    GRPCUtils::setupCtx(ctx);
    auto reader = m_stub->ListPending(&ctx, req);
    if (reader == nullptr)
    {
        msg.setMsg(ErrMsg::OS_ERROR, "reader is nullptr");
        spdlog::error("{}:{} reader is nullptr", __FILE__, __LINE__);
        return;
    }

    while(reader->Read(&res))
    {
        out.push_back(res.id());
    }

    UNUSED(reader->Finish());
}

void GRPCQueue::listFinished(std::vector<int> &out, ErrMsg &msg)
{
    out.clear();
    out.reserve(128);

    stq::QueueReq req;
    req.set_name(m_queueName);

    grpc::ClientContext ctx;
    stq::ListTaskRes res;

    GRPCUtils::setupCtx(ctx);
    auto reader = m_stub->ListFinished(&ctx, req);
    if (reader == nullptr)
    {
        msg.setMsg(ErrMsg::OS_ERROR, "reader is nullptr");
        spdlog::error("{}:{} reader is nullptr", __FILE__, __LINE__);
        return;
    }

    while(reader->Read(&res))
    {
        out.push_back(res.id());
    }

    UNUSED(reader->Finish());
}

void GRPCQueue::pendingDetails(const int_fast32_t id,
                               Proc::Task &out,
                               ErrMsg &msg)
{
    stq::TaskDetailsReq req;
    req.set_queuename(m_queueName);
    req.set_id(id);

    grpc::ClientContext ctx;
    stq::TaskDetailsRes res;

    GRPCUtils::setupCtx(ctx);
    grpc::Status status = m_stub->PendingDetails(&ctx, req, &res);
    if (status.ok())
    {
        buildTask(res, out);
        return;
    }

    msg.setMsg(ErrMsg::OS_ERROR, status.error_message());
    GRPCUtils::buildErrMsg(__FILE__, __LINE__, status);
}

void GRPCQueue::finishedDetails(const int_fast32_t id,
                                Proc::Task &out,
                                ErrMsg &msg)
{
    stq::TaskDetailsReq req;
    req.set_queuename(m_queueName);
    req.set_id(id);

    grpc::ClientContext ctx;
    stq::TaskDetailsRes res;

    GRPCUtils::setupCtx(ctx);
    grpc::Status status = m_stub->FinishedDetails(&ctx, req, &res);
    if (status.ok())
    {
        buildTask(res, out);
        return;
    }

    msg.setMsg(ErrMsg::OS_ERROR, status.error_message());
    GRPCUtils::buildErrMsg(__FILE__, __LINE__, status);
}

void GRPCQueue::clearPending(ErrMsg &msg)
{
    stq::QueueReq req;
    req.set_name(m_queueName);

    grpc::ClientContext ctx;
    stq::Empty res;

    GRPCUtils::setupCtx(ctx);
    grpc::Status status = m_stub->ClearPending(&ctx, req, &res);
    if (status.ok())
    {
        return;
    }

    msg.setMsg(ErrMsg::OS_ERROR, status.error_message());
    GRPCUtils::buildErrMsg(__FILE__, __LINE__, status);
}

void GRPCQueue::clearFinished(ErrMsg &msg)
{
    stq::QueueReq req;
    req.set_name(m_queueName);

    grpc::ClientContext ctx;
    stq::Empty res;

    GRPCUtils::setupCtx(ctx);
    grpc::Status status = m_stub->ClearFinished(&ctx, req, &res);
    if (status.ok())
    {
        return;
    }

    msg.setMsg(ErrMsg::OS_ERROR, status.error_message());
    GRPCUtils::buildErrMsg(__FILE__, __LINE__, status);
}

void GRPCQueue::currentTask(Proc::Task &out, ErrMsg &msg)
{
    stq::QueueReq req;
    req.set_name(m_queueName);

    grpc::ClientContext ctx;
    stq::TaskDetailsRes res;

    GRPCUtils::setupCtx(ctx);
    grpc::Status status = m_stub->CurrentTask(&ctx, req, &res);
    if (status.ok())
    {
        buildTask(res, out);
        return;
    }

    msg.setMsg(ErrMsg::OS_ERROR, status.error_message());
    GRPCUtils::buildErrMsg(__FILE__, __LINE__, status);
}

void GRPCQueue::addTask(Proc::Task &in, ErrMsg &msg)
{
    stq::AddTaskReq req;
    req.set_queuename(m_queueName);
    req.set_workdir(in.workDir);
    req.set_execname(in.execName);
    for (auto it = in.args.begin();
         it != in.args.end();
         ++it)
    {
        req.add_args(*it);
    }

    grpc::ClientContext ctx;
    stq::ListTaskRes res;

    GRPCUtils::setupCtx(ctx);
    grpc::Status status = m_stub->AddTask(&ctx, req, &res);
    if (status.ok())
    {
        in.ID = res.id();
        return;
    }

    msg.setMsg(ErrMsg::OS_ERROR, status.error_message());
    GRPCUtils::buildErrMsg(__FILE__, __LINE__, status);
}

void GRPCQueue::removeTask(const int_fast32_t in, ErrMsg &msg)
{
    stq::TaskDetailsReq req;
    req.set_queuename(m_queueName);
    req.set_id(in);

    grpc::ClientContext ctx;
    stq::Empty res;

    GRPCUtils::setupCtx(ctx);
    grpc::Status status;

    status = m_stub->RemoveTask(&ctx, req, &res);
    if (!status.ok())
    {
        msg.setMsg(ErrMsg::OS_ERROR, status.error_message());
        GRPCUtils::buildErrMsg(__FILE__, __LINE__, status);
    }
}

bool GRPCQueue::isRunning() const
{
    stq::QueueReq req;
    req.set_name(m_queueName);

    grpc::ClientContext ctx;
    stq::IsRunningRes res;

    GRPCUtils::setupCtx(ctx);
    grpc::Status status = m_stub->IsRunning(&ctx, req, &res);
    if (status.ok())
    {
        return res.isrunning();
    }

    GRPCUtils::buildErrMsg(__FILE__, __LINE__, status);
    return false;
}

void GRPCQueue::readCurrentOutput(std::string &out, ErrMsg &msg)
{
    stq::QueueReq req;
    req.set_name(m_queueName);

    grpc::ClientContext ctx;
    stq::Msg res;

    GRPCUtils::setupCtx(ctx);
    grpc::Status status = m_stub->ReadCurrentOutput(&ctx, req, &res);
    if (status.ok())
    {
        out = res.msg();
        return;
    }

    msg.setMsg(ErrMsg::OS_ERROR, status.error_message());
    GRPCUtils::buildErrMsg(__FILE__, __LINE__, status);
}

void GRPCQueue::start(ErrMsg &msg)
{
    stq::QueueReq req;
    req.set_name(m_queueName);

    grpc::ClientContext ctx;
    stq::Empty res;

    GRPCUtils::setupCtx(ctx);
    grpc::Status status = m_stub->Start(&ctx, req, &res);
    if (status.ok())
    {
        return;
    }

    msg.setMsg(ErrMsg::OS_ERROR, status.error_message());
    GRPCUtils::buildErrMsg(__FILE__, __LINE__, status);
}

void GRPCQueue::stop()
{
    stq::QueueReq req;
    req.set_name(m_queueName);

    grpc::ClientContext ctx;
    stq::Empty res;

    GRPCUtils::setupCtx(ctx);
    grpc::Status status = m_stub->Stop(&ctx, req, &res);
    if (status.ok())
    {
        return;
    }

    GRPCUtils::buildErrMsg(__FILE__, __LINE__, status);
}

// private member functions
void GRPCQueue::buildTask(stq::TaskDetailsRes &res, Proc::Task &task)
{
    task.workDir = res.workdir();
    task.execName = res.execname();
    task.args.clear();
    task.args.reserve(res.args_size());
    for (auto i = 0; i < res.args_size(); ++i)
    {
        task.args.push_back(res.args().at(i));
    }

    task.exitCode = res.exitcode();
    task.ID = res.id();
}

} // end namespace DAO

} // end namespace Model
