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

#include "controller/global/init.hpp"

#include "queueimpl.hpp"

namespace Controller
{

namespace GRPCServer
{

grpc::Status
QueueImpl::ListPending(grpc::ServerContext *ctx,
                       const stq::QueueReq *req,
                       grpc::ServerWriter<stq::ListTaskRes> *writer)
{
    UNUSED(ctx);
    if (!req || !writer)
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return grpc::Status(grpc::StatusCode::INTERNAL, "Invalid input");
    }

    Model::ErrMsg msg;
    Model::ErrMsg::ErrCode code;
    std::string errMsg;

    auto queue = Global::sqliteQueueList->getQueue(req->name());
    if (queue == nullptr)
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Fail to get queue");
    }

    std::vector<int> out;
    queue->listPending(out, msg);
    msg.msg(&code, &errMsg);
    if (code != Model::ErrMsg::OK)
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return Model::ErrMsg::toGRPCStatus(code, errMsg);
    }

    stq::ListTaskRes res;
    for (auto it = out.begin(); it != out.end(); ++it)
    {
        res.set_id(*it);
        writer->Write(res);
    }

    return grpc::Status::OK;
}

grpc::Status
QueueImpl::ListFinished(grpc::ServerContext *ctx,
                        const stq::QueueReq *req,
                        grpc::ServerWriter<stq::ListTaskRes> *writer)
{
    UNUSED(ctx);
    if (!req || !writer)
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return grpc::Status(grpc::StatusCode::INTERNAL, "Invalid input");
    }

    Model::ErrMsg msg;
    Model::ErrMsg::ErrCode code;
    std::string errMsg;

    auto queue = Global::sqliteQueueList->getQueue(req->name());
    if (queue == nullptr)
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Fail to get queue");
    }

    std::vector<int> out;
    queue->listFinished(out, msg);
    msg.msg(&code, &errMsg);
    if (code != Model::ErrMsg::OK)
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return Model::ErrMsg::toGRPCStatus(code, errMsg);
    }

    stq::ListTaskRes res;
    for (auto it = out.begin(); it != out.end(); ++it)
    {
        res.set_id(*it);
        writer->Write(res);
    }

    return grpc::Status::OK;
}

static void
buildTaskDetailsRes(Model::Proc::Task &task, stq::TaskDetailsRes *res)
{
    if (!res)
    {
        spdlog::critical("{}:{} invalid input", __FILE__, __LINE__);
        return;
    }

    res->set_workdir(task.workDir);
    res->set_execname(task.execName);
    for (auto it = task.args.begin(); it != task.args.end(); ++it)
    {
        res->add_args(*it);
    }

    res->set_exitcode(task.exitCode);
    res->set_id(task.ID);
}

grpc::Status
QueueImpl::PendingDetails(grpc::ServerContext *ctx,
                          const stq::TaskDetailsReq *req,
                          stq::TaskDetailsRes *res)
{
    UNUSED(ctx);
    if (!req || !res)
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return grpc::Status(grpc::StatusCode::INTERNAL, "Invalid input");
    }

    Model::ErrMsg msg;
    Model::ErrMsg::ErrCode code;
    std::string errMsg;

    auto queue = Global::sqliteQueueList->getQueue(req->name());
    if (queue == nullptr)
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Fail to get queue");
    }

    Model::Proc::Task out;
    queue->pendingDetails(req->id(), out, msg);
    msg.msg(&code, &errMsg);
    if (code != Model::ErrMsg::OK)
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return Model::ErrMsg::toGRPCStatus(code, errMsg);
    }

    buildTaskDetailsRes(out, res);
    return grpc::Status::OK;
}

