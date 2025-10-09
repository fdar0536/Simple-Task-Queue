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

#ifndef _VIEW_MAIN_HPP_
#define _VIEW_MAIN_HPP_

#include "QAction"
#include "QSystemTrayIcon"
#include "QObject"

#include "controller/global/defines.hpp"

namespace View
{

class Main : public QObject
{

    Q_OBJECT

public:

    explicit Main(QObject *parent = nullptr);

    ~Main();

    Q_INVOKABLE bool init();

    Q_INVOKABLE void exit();

signals:

    void show();

public slots:

    void exitProcess(bool);

private slots:

    void onTrayIconActivated(QSystemTrayIcon::ActivationReason);

private:

    // tray icon
    QSystemTrayIcon *m_trayIcon;

    QMenu *m_menu;

    QAction *m_showAction;

    QAction *m_exitAction;

    u8 setupTrayIcon();

    void connectHook();

    void cleanMemory();

}; // class main

} // namespace View

#endif // _VIEW_MAIN_HPP_
