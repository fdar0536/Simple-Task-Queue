/*
 * Simple Task Queue
 * Copyright (c) 2025-present fdar0536
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

#include "QObject"
#include "QQmlApplicationEngine"
#include "QVariant"

#include "controller/global/defines.hpp"

namespace Controller
{

namespace GUI
{

class Global : public QObject
{

    Q_OBJECT

    Q_DISABLE_COPY(Global)

public:

    static Global *instance();

    static void destroy();

    QQmlApplicationEngine *engine();

    // note that the host info has following field:
    // 1. name
    // 2. target
    // 3. port
    // 4. it is an embedded one or not
    QList<QSharedPointer<QJSValue>> hostList;

    QSharedPointer<QJSValue> lastHost;

private:

    explicit Global(QObject *parent = nullptr);

    ~Global();

    QQmlApplicationEngine m_engine;

    void parseConfig();

    void saveConfig();

    void getDefaultConfig();

    u8 getConfigPath(QString &);

}; // class global

} // namespace GUI

} // namespace Controller

#endif // _CONTROLLER_GUI_GLOBAL_HPP_
