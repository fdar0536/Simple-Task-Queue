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

#include "../common.hpp"
#include "implcommon.hpp"

#include "global.hpp"
#include "pendingimpl.hpp"

::grpc::Status
PendingImpl::List(::grpc::ServerContext *ctx,
                  const ::stq::QueueReq *req,
                  ::grpc::ServerWriter<::stq::ListTaskRes> *res)
{
    UNUSED(ctx);
    std::shared_ptr<STQQueue> queue = Global::queueList.getQueue(req->name());
    if (queue == nullptr)
    {
        return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                              "No such queue: " + req->name());
    }

    queue->listPanding(res);
    return ::grpc::Status::OK;
}

::grpc::Status
PendingImpl::Details(::grpc::ServerContext *ctx,
                     const ::stq::TaskDetailsReq *req,
                     ::stq::TaskDetailsRes *res)
{
    UNUSED(ctx);
    std::shared_ptr<STQQueue> queue = Global::queueList.getQueue(req->queuename());
    if (queue == nullptr)
    {
        return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                              "No such queue: " + req->queuename());
    }

    STQTask task;
    if (queue->pendingDetails(req->id(), &task))
    {
        return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                              "No such task: " + std::to_string(req->id()));
    }

    ImplCommon::buildTaskRes(res, task);
    return ::grpc::Status::OK;
}

::grpc::Status
PendingImpl::Current(::grpc::ServerContext *ctx,
                     const ::stq::QueueReq *req,
                     ::stq::TaskDetailsRes *res)
{
    UNUSED(ctx);

    std::shared_ptr<STQQueue> queue = Global::queueList.getQueue(req->name());
    if (queue == nullptr)
    {
        return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                              "No such queue: " + req->name());
    }

    STQTask task;
    if (queue->currentTask(&task))
    {
        task.execName = "";
    }

    ImplCommon::buildTaskRes(res, task);
    return ::grpc::Status::OK;
}

::grpc::Status
PendingImpl::Add(::grpc::ServerContext *ctx,
                 const ::stq::AddTaskReq *req,
                 ::stq::ListTaskRes *res)
{
    UNUSED(ctx);

    std::shared_ptr<STQQueue> queue = Global::queueList.getQueue(req->queuename());
    if (queue == nullptr)
    {
        return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                              "No such queue: " + req->queuename());
    }

    STQTask task;
    task.workDir = req->workdir();
    task.execName = req->programname();
    auto size = req->args().size();
    auto args = req->args();
    task.args.reserve(size);

    for (auto i = 0; i < size; ++i)
    {
        task.args.push_back(args.at(i));
    }

    task.priority = static_cast<STQPriority>(req->priority());
    if (queue->addTask(&task))
    {
        return ::grpc::Status(::grpc::StatusCode::UNAVAILABLE,
                              "Fail to add task.");
    }

    res->set_id(task.id);
    return ::grpc::Status::OK;
}

::grpc::Status
PendingImpl::Remove(::grpc::ServerContext *ctx,
                    const ::stq::TaskDetailsReq *req,
                    ::stq::Empty *res)
{
    UNUSED(ctx);
    UNUSED(res);

    std::shared_ptr<STQQueue> queue = Global::queueList.getQueue(req->queuename());
    if (queue == nullptr)
    {
        return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                              "No such queue: " + req->queuename());
    }

    if (queue->removeTask(req->id()))
    {
        return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                              "Fail to remove task.");
    }

    return ::grpc::Status::OK;
}

::grpc::Status
PendingImpl::Start(::grpc::ServerContext *ctx,
                   const ::stq::QueueReq *req,
                   ::stq::Empty *res)
{
    UNUSED(ctx);
    UNUSED(res);
    std::shared_ptr<STQQueue> queue = Global::queueList.getQueue(req->name());
    if (queue == nullptr)
    {
        return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                              "No such queue: " + req->name());
    }

    queue->start();
    return ::grpc::Status::OK;
}

::grpc::Status
PendingImpl::Stop(::grpc::ServerContext *ctx,
                  const ::stq::QueueReq *req,
                  ::stq::Empty *res)
{
    UNUSED(ctx);
    UNUSED(res);

    std::shared_ptr<STQQueue> queue = Global::queueList.getQueue(req->name());
    if (queue == nullptr)
    {
        return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                              "No such queue: " + req->name());
    }

    queue->stop();
    return ::grpc::Status::OK;
}
