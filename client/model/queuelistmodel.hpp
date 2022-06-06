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

#pragma once

#include <vector>

#include "QJSValue"
#include "QThread"

#include "queue.grpc.pb.h"

class QueueListModel : public QThread
{
    Q_OBJECT

public:

    QueueListModel();

    ~QueueListModel();

    Q_INVOKABLE bool init();

    Q_INVOKABLE bool hasError();

    Q_INVOKABLE QString lastError();

    Q_INVOKABLE QJSValue result();

    Q_INVOKABLE void startCreate(QString);

    Q_INVOKABLE void startRename(QString, QString);

    Q_INVOKABLE void startDelete(QString);

    Q_INVOKABLE void startList();

    void run() override;

signals:

    void done();

private:

    bool m_isInit;

    QQmlApplicationEngine *m_engine;

    bool m_hasError;

    QString m_lastError;

    QString m_oldName;

    QString m_name;

    QJSValue m_res;

    typedef enum funcs
    {
        Create, Rename, Delete, List
    } funcs;

    funcs m_func;

    std::unique_ptr<stq::Queue::Stub> m_stub;

    typedef void (QueueListModel::*Handler)();

    void createImpl();

    void renameImpl();

    void deleteImpl();

    void listImpl();

    Handler m_handler[4] =
    {
        &QueueListModel::createImpl,
        &QueueListModel::renameImpl,
        &QueueListModel::deleteImpl,
        &QueueListModel::listImpl
    };

    void reset();

    void buildRes(std::vector<std::string> &);
};
