/*
 * Simple Task Queue
 * Copyright (c) 2023-2024 fdar0536
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

#include "controller/global/defines.hpp"
#include "model/errmsg.hpp"
#include "init.hpp"

#include "queuelistimpl.hpp"

namespace Controller
{

namespace GRPCServer
{

grpc::Status
QueueListImpl::Create(grpc::ServerContext *ctx,
                      const stq::QueueReq *req,
                      stq::Empty *res)
{
    UNUSED(ctx);
    UNUSED(res);

    if (!req)
    {
        spdlog::critical("{}:{} invalid input", __FILE__, __LINE__);
        return grpc::Status(grpc::StatusCode::INTERNAL,
                            "Internal server error");
    }

    if (req->name().empty())
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT,
                            "\"name\" is empty string");
    }

    u8 code = sqliteQueueList->createQueue(req->name());
    if (code)
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return Model::ErrMsg::toGRPCStatus(code, "Fail to create queue");
    }

    return grpc::Status::OK;
}

grpc::Status
QueueListImpl::Rename(grpc::ServerContext *ctx,
                      const stq::RenameQueueReq *req,
                      stq::Empty *res)
{
    UNUSED(ctx);
    UNUSED(res);

    if (!req)
    {
        spdlog::critical("{}:{} invalid input", __FILE__, __LINE__);
        return grpc::Status(grpc::StatusCode::INTERNAL,
                            "Internal server error");
    }

    if (req->oldname().empty() || req->newname().empty())
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT,
                            "\"oldName\" or \"newName\" is empty string");
    }

    u8 code = sqliteQueueList->renameQueue(req->oldname(), req->newname());
    if (code)
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return Model::ErrMsg::toGRPCStatus(code, "Fail to rename");
    }

    return grpc::Status::OK;
}

grpc::Status
QueueListImpl::Delete(grpc::ServerContext *ctx,
                      const stq::QueueReq *req,
                      stq::Empty *res)
{
    UNUSED(ctx);
    UNUSED(res);

    if (!req)
    {
        spdlog::critical("{}:{} invalid input", __FILE__, __LINE__);
        return grpc::Status(grpc::StatusCode::INTERNAL,
                            "Internal server error");
    }

    if (req->name().empty())
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT,
                            "\"name\" is empty string");
    }

    u8 code = sqliteQueueList->deleteQueue(req->name());
    if (code)
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return Model::ErrMsg::toGRPCStatus(code, "Fail to delete");
    }

    return grpc::Status::OK;
}

grpc::Status
QueueListImpl::List(grpc::ServerContext *ctx,
                    const stq::Empty *req,
                    grpc::ServerWriter<::stq::ListQueueRes> *writer)
{
    UNUSED(ctx);
    UNUSED(req);

    if (!writer)
    {
        spdlog::critical("{}:{} invalid input", __FILE__, __LINE__);
        return grpc::Status(grpc::StatusCode::INTERNAL,
                            "Internal server error");
    }

    std::vector<std::string> out;
    u8 code = sqliteQueueList->listQueue(out);
    if (code)
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return Model::ErrMsg::toGRPCStatus(code, "Fail to list queue");
    }

    stq::ListQueueRes toWrite;
    for (size_t i = 0; i < out.size(); ++i)
    {
        toWrite.set_name(out.at(i));
        writer->Write(toWrite);
    }

    return grpc::Status::OK;
}

grpc::Status
QueueListImpl::GetQueue(grpc::ServerContext *ctx,
                        const stq::QueueReq *req,
                        stq::Empty *res)
{
    UNUSED(ctx);
    UNUSED(res);
    if (!req)
    {
        spdlog::critical("{}:{} invalid input", __FILE__, __LINE__);
        return grpc::Status(grpc::StatusCode::INTERNAL,
                            "Internal server error");
    }

    auto queue = sqliteQueueList->getQueue(req->name());
    if (queue == nullptr)
    {
        return grpc::Status(grpc::StatusCode::NOT_FOUND,
                            "No such queue");
    }

    return grpc::Status::OK;
}

} // end namespace GRPCServer

} // end namespace Controller
