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

#ifndef _VIEW_MAINWINDOW_HPP_
#define _VIEW_MAINWINDOW_HPP_

#include "QIcon"
#include "QMainWindow"
#include "QSystemTrayIcon"

#include "controller/global/defines.hpp"

// ui
#include "configform.hpp"

namespace Ui
{

class MainWindow;

}

namespace View
{

class MainWindow : public QMainWindow
{

    Q_OBJECT

public:

    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

    u8 init();

protected:

    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:

    // tool bar
    void onActionConfigTriggered(bool);

    // tray icon
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason);

    void onShowActionTriggered(bool);

    // others
    void exitProcess(bool);

private:

    Ui::MainWindow *m_ui;

    // icon
    QIcon m_icon;

    // forms
    ConfigForm *m_configForm;

    // tray icon
    QSystemTrayIcon *m_trayIcon;

    QMenu *m_menu;

    QAction *m_showAction;

    QAction *m_exitAction;

    u8 setupTrayIcon();

    void connectHook();

    void cleanMemory();

}; // class MainWindow

} // namespace View

#endif // _VIEW_MAINWINDOW_HPP_
