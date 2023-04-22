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

#ifndef _CONTROLLER_GUI_MAIN_HPP_
#define _CONTROLLER_GUI_MAIN_HPP_

#include "config.h"

#include "spdlog/spdlog.h"
#include "QObject"

#ifndef STQ_MOBILE
#include "QWidget"
#include "QSystemTrayIcon"
#endif

namespace Controller
{

namespace GUI
{

class Main : public QObject
{
    Q_OBJECT

public:

    Main(QObject * = nullptr);

    ~Main();

    Q_INVOKABLE bool init();

signals:

    void Show();

    void Exit();

public slots:

    void AboutQt();

private slots:
#ifndef STQ_MOBILE
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
#endif

private:

    std::shared_ptr<spdlog::logger> m_defaultLogger;

#ifndef STQ_MOBILE
    QSystemTrayIcon *m_icon;

    QMenu *m_iconContextMenu;

    QAction *m_showAction;

    QAction *m_exitAction;

    uint_fast8_t trayIconInit();

    void sqliteInit();
#endif

    void spdlogInit();
}; // end class Main

} // namespace GUI

} // end namespace Controller

#endif // _CONTROLLER_GUI_MAIN_HPP_