grpc::Status
QueueImpl::FinishedDetails(grpc::ServerContext *ctx,
                           const stq::TaskDetailsReq *req,
                           stq::TaskDetailsRes *res)
{
    UNUSED(ctx);
    if (!req || !res)
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return grpc::Status(grpc::StatusCode::INTERNAL, "Invalid input");
    }

    Model::ErrMsg msg;
    Model::ErrMsg::ErrCode code;
    std::string errMsg;

    auto queue = Global::sqliteQueueList->getQueue(req->name());
    if (queue == nullptr)
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Fail to get queue");
    }

    Model::Proc::Task out;
    queue->finishedDetails(req->id(), out, msg);
    msg.msg(&code, &errMsg);
    if (code != Model::ErrMsg::OK)
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return Model::ErrMsg::toGRPCStatus(code, errMsg);
    }

    buildTaskDetailsRes(out, res);
    return grpc::Status::OK;
}

grpc::Status
QueueImpl::ClearPending(grpc::ServerContext *ctx,
                        const stq::QueueReq *req,
                        stq::Empty *res)
{
    UNUSED(ctx);
    UNUSED(res);
    if (!req)
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return grpc::Status(grpc::StatusCode::INTERNAL, "Invalid input");
    }

    Model::ErrMsg msg;
    Model::ErrMsg::ErrCode code;
    std::string errMsg;

    auto queue = Global::sqliteQueueList->getQueue(req->name());
    if (queue == nullptr)
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Fail to get queue");
    }

    queue->clearPending(msg);
    msg.msg(&code, &errMsg);
    if (code != Model::ErrMsg::OK)
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return Model::ErrMsg::toGRPCStatus(code, errMsg);
    }

    return grpc::Status::OK;
}

grpc::Status
QueueImpl::ClearFinished(grpc::ServerContext *ctx,
                         const stq::QueueReq *req,
                         stq::Empty *res)
{
    UNUSED(ctx);
    UNUSED(res);
    if (!req)
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return grpc::Status(grpc::StatusCode::INTERNAL, "Invalid input");
    }

    Model::ErrMsg msg;
    Model::ErrMsg::ErrCode code;
    std::string errMsg;

    auto queue = Global::sqliteQueueList->getQueue(req->name());
    if (queue == nullptr)
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Fail to get queue");
    }

    queue->clearFinished(msg);
    msg.msg(&code, &errMsg);
    if (code != Model::ErrMsg::OK)
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return Model::ErrMsg::toGRPCStatus(code, errMsg);
    }

    return grpc::Status::OK;
}

grpc::Status
QueueImpl::CurrentTask(grpc::ServerContext *ctx,
                       const stq::QueueReq *req,
                       stq::TaskDetailsRes *res)
{
    UNUSED(ctx);
    if (!req || !res)
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return grpc::Status(grpc::StatusCode::INTERNAL, "Invalid input");
    }

    Model::ErrMsg msg;
    Model::ErrMsg::ErrCode code;
    std::string errMsg;

    auto queue = Global::sqliteQueueList->getQueue(req->name());
    if (queue == nullptr)
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Fail to get queue");
    }

    Model::Proc::Task out;
    queue->currentTask(out, msg);
    msg.msg(&code, &errMsg);
    if (code != Model::ErrMsg::OK)
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return Model::ErrMsg::toGRPCStatus(code, errMsg);
    }

    buildTaskDetailsRes(out, res);
    return grpc::Status::OK;
}

grpc::Status
QueueImpl::AddTask(grpc::ServerContext *ctx,
                   const stq::AddTaskReq *req,
                   stq::ListTaskRes *res)
{
    UNUSED(ctx);
    if (!req || !res)
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return grpc::Status(grpc::StatusCode::INTERNAL, "Invalid input");
    }

    Model::ErrMsg msg;
    Model::ErrMsg::ErrCode code;
    std::string errMsg;

    auto queue = Global::sqliteQueueList->getQueue(req->name());
    if (queue == nullptr)
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Fail to get queue");
    }

    Model::Proc::Task in;
    in.execName = req->execname();
    in.workDir = req->workdir();
    in.args.reserve(req->args_size());
    for (auto it = req->args().begin(); it != req->args().end(); ++it)
    {
        in.args.push_back(*it);
    }

    queue->addTask(in, msg);
    msg.msg(&code, &errMsg);
    if (code != Model::ErrMsg::OK)
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return Model::ErrMsg::toGRPCStatus(code, errMsg);
    }

    res->set_id(in.ID);
    return grpc::Status::OK;
}

