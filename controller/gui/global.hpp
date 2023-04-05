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

#ifdef STQ_MOBILE
#include "QObject"
#else
#include "QWidget"
#endif

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

#ifdef STQ_MOBILE
class Global : public QObject
#else
class Global : public QWidget
#endif
{

    Q_OBJECT

    Q_PROPERTY(bool isnNotMobile READ isNotMobile CONSTANT)

public:

#ifndef STQ_MOBILE
    typedef enum BackendMode
    {
        GRPC, SQLITE
    } BackendMode;
#endif

    Global();

    ~Global();

    uint_fast8_t init();

    bool isNotMobile() const;

    void setEngine(QQmlApplicationEngine *engine);

    QQmlApplicationEngine *engine() const;

#ifndef STQ_MOBILE
    void setBackendMode(BackendMode);

    BackendMode backendMode() const;

    void
    setSqliteQueueList(std::shared_ptr<Model::DAO::IQueueList<Model::DAO::SQLiteToken>> &);

    std::shared_ptr<Model::DAO::IQueueList<Model::DAO::SQLiteToken>>
    sqliteQueueList() const;
#endif

signals:

    void InitDone(bool);

private:

    std::atomic<bool> m_isInit;

    bool m_isNotMobile;

    QSettings m_settings;

    QQmlApplicationEngine *m_engine;

#ifndef STQ_MOBILE
    BackendMode m_backendMode;

    std::shared_ptr<Model::DAO::IQueueList<Model::DAO::SQLiteToken>>
        m_sqliteQueueList;
#endif

}; // and class Global

} // namespace GUI

} // end namespace Controller

#endif // _CONTROLLER_GUI_GLOBAL_HPP_
