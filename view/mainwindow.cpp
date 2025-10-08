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

#include "fmt/format.h"

#include "QCloseEvent"
#include "QDebug"
#include "QMenu"
#include "QMessageBox"

#include "mainwindow.hpp"
#include "ui_mainwindow.h"

namespace View
{

MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent),
    m_ui(nullptr),
    m_icon(QIcon("://original-icon.jpg")),
    // forms
    m_configForm(nullptr),
    // tray icon
    m_trayIcon(nullptr),
    m_menu(nullptr),
    m_showAction(nullptr),
    m_exitAction(nullptr)
{}

MainWindow::~MainWindow()
{
    cleanMemory();
}

u8 MainWindow::init()
{
    if (!m_ui) m_ui = new (std::nothrow) Ui::MainWindow;
    if (!m_ui)
    {
        qCritical(
            fmt::format("{}:{} Fail to allocate memory",
                        __FILE__, __LINE__).c_str());
        return 1;
    }

    m_ui->setupUi(this);

    m_configForm = new(std::nothrow) ConfigForm(this);
    if (m_configForm->init())
    {
        qCritical(
            fmt::format("{}:{} ConfigForm init failed",
                        __FILE__, __LINE__).c_str());
        cleanMemory();
        return 1;
    }

    setCentralWidget(m_configForm);

    if (setupTrayIcon())
    {
        qCritical(
            fmt::format("{}:{} Setup tray icon failed",
                        __FILE__, __LINE__).c_str());
        cleanMemory();
        return 1;
    }

    connectHook();
    installEventFilter(this);
    setWindowIcon(m_icon);
    setWindowTitle("Flex Flow GUI");
    return 0;
}

// protected member functions
bool MainWindow::eventFilter(QObject *obj, QEvent *ev)
{
    if (obj == this)
    {
        if (ev->type() == QEvent::Close)
        {
            QCloseEvent *closeEvent = reinterpret_cast<QCloseEvent *>(ev);
            closeEvent->ignore();
            hide();
            return true;
        }
    }

    return QObject::eventFilter(obj, ev);
}

// private slots
// tool bar
void MainWindow::onActionConfigTriggered(bool)
{
    setCentralWidget(m_configForm);
}

// tray icon
void MainWindow::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick)
    {
        show();
    }
}

void MainWindow::onShowActionTriggered(bool)
{
    show();
}

void MainWindow::exitProcess(bool)
{
    if (QMessageBox::question(this, tr("Exit"),
                                    tr("Are you sure to exit?"
                                       "All unfinished process in embadded mode"
                                       "will be killed"),
                                       QMessageBox::Yes |
                                       QMessageBox::Cancel |
                                       QMessageBox::No)
        != QMessageBox::Yes)
    {
        return;
    }

    qApp->exit(0);
}

// private member functions
u8 MainWindow::setupTrayIcon()
{
    // tool bar
    connect(m_ui->actionConfig,
            &QAction::triggered,
            this,
            &MainWindow::onActionConfigTriggered);

    if (QSystemTrayIcon::isSystemTrayAvailable())
    {
        m_trayIcon = new (std::nothrow) QSystemTrayIcon(this);
        if (!m_trayIcon)
        {
            qCritical(
                fmt::format("{}:{} Fail to allocate memory",
                            __FILE__, __LINE__).c_str());
            return 1;
        }

        m_menu = new (std::nothrow) QMenu(this);
        if (!m_menu)
        {
            qCritical(
                fmt::format("{}:{} Fail to allocate memory",
                            __FILE__, __LINE__).c_str());
            return 1;
        }

        m_showAction = new (std::nothrow) QAction(tr("Show"), this);
        if (!m_showAction)
        {
            qCritical(
                fmt::format("{}:{} Fail to allocate memory",
                            __FILE__, __LINE__).c_str());
            return 1;
        }

        m_exitAction = new (std::nothrow) QAction(tr("Exit"), this);
        if (!m_exitAction)
        {
            qCritical(
                fmt::format("{}:{} Fail to allocate memory",
                            __FILE__, __LINE__).c_str());
            return 1;
        }

        m_menu->addAction(m_showAction);
        m_menu->addAction(m_exitAction);
        m_trayIcon->setContextMenu(m_menu);
        m_trayIcon->setIcon(m_icon);
        m_trayIcon->show();
    }

    return 0;
}

void MainWindow::connectHook()
{
    // bar
    connect(m_ui->actionExit,
            &QAction::triggered,
            this,
            &MainWindow::exitProcess);

    if (QSystemTrayIcon::isSystemTrayAvailable())
    {
        // tray icons
        connect(m_trayIcon,
                &QSystemTrayIcon::activated,
                this,
                &MainWindow::onTrayIconActivated);

        connect(m_showAction,
                &QAction::triggered,
                this,
                &MainWindow::onShowActionTriggered);

        connect(m_exitAction,
                &QAction::triggered,
                this,
                &MainWindow::exitProcess);
    }
}

void MainWindow::cleanMemory()
{
    if (m_ui) delete m_ui;

    // forms
    if (m_configForm) delete m_configForm;

    // tray icon
    if (m_trayIcon) delete m_trayIcon;
    if (m_menu) delete m_menu;
    if (m_showAction) delete m_showAction;
    if (m_exitAction) delete m_exitAction;
}

} // namespace View
