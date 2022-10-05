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

#include "QCloseEvent"
#include "QMenu"
#include "QMessageBox"

#include "donelist.hpp"
#include "pendinglist.hpp"
#include "queuelist.hpp"
#include "settings.hpp"

#include "mainwindow.hpp"
#include "../view/ui_mainwindow.h"

#include "config.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(nullptr),
    m_global(nullptr),
    m_icon(nullptr),
    m_iconContextMenu(nullptr),
    m_showAction(nullptr),
    m_exitAction(nullptr),
    m_currentWidget(SETTINGS)
{}

MainWindow::~MainWindow()
{
    if (m_ui) delete m_ui;

    if (QSystemTrayIcon::isSystemTrayAvailable())
    {
        if (m_icon) delete m_icon;
        if (m_iconContextMenu) delete m_iconContextMenu;
        if (m_showAction) delete m_showAction;
        if (m_exitAction) delete m_exitAction;
    }
}

uint8_t MainWindow::init()
{
    try
    {
        m_ui = new Ui::MainWindow;
    }
    catch (...)
    {
        m_ui = nullptr;
        return 1;
    }

    m_ui->setupUi(this);

    m_global = Global::instance();
    if (m_global == nullptr)
    {
        return 1;
    }

    //trayIcon
    if (QSystemTrayIcon::isSystemTrayAvailable())
    {
        m_icon = new (std::nothrow) QSystemTrayIcon(QIcon(":/view/icon/computer_black_48dp.svg"),
                                                    this);
        if (!m_icon)
        {
            return 1;
        }

        m_icon->show();

        //iconContextMenu
        m_iconContextMenu = new (std::nothrow) QMenu;
        if (!m_iconContextMenu)
        {
            return 1;
        }

        m_showAction = new (std::nothrow) QAction("Show", this);
        if (!m_showAction)
        {
            return 1;
        }

        m_exitAction = new (std::nothrow) QAction("Exit", this);
        if (!m_exitAction)
        {
            return 1;
        }

        m_iconContextMenu->addAction(m_showAction);
        m_iconContextMenu->addAction(m_exitAction);
        m_icon->setContextMenu(m_iconContextMenu);
    }

    Settings *toBeInsert = Settings::create(this);
    if (!toBeInsert)
    {
        return 1;
    }

    setCentralWidget(toBeInsert);
    connectHook();

    QString title(STQ_NAME);
    title += " ";
    title += STQ_VERSION;
    setWindowTitle(title);
    return 0;
}

// protected member function
void MainWindow::closeEvent(QCloseEvent *e)
{
    e->ignore();
    hide();
}

// private slots
void MainWindow::programExit()
{
    auto res(QMessageBox::question(nullptr,
                                   tr("Exit"),
                                   tr("Are you sure to exit?"),
                                   QMessageBox::Yes |
                                   QMessageBox::No |
                                   QMessageBox::Cancel));
    if (res == QMessageBox::Yes)
    {
        QApplication::quit();
    }
}

// Tray Icon
void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger ||
        reason == QSystemTrayIcon::DoubleClick)
    {
        show();
    }
}

#define CREATE_WIDGET(targetCode, target, errMsg) \
    if (cleanCentral(targetCode)) return; \
    target *widget(target::create(this)); \
    if (!widget) \
    { \
        QMessageBox::critical(this, \
                              "Error", \
                              errMsg); \
        return; \
    } \
    setCentralWidget(widget); \
    m_currentWidget = targetCode

void MainWindow::onActionSettingsTriggered()
{
    CREATE_WIDGET(SETTINGS, Settings, "Fail to initialize \"Settings\"");
}

void MainWindow::onActionQueueListTriggered()
{
    CREATE_WIDGET(QUEUELIST, QueueList, "Fail to initialize \"Queue list\"");
}

void MainWindow::onActionPendingTriggered()
{
    CREATE_WIDGET(PENDING, PendingList, "Fail to initialize \"Pending list\"");
}

void MainWindow::onActionDoneTriggered()
{
    CREATE_WIDGET(DONE, DoneList, "Fail to initialize \"Done list\"");
}

#undef CREATE_WIDGET

void MainWindow::onActionAboutQtTriggered()
{
    QMessageBox::aboutQt(this);
}

// private member functions
void MainWindow::connectHook()
{
    connect(m_ui->actionExit,
            &QAction::triggered,
            this,
            &MainWindow::programExit);

    //trayIcon
    if (QSystemTrayIcon::isSystemTrayAvailable())
    {
        connect(m_icon,
                &QSystemTrayIcon::activated,
                this,
                &MainWindow::iconActivated);

        connect(m_showAction,
                &QAction::triggered,
                this,
                &MainWindow::show);

        connect(m_exitAction,
                &QAction::triggered,
                this,
                &MainWindow::programExit);
    }

    // ui
    connect(m_ui->actionSettings,
            &QAction::triggered,
            this,
            &MainWindow::onActionSettingsTriggered);

    connect(m_ui->actionQueueList,
            &QAction::triggered,
            this,
            &MainWindow::onActionQueueListTriggered);

    connect(m_ui->actionPending,
            &QAction::triggered,
            this,
            &MainWindow::onActionPendingTriggered);

    connect(m_ui->actionDone,
            &QAction::triggered,
            this,
            &MainWindow::onActionDoneTriggered);

    connect(m_ui->actionAboutQt,
            &QAction::triggered,
            this,
            &MainWindow::onActionAboutQtTriggered);
}

uint8_t MainWindow::cleanCentral(CurrentWidget index)
{
    if (index == m_currentWidget)
    {
        return 1;
    }

    if (index != SETTINGS)
    {
        if (m_global->grpcChannel() == nullptr)
        {
            return 1;
        }
    }

    QWidget *widget(takeCentralWidget());
    delete widget;
    return 0;
}
