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
#include "consoleimpl.hpp"

::grpc::Status
ConsoleImpl::Output
(
    ::grpc::ServerContext *ctx,
    ::grpc::ServerReaderWriter<::stq::Msg, ::stq::Msg> *stream
)
{
    UNUSED(ctx);
    std::shared_ptr<STQQueue> queue = nullptr;
    bool flag = false;
    ::stq::Msg msg;
    char buf[4096];
    while (stream->Read(&msg))
    {
        if (flag)
        {
            if (msg.msg() != "c")
            {
                break;
            }
        }
        else
        {
            flag = true;
            queue = Global::queueList.getQueue(msg.msg());
            if (queue == nullptr)
            {
                return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                              "No such queue: " + msg.msg());
            }
        }

        if (queue->readCurrentOutput(buf, 4096))
        {
            return ::grpc::Status(::grpc::StatusCode::INTERNAL,
                      "Queue is stopped or internal server error.");
        }

        msg.set_msg(buf);
        stream->Write(msg);
    }

    return ::grpc::Status::OK;
}
