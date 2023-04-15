/*
 * Simple Task Queue
 * Copyright (c) 2023 fdar0536
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

#ifndef _CONTROLLER_GUI_GLOBAL_HPP_
#define _CONTROLLER_GUI_GLOBAL_HPP_

#include "controller/global/init.hpp"

#include <atomic>

#include <cinttypes>

#include "QObject"

#include "QSettings"
#include "QQmlApplicationEngine"

#include "model/dao/iqueuelist.hpp"

#ifndef STQ_MOBILE
#include "model/dao/sqliteconnect.hpp"
#endif

namespace Controller
{

namespace GUI
{

class Global : public QObject
{

    Q_OBJECT

    Q_PROPERTY(bool isNotMobile READ isNotMobile CONSTANT)

public:

    typedef enum BackendMode
    {
        GRPC, SQLITE
    } BackendMode;

    Global(QObject * = nullptr);

    ~Global();

    uint_fast8_t init();

    Q_INVOKABLE bool isNotMobile() const;

    void setEngine(QQmlApplicationEngine *engine);

    QQmlApplicationEngine *engine() const;

    void setBackendMode(BackendMode);

    BackendMode backendMode() const;

    void
    setQueueList(BackendMode, std::shared_ptr<Model::DAO::IQueueList> &);

    std::shared_ptr<Model::DAO::IQueueList>
    queueList() const;

    void setQueue(BackendMode, std::shared_ptr<Model::DAO::IQueue> &);

    std::shared_ptr<Model::DAO::IQueue>
    queue() const;

    QSettings *settings();

    Q_INVOKABLE void setState(QString, QJSValue);

    Q_INVOKABLE QJSValue state(QString);

    Q_INVOKABLE void notifyClosing();

    Q_INVOKABLE void notifyAllCleaned();

signals:

    void WindowClosing();

    void AllCleaned();

private:

    std::atomic<bool> m_isInit;

    bool m_isNotMobile;

    QSettings *m_settings;

    QQmlApplicationEngine *m_engine;

#ifndef STQ_MOBILE
    BackendMode m_backendMode;

    std::shared_ptr<Model::DAO::IQueue>
        m_sqliteQueue;
#endif

    std::shared_ptr<Model::DAO::IQueueList>
        m_grpcQueueList;

    std::shared_ptr<Model::DAO::IQueue>
        m_grpcQueue;

    std::unordered_map<QString, QJSValue> m_state;

}; // and class Global

} // namespace GUI

} // end namespace Controller

#endif // _CONTROLLER_GUI_GLOBAL_HPP_
