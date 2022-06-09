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
#include "../global.hpp"

#include "queueimpl.hpp"

::grpc::Status QueueImpl::CreateQueue(::grpc::ServerContext* context,
                           const ::stq::QueueReq* req,
                           ::stq::Empty* res)
{
    UNUSED(context);
    UNUSED(res);
    if (req->name() == "")
    {
        return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                              "Input is empty string");
    }

    if (Global::queueList.createQueue(req->name()))
    {
        return ::grpc::Status(::grpc::StatusCode::ALREADY_EXISTS,
                              req->name() + " is already exist");
    }

    return ::grpc::Status::OK;
}

::grpc::Status QueueImpl::RenameQueue(::grpc::ServerContext* context,
                           const ::stq::RenameQueueReq* req,
                           ::stq::Empty* res)
{
    UNUSED(context);
    UNUSED(res);
    if (req->oldname() == "" || req->newname() == "")
    {
        return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                              "oldName or newName is empty string");
    }

    if (Global::queueList.renameQueue(req->oldname(), req->newname()))
    {
        return ::grpc::Status(::grpc::StatusCode::NOT_FOUND,
                              req->oldname() + " is not exist.");
    }

    return ::grpc::Status::OK;
}

::grpc::Status QueueImpl::DeleteQueue(::grpc::ServerContext* context,
                           const ::stq::QueueReq* req,
                           ::stq::Empty* res)
{
    UNUSED(context);
    UNUSED(res);
    if (req->name() == "")
    {
        return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                              "Input is empty string");
    }

    if (Global::queueList.deleteQueue(req->name()))
    {
        return ::grpc::Status(::grpc::StatusCode::NOT_FOUND,
                              req->name() + " is not exist.");
    }

    return ::grpc::Status::OK;
}

::grpc::Status QueueImpl::ListQueue(::grpc::ServerContext *ctx,
                         const ::stq::Empty *req,
                         ::grpc::ServerWriter<::stq::ListQueueRes> *res)
{
    UNUSED(ctx);
    UNUSED(req);
    char buf[2048];
    switch (Global::queueList.listQueue(res, buf))
    {
    case 0:
    {
        return ::grpc::Status::OK;
    }
    case 1:
    {
        return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                              buf);
    }
    case 2:
    {
        return ::grpc::Status(::grpc::StatusCode::NOT_FOUND,
                              buf);
    }
    default:
    {
        return ::grpc::Status(::grpc::StatusCode::ABORTED,
                              buf);
    }
    } // end switch
}
