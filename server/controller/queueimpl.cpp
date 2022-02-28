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
                           ::stq::ExitRes* res)
{
    UNUSED(context);
    if (req->name() == "")
    {
        res->set_statecode(::stq::StateCode::FAILED);
        res->set_reason("Name is empty");
        goto exit;
    }

    if (Global::queueList.createQueue(req->name()))
    {
        res->set_statecode(::stq::StateCode::FAILED);
        res->set_reason("");
        goto exit;
    }

    res->set_statecode(::stq::StateCode::OK);
    res->set_reason("");

exit:
    return ::grpc::Status::OK;
}

::grpc::Status QueueImpl::RenameQueue(::grpc::ServerContext* context,
                           const ::stq::RenameQueueReq* req,
                           ::stq::ExitRes* res)
{
    UNUSED(context);
    if (req->oldname() == "" || req->newname() == "")
    {
        res->set_statecode(::stq::StateCode::FAILED);
        res->set_reason("Name is empty");
        goto exit;
    }

    if (Global::queueList.renameQueue(req->oldname(), req->newname()))
    {
        res->set_statecode(::stq::StateCode::FAILED);
        res->set_reason("");
        goto exit;
    }

    res->set_statecode(::stq::StateCode::OK);
    res->set_reason("");

exit:
    return ::grpc::Status::OK;
}

::grpc::Status QueueImpl::DeleteQueue(::grpc::ServerContext* context,
                           const ::stq::QueueReq* req,
                           ::stq::ExitRes* res)
{
    UNUSED(context);
    if (req->name() == "")
    {
        res->set_statecode(::stq::StateCode::FAILED);
        res->set_reason("Name is empty");
        goto exit;
    }

    if (Global::queueList.deleteQueue(req->name()))
    {
        res->set_statecode(::stq::StateCode::FAILED);
        res->set_reason("");
        goto exit;
    }

    res->set_statecode(::stq::StateCode::OK);
    res->set_reason("");

exit:
    return ::grpc::Status::OK;
}

::grpc::Status QueueImpl::ListQueue(::grpc::ServerContext* context,
                         const ::stq::Inquiry* req,
                         ::stq::ListQueueRes* res)
{
    UNUSED(context);
    UNUSED(req);

    auto list = Global::queueList.listQueue();
    if (list.empty())
    {
        res->set_statecode(::stq::StateCode::FAILED);
        goto exit;
    }

    for (auto it = list.begin(); it != list.end(); ++it)
    {
        res->add_list(*it);
    }

    res->set_statecode(::stq::StateCode::OK);
exit:
    return ::grpc::Status::OK;
}
