/*
 * Simple Task Queue
 * Copyright (c) 2020 fdar0536
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

#include <map>

#include "QMainWindow"
#include "QSystemTrayIcon"
#include "QMenu"
#include "QAction"
#include "QCloseEvent"

#include "tasktab.hpp"
#include "inputdialog.hpp"

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    static MainWindow *create(QWidget *parent = nullptr);

    ~MainWindow();

private slots:

    void on_actionAbout_Qt_triggered();

    void programExit();

    // InputDialog
    void onInputAccepted();

    // TaskTab
    void on_taskTab_tabCloseRequested(int index);

    void onTabRenameRequried(TaskTab *, QString &);

    // tray icon
    void iconActivated(QSystemTrayIcon::ActivationReason);

    void iconAction1Triggered();

private:

    explicit MainWindow(QWidget *parent = nullptr) :
        QMainWindow(parent),
        m_ui(nullptr),
        m_inputDialog(nullptr),
        m_icon(nullptr),
        m_iconContextMenu(nullptr),
        m_iconAction1(nullptr),
        m_iconAction2(nullptr)
    {}

    MainWindow(const MainWindow &) = delete;

    MainWindow() = delete;

    MainWindow &operator=(MainWindow) = delete;

    Ui::MainWindow *m_ui;

    InputDialog *m_inputDialog;

    QSystemTrayIcon *m_icon;

    QMenu *m_iconContextMenu;

    QAction *m_iconAction1;

    QAction *m_iconAction2;

    void connectHook();

    void closeEvent(QCloseEvent *);
};
