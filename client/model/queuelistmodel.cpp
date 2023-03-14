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

#include "queuelistmodel.hpp"

// public member function
QueueListModel *QueueListModel::create(QObject *parent)
{
    std::shared_ptr<Global> global = Global::instance();
    if (global == nullptr)
    {
        return nullptr;
    }

    QueueListModel *ret(nullptr);
    try
    {
        ret = new QueueListModel(parent);
    }
    catch (...)
    {
        return nullptr;
    }

    try
    {
        ret->m_stub = stq::Queue::NewStub(global->grpcChannel());
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

    ret->m_res.reserve(100);
    return ret;
}

QueueListModel::~QueueListModel()
{}

uint_fast8_t QueueListModel::hasError(bool &out)
{
    if (m_isRunning.load(std::memory_order_relaxed)) return 1;
    out = m_hasError;
    return 0;
}

uint_fast8_t QueueListModel::lastError(QString &out)
{
    if (m_isRunning.load(std::memory_order_relaxed)) return 1;
    out = m_lastError;
    return 0;
}

uint_fast8_t QueueListModel::result(QStringList &out)
{
    if (m_isRunning.load(std::memory_order_relaxed)) return 1;
    out = m_res;
    return 0;
}

uint_fast8_t QueueListModel::startCreate(const QString &name)
{
    if (m_isRunning.load(std::memory_order_relaxed)) return 1;
    m_name = name;
    reset();
    m_func = Create;
    start();
    return 0;
}

uint_fast8_t QueueListModel::startRename(const QString &oldName, const QString &newName)
{
    if (m_isRunning.load(std::memory_order_relaxed)) return 1;
    m_oldName = oldName;
    m_name = newName;
    reset();
    m_func = Rename;
    start();

    return 0;
}

uint_fast8_t QueueListModel::startDelete(const QString &name)
{
    if (m_isRunning.load(std::memory_order_relaxed)) return 1;
    m_name = name;
    reset();
    m_func = Delete;
    start();
    return 0;
}

uint_fast8_t QueueListModel::startList()
{
    if (m_isRunning.load(std::memory_order_relaxed)) return 1;
    reset();
    m_func = List;
    start();
    return 0;
}

void QueueListModel::run()
{
    m_isRunning.store(true, std::memory_order_relaxed);
    (this->*m_handler[m_func])();
    m_isRunning.store(false, std::memory_order_relaxed);
}

// private member functions
QueueListModel::QueueListModel(QObject *parent) :
    QThread(parent),
    m_hasError(false),
    m_lastError(""),
    m_oldName(""),
    m_name(""),
    m_func(Create),
    m_stub(nullptr)
{
    m_isRunning.store(false, std::memory_order_relaxed);
}

void QueueListModel::createImpl()
{
    stq::QueueReq req;
    req.set_name(m_name.toLocal8Bit().toStdString());

    stq::Empty res;
    grpc::ClientContext ctx;

    GrpcCommon::setupCtx(ctx);
    grpc::Status status = m_stub->CreateQueue(&ctx, req, &res);
    if (status.ok())
    {
        listImpl();
        return;
    }

    m_hasError = true;
    GrpcCommon::buildErrMsg(status, m_lastError);
    emit done();
}

void QueueListModel::renameImpl()
{
    stq::RenameQueueReq req;
    req.set_oldname(m_oldName.toLocal8Bit().toStdString());
    req.set_newname(m_name.toLocal8Bit().toStdString());

    stq::Empty res;
    grpc::ClientContext ctx;

    GrpcCommon::setupCtx(ctx);
    grpc::Status status = m_stub->RenameQueue(&ctx, req, &res);
    if (status.ok())
    {
        listImpl();
        return;
    }

    m_hasError = true;
    GrpcCommon::buildErrMsg(status, m_lastError);
    emit done();
}

void QueueListModel::deleteImpl()
{
    stq::QueueReq req;
    req.set_name(m_name.toLocal8Bit().toStdString());

    stq::Empty res;
    grpc::ClientContext ctx;

    GrpcCommon::setupCtx(ctx);
    grpc::Status status = m_stub->DeleteQueue(&ctx, req, &res);
    if (status.ok())
    {
        listImpl();
        return;
    }

    m_hasError = true;
    GrpcCommon::buildErrMsg(status, m_lastError);
    emit done();
}

void QueueListModel::listImpl()
{
    std::vector<std::string> list;
    list.reserve(100);

    stq::Empty req;
    stq::ListQueueRes res;
    grpc::ClientContext ctx;

    auto reader = m_stub->ListQueue(&ctx, req);
    if (reader == nullptr)
    {
        m_lastError = QString("%1:%2 reader is nullptr.").arg(__FILE__).arg(__LINE__);
        m_hasError = true;
        m_isRunning.store(false, std::memory_order_relaxed);
        emit done();
        return;
    }

    while (reader->Read(&res))
    {
        m_res.push_back(QString::fromStdString(res.name()));
    }

    grpc::Status status = reader->Finish();
    if (status.ok())
    {
        emit done();
        return;
    }

    m_hasError = true;
    m_res.clear();
    GrpcCommon::buildErrMsg(status, m_lastError);
    emit done();
}

void QueueListModel::reset()
{
    m_lastError = "";
    m_hasError = false;
    m_res.clear();
}
