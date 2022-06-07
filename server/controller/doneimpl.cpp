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

#include "common.hpp"
#include "global.hpp"
#include "implcommon.hpp"
#include "doneimpl.hpp"

::grpc::Status
DoneImpl::List(::grpc::ServerContext *ctx,
               const ::stq::QueueReq *req,
               ::stq::ListTaskRes *res)
{
    UNUSED(ctx);

    std::shared_ptr<STQQueue> queue = Global::queueList.getQueue(req->name());
    if (queue == nullptr)
    {
        return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                              "No such queue: " + req->name());
    }

    queue->listFinished(res);
    return ::grpc::Status::OK;
}

::grpc::Status
DoneImpl::Details(::grpc::ServerContext *ctx,
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
    if (queue->finishedDetails(req->id(), &task))
    {
        return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                              "No such task: " + std::to_string(req->id()));
    }

    ImplCommon::buildTaskRes(res, task);
    return ::grpc::Status::OK;
}

::grpc::Status
DoneImpl::Clear(::grpc::ServerContext *ctx,
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

    queue->clearFinished();
    return ::grpc::Status::OK;
}