grpc::Status
QueueImpl::RemoveTask(grpc::ServerContext *ctx,
                      const stq::TaskDetailsReq *req,
                      stq::Empty *res)
{
    UNUSED(ctx);
    UNUSED(res);
    if (!req)
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return grpc::Status(grpc::StatusCode::INTERNAL, "Invalid input");
    }

    Model::ErrMsg msg;
    Model::ErrMsg::ErrCode code;
    std::string errMsg;

    auto queue = Global::sqliteQueueList->getQueue(req->name());
    if (queue == nullptr)
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Fail to get queue");
    }

    queue->removeTask(req->id(), msg);
    msg.msg(&code, &errMsg);
    if (code != Model::ErrMsg::OK)
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return Model::ErrMsg::toGRPCStatus(code, errMsg);
    }

    return grpc::Status::OK;
}

grpc::Status
QueueImpl::IsRunning(grpc::ServerContext *ctx,
                     const stq::QueueReq *req,
                     stq::IsRunningRes *res)
{
    UNUSED(ctx);
    if (!req || !res)
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return grpc::Status(grpc::StatusCode::INTERNAL, "Invalid input");
    }

    auto queue = Global::sqliteQueueList->getQueue(req->name());
    if (queue == nullptr)
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Fail to get queue");
    }

    res->set_isrunning(queue->isRunning());
    return grpc::Status::OK;
}

grpc::Status
QueueImpl::ReadCurrentOutput(grpc::ServerContext *ctx,
                             const stq::QueueReq *req,
                             stq::Msg *res)
{
    UNUSED(ctx);
    if (!req || !res)
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return grpc::Status(grpc::StatusCode::INTERNAL, "Invalid input");
    }

    Model::ErrMsg msg;
    Model::ErrMsg::ErrCode code;
    std::string errMsg;

    auto queue = Global::sqliteQueueList->getQueue(req->name());
    if (queue == nullptr)
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Fail to get queue");
    }

    std::string output;
    queue->readCurrentOutput(output, msg);
    msg.msg(&code, &errMsg);
    if (code != Model::ErrMsg::OK)
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return Model::ErrMsg::toGRPCStatus(code, errMsg);
    }

    res->set_msg(output);
    return grpc::Status::OK;
}

grpc::Status
QueueImpl::Start(grpc::ServerContext *ctx,
                const stq::QueueReq *req,
                stq::Empty *res)
{
    UNUSED(ctx);
    UNUSED(res);
    if (!req)
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return grpc::Status(grpc::StatusCode::INTERNAL, "Invalid input");
    }

    Model::ErrMsg msg;
    Model::ErrMsg::ErrCode code;
    std::string errMsg;

    auto queue = Global::sqliteQueueList->getQueue(req->name());
    if (queue == nullptr)
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Fail to get queue");
    }

    queue->start(msg);
    msg.msg(&code, &errMsg);
    if (code != Model::ErrMsg::OK)
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return Model::ErrMsg::toGRPCStatus(code, errMsg);
    }

    return grpc::Status::OK;
}

grpc::Status
QueueImpl::Stop(grpc::ServerContext *ctx,
                const stq::QueueReq *req,
                stq::Empty *res)
{
    UNUSED(ctx);
    UNUSED(res);
    if (!req)
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return grpc::Status(grpc::StatusCode::INTERNAL, "Invalid input");
    }

    auto queue = Global::sqliteQueueList->getQueue(req->name());
    if (queue == nullptr)
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Fail to get queue");
    }

    queue->stop();
    return grpc::Status::OK;
}

} // end namespace GRPCServer

} // end namespace Controller
