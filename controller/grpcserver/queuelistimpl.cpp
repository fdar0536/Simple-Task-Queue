/*
  *Simple Task Queue
  *Copyright(c) 2023 fdar0536
 *
  *Permission is hereby granted, free of charge, to any person obtaining a copy
  *of this software and associated documentation files(the "Software"), to deal
  *in the Software without restriction, including without limitation the rights
  *to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  *copies of the Software, and to permit persons to whom the Software is
  *furnished to do so, subject to the following conditions:
 *
  *The above copyright notice and this permission notice shall be included in all
  *copies or substantial portions of the Software.
 *
  *THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  *IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  *FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  *AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  *LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  *OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  *SOFTWARE.
 */

#include "spdlog/spdlog.h"

#include "controller/global/init.hpp"
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

    if (Global::sqliteQueueList->createQueue(req->name()))
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT,
                            "Please see server log for details.");
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

    if (Global::sqliteQueueList->renameQueue(req->oldname(), req->newname()))
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT,
                            "Please see server log for details.");
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

    if (Global::sqliteQueueList->deleteQueue(req->name()))
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT,
                            "Please see server log for details.");
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
    if (Global::sqliteQueueList->listQueue(out))
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT,
                            "Please see server log for details.");
    }

    /*
    if (req->name().empty())
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT,
                            "\"name\" is empty string");
    }

    if (Global::sqliteQueueList->deleteQueue(req->name()))
    {
        spdlog::debug("{}:{} trace", __FILE__, __LINE__);
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT,
                            "Please see server log for details.");
    }
    */

    return grpc::Status::OK;
}

/*


    grpc::Status List(grpc::ServerContext *ctx,
                      const stq::Empty *req,
                      grpc::ServerWriter<::stq::ListQueueRes> *writer) override;

    grpc::Status GetQueue(grpc::ServerContext *ctx,
                          const stq::QueueReq *req,
                          stq::Empty *res) override;

*/

} // end namespace GRPCServer

} // end namespace Controller
