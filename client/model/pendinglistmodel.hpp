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
#include <vector>

#include "QThread"

#include "pending.grpc.pb.h"

#include "taskdetails.hpp"

class PendingListModel : public QThread
{
    Q_OBJECT

public:

    static PendingListModel *create(QObject * = nullptr);

    uint8_t lastError(QString &);

    uint8_t startList();

    uint8_t pendingList(std::vector<uint32_t> &);

    uint8_t startDetails(uint32_t id);

    uint8_t taskDetails(TaskDetails &);

    uint8_t startCurrent();

    uint8_t startAdd(const std::string &workDir,
                     const std::string &programName,
                     const std::vector<std::string> &args,
                     const uint32_t priority);

    uint8_t resID(uint32_t &);

    uint8_t startRemove(std::vector<uint32_t> &);

    uint8_t startStart();

    uint8_t startStop();

    void run() override;

signals:

    void errorOccurred();

    void detailsDone();

    void listDone();

    void addDone();

private:

    PendingListModel(QObject *parent = nullptr);

    typedef enum funcs
    {
        List, Details, Current, Add, Remove, Start, Stop
    } funcs;

    funcs m_func;

    std::unique_ptr<stq::Pending::Stub> m_stub;

    typedef void (PendingListModel::*Handler)();

    void listImpl();

    void detailsImpl();

    void currentImpl();

    void addImpl();

    void removeImpl();

    void startImpl();

    void stopImpl();

    void startStopImpl(uint8_t);

    Handler m_handler[7] =
    {
        &PendingListModel::listImpl,
        &PendingListModel::detailsImpl,
        &PendingListModel::currentImpl,
        &PendingListModel::addImpl,
        &PendingListModel::removeImpl,
        &PendingListModel::startImpl,
        &PendingListModel::stopImpl
    };

    QString m_queueName;

    QString m_lastError;

    std::atomic<bool> m_isRunning;

    std::vector<uint32_t> m_pendingList;

    std::vector<uint32_t> m_toRemove;

    TaskDetails m_taskDetailsReq;

    TaskDetails m_taskDetailsRes;

    uint32_t m_reqID;

    uint32_t m_resID;

    void reset();
};

