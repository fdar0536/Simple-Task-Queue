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

#include <atomic>

#include "QThread"

#include "taskdetails.hpp"

#include "done.grpc.pb.h"

class DoneListModel : public QThread
{
    Q_OBJECT

public:

    static DoneListModel *create(QObject * = nullptr);

    uint_fast8_t lastError(QString &);

    uint_fast8_t startList();

    uint_fast8_t doneList(std::vector<uint32_t> &);

    uint_fast8_t startDetails(uint32_t);

    uint_fast8_t taskDetails(TaskDetails &);

    uint_fast8_t startClear();

    void run() override;

signals:

    void errorOccurred();

    void listDone();

    void detailsDone();

private:

    DoneListModel(QObject * = nullptr);

    std::atomic<bool> m_isRunning;

    QString m_queueName;

    std::unique_ptr<stq::Done::Stub> m_stub;

    QString m_lastError;

    std::vector<uint32_t> m_doneList;

    uint32_t m_reqID;

    TaskDetails m_taskDetailsRes;

    typedef enum funcs
    {
        List, Details, Clear
    } funcs;

    funcs m_func;

    typedef void (DoneListModel::*Handler)();

    void listImpl();

    void detailsImpl();

    void clearImpl();

    Handler m_handler[3] =
    {
        &DoneListModel::listImpl,
        &DoneListModel::detailsImpl,
        &DoneListModel::clearImpl
    };

    void reset();

};

