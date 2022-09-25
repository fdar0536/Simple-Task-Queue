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

#include <mutex>

#include "grpcpp/grpcpp.h"
#include "QObject"
#include "QHash"
#include "QRegularExpressionValidator"

#include "controller/taskdetailsdialog.hpp"

class Global : public QObject
{
    Q_OBJECT

public:

    Global();

    ~Global();

    static std::shared_ptr<Global> instance();

    uint8_t state(const QString &, QHash<QString, QVariant> &);

    void setState(const QString &, QHash<QString, QVariant> &);

    QHash<QString, QVariant> settings();

    void saveSettings(QHash<QString, QVariant> &);

    bool isSettingsNotAccepted();

    std::shared_ptr<grpc::ChannelInterface> grpcChannel();

    void setGrpcChannel(std::shared_ptr<grpc::ChannelInterface> &);

    QRegularExpressionValidator *ipRegex() const;

    uint8_t taskDetailsDialog(TaskDetailsDialog **);

    void freeTaskDetailsDialog();

private:

    static std::shared_ptr<Global> m_instance;

    QHash<QString, QHash<QString, QVariant>> m_stateStore;

    std::mutex m_stateMutex;

    std::shared_ptr<grpc::ChannelInterface> m_channel;

    std::mutex m_channelMutex;

    QRegularExpressionValidator *m_ipRegex;

    TaskDetailsDialog *m_taskDetailsDialog;

    std::atomic<bool> m_taskDetailsDialogAvailable;

    uint8_t initConfigFile();
};
