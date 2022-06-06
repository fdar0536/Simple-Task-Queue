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
QueueListModel::QueueListModel() :
    QThread(),
    m_isInit(false),
    m_engine(nullptr),
    m_hasError(false),
    m_lastError(""),
    m_oldName(""),
    m_name(""),
    m_res(QJSValue()),
    m_func(Create),
    m_stub(nullptr)
{}

QueueListModel::~QueueListModel()
{}

bool QueueListModel::init()
{
    if (m_isInit)
    {
        return false;
    }

    Global *global = Global::instance();
    if (!global)
    {
        return true;
    }

    m_engine = global->engine();
    if (!m_engine)
    {
        return true;
    }

    try
    {
        m_stub = stq::Queue::NewStub(global->grpcChannel());
        if (m_stub == nullptr)
        {
            return true;
        }
    }
    catch (...)
    {
        return true;
    }

    m_isInit = true;
    return false;
}

bool QueueListModel::hasError()
{
    return m_hasError;
}

QString QueueListModel::lastError()
{
    return m_lastError;
}

QJSValue QueueListModel::result()
{
    return m_res;
}

void QueueListModel::startCreate(QString name)
{
    m_name = name;
    reset();
    m_func = Create;
    start();
}

void QueueListModel::startRename(QString oldName, QString newName)
{
    m_oldName = oldName;
    m_name = newName;
    reset();
    m_func = Rename;
    start();
}

void QueueListModel::startDelete(QString name)
{
    m_name = name;
    reset();
    m_func = Delete;
    start();
}

void QueueListModel::startList()
{
    reset();
    m_func = List;
    start();
}

void QueueListModel::run()
{
    (this->*m_handler[m_func])();
}

// private member functions
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

    int startIndex(0);

    while (1)
    {
        stq::ListQueueReq req;
        req.set_startindex(startIndex);
        req.set_limit(10);

        stq::ListQueueRes res;
        grpc::ClientContext ctx;

        GrpcCommon::setupCtx(ctx);
        grpc::Status status = m_stub->ListQueue(&ctx, req, &res);
        if (status.ok())
        {
            auto data = res.list();
            for (auto i = 0; i < data.size(); ++i)
            {
                list.push_back(data.at(i));
            }

            if (data.size() < 10)
            {
                buildRes(list);
                emit done();
                return;
            }

            startIndex += 10;
            continue;
        }

        m_hasError = true;
        GrpcCommon::buildErrMsg(status, m_lastError);
        buildRes(list);
        emit done();
        break;
    }
}

void QueueListModel::reset()
{
    m_lastError = "";
    m_hasError = false;
    m_res = m_engine->newArray(0);
}

void QueueListModel::buildRes(std::vector<std::string> &in)
{
    if (in.empty()) return;

    size_t size = in.size();
    m_res = m_engine->newArray(size);
    for (size_t i = 0; i < size; ++i)
    {
        m_res.setProperty(i, QJSValue(QString::fromStdString(in.at(i))));
    }
}
