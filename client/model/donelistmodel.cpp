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

#include "global.hpp"
#include "grpccommon.hpp"

#include "donelistmodel.hpp"

// public member functions
DoneListModel *DoneListModel::create(QObject *parent)
{
    std::shared_ptr<Global> global = Global::instance();
    if (global == nullptr)
    {
        return nullptr;
    }

    DoneListModel *ret(nullptr);

    try
    {
        ret = new DoneListModel(parent);
    }
    catch (...)
    {
        return nullptr;
    }

    try
    {
        ret->m_stub = stq::Done::NewStub(global->grpcChannel());
        if (ret->m_stub == nullptr)
        {
            delete ret;
            return nullptr;
        }
    }
    catch (...)
    {
        delete ret;
        return nullptr;
    }

    if (GrpcCommon::getQueueName(global, ret->m_queueName))
    {
        delete ret;
        return nullptr;
    }

    return ret;
}

uint_fast8_t DoneListModel::lastError(QString &out)
{
    if (m_isRunning.load(std::memory_order_relaxed)) return 1;
    out = m_lastError;
    return 0;
}

uint_fast8_t DoneListModel::startList()
{
    if (m_isRunning.load(std::memory_order_relaxed)) return 1;

    m_func = List;
    start();
    return 0;
}

uint_fast8_t DoneListModel::doneList(std::vector<uint32_t> &out)
{
    if (m_isRunning.load(std::memory_order_relaxed)) return 1;

    out = m_doneList;
    return 0;
}

uint_fast8_t DoneListModel::startDetails(uint32_t in)
{
    if (m_isRunning.load(std::memory_order_relaxed)) return 1;

    m_reqID = in;
    m_func = Details;
    start();
    return 0;
}

uint_fast8_t DoneListModel::taskDetails(TaskDetails &out)
{
    if (m_isRunning.load(std::memory_order_relaxed)) return 1;

    out = m_taskDetailsRes;
    return 0;
}

uint_fast8_t DoneListModel::startClear()
{
    if (m_isRunning.load(std::memory_order_relaxed)) return 1;

    m_func = Clear;
    start();
    return 0;
}

void DoneListModel::run()
{
    m_isRunning.store(true, std::memory_order_relaxed);
    reset();
    (this->*m_handler[m_func])();
}

// private member functions
DoneListModel::DoneListModel(QObject *parent) :
    QThread(parent),
    m_lastError(""),
    m_reqID(0)
{
    m_isRunning.store(false, std::memory_order_relaxed);
    m_doneList.reserve(64);
}

void DoneListModel::listImpl()
{
    stq::QueueReq req;
    req.set_name(m_queueName.toLocal8Bit().toStdString());

    grpc::ClientContext ctx;
    stq::ListTaskRes res;

    GrpcCommon::setupCtx(ctx);
    auto reader = m_stub->List(&ctx, req);
    if (reader == nullptr)
    {
        m_lastError = QString("%1:%2 reader is nullptr.").arg(__FILE__).arg(__LINE__);
        m_isRunning.store(false, std::memory_order_relaxed);
        emit errorOccurred();
        return;
    }

    while(reader->Read(&res))
    {
        m_doneList.push_back(res.id());
    }

    grpc::Status status = reader->Finish();
    if (!status.ok())
    {
        m_doneList.clear();
    }

    m_isRunning.store(false, std::memory_order_relaxed);
    emit listDone();
}

void DoneListModel::detailsImpl()
{
    stq::TaskDetailsReq req;
    req.set_queuename(m_queueName.toLocal8Bit().toStdString());
    req.set_id(m_reqID);

    grpc::ClientContext ctx;
    stq::TaskDetailsRes res;

    GrpcCommon::setupCtx(ctx);
    grpc::Status status = m_stub->Details(&ctx, req, &res);
    if (status.ok())
    {
        GrpcCommon::buildTaskDetails(res, m_taskDetailsRes);
        m_isRunning.store(false, std::memory_order_relaxed);
        emit detailsDone();
        return;
    }

    GrpcCommon::buildErrMsg(status, m_lastError);
    m_isRunning.store(false, std::memory_order_relaxed);
    emit errorOccurred();
}

void DoneListModel::clearImpl()
{
    stq::QueueReq req;
    req.set_name(m_queueName.toLocal8Bit().toStdString());

    grpc::ClientContext ctx;
    stq::Empty res;

    GrpcCommon::setupCtx(ctx);
    grpc::Status status = m_stub->Clear(&ctx, req, &res);
    if (status.ok())
    {
        listImpl();
        return;
    }

    GrpcCommon::buildErrMsg(status, m_lastError);
    m_isRunning.store(false, std::memory_order_relaxed);
    emit errorOccurred();
}

void DoneListModel::reset()
{
    m_lastError = "";
    m_doneList.clear();

    // reset task details
    m_taskDetailsRes.workDir = "";
    m_taskDetailsRes.programName = "";
    m_taskDetailsRes.args.clear();
    m_taskDetailsRes.exitCode = 0;
    m_taskDetailsRes.priority = 2;
}
