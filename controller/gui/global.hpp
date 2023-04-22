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

    Q_PROPERTY(bool isLocalAvailable READ isLocalAvailable CONSTANT)

public:

    typedef enum BackendMode
    {
        GRPC, SQLITE
    } BackendMode;

    Global(QObject * = nullptr);

    ~Global();

    uint_fast8_t init();

    bool isNotMobile() const;

    bool isLocalAvailable() const;

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

    Q_INVOKABLE void notifyClosing();

    Q_INVOKABLE void notifyAllCleaned();

#ifndef STQ_MOBILE
    Q_INVOKABLE void saveFile(const QString &, const QString &);
#endif

    Q_INVOKABLE QJSValue getLog();

    void onSpdlogLog(const QString &);

signals:

    void WindowClosing();

    void AllCleaned();

private:

    std::atomic<bool> m_isInit;

    bool m_isNotMobile;

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

    QList<QString> m_logBuf;

    std::mutex m_logMutex;

}; // and class Global

} // namespace GUI

} // end namespace Controller

#endif // _CONTROLLER_GUI_GLOBAL_HPP_
