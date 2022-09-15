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

#include "grpccommon.hpp"

#include "config.h"

namespace GrpcCommon
{

void setupCtx(grpc::ClientContext &ctx)
{
    ctx.set_deadline(std::chrono::system_clock::now() +
    std::chrono::milliseconds(STQ_CLIENT_TIMEOUT * 1000));
}

void buildErrMsg(grpc::Status &status, QString &reason)
{
    reason = QString::number(static_cast<int>(status.error_code()));
    reason += ": ";
    reason += QString::fromStdString(status.error_message());
}

uint8_t getQueueName(std::shared_ptr<Global> &global, QString &output)
{
    if (global == nullptr) return 1;

    QHash<QString, QVariant> listState;
    if (global->state("queueListState", listState))
    {
        return 1;
    }

    QStringList queueList(listState["list"].toStringList());
    if (!queueList.size())
    {
        return 1;
    }

    int listIndex(listState["index"].toInt());
    if (listIndex < 0 || listIndex >= queueList.size())
    {
        return 1;
    }

    output = queueList[listIndex];
    if (output.isEmpty()) return 1;

    return 0;
}

} // end namespace GrpcCommon
