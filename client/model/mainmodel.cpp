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

#include "QApplication"
#include "QMessageBox"

#include "mainmodel.hpp"

// public member functions
MainModel::MainModel() :
    QObject(),
    m_isInit(false),
    m_icon(nullptr),
    m_iconContextMenu(nullptr),
    m_showAction(nullptr),
    m_exitAction(nullptr)
{}

MainModel::~MainModel()
{
    if (QSystemTrayIcon::isSystemTrayAvailable())
    {
        if (m_icon) delete m_icon;
        if (m_iconContextMenu) delete m_iconContextMenu;
        if (m_showAction) delete m_showAction;
        if (m_exitAction) delete m_exitAction;
    }
}

bool MainModel::init()
{
    if (m_isInit)
    {
        return false;
    }

    //trayIcon
    if (QSystemTrayIcon::isSystemTrayAvailable())
    {
        m_icon = new (std::nothrow) QSystemTrayIcon(QIcon(":/ui/icon/computer_black_48dp.svg"),
                                                    this);
        if (!m_icon)
        {
            return true;
        }

        m_icon->show();

        //iconContextMenu
        m_iconContextMenu = new (std::nothrow) QMenu;
        if (!m_iconContextMenu)
        {
            return true;
        }

        m_showAction = new (std::nothrow) QAction("Show", this);
        if (!m_showAction)
        {
            return true;
        }

        m_exitAction = new (std::nothrow) QAction("Exit", this);
        if (!m_exitAction)
        {
            return true;
        }

        m_iconContextMenu->addAction(m_showAction);
        m_iconContextMenu->addAction(m_exitAction);
        m_icon->setContextMenu(m_iconContextMenu);
    }

    connectHook();
    m_isInit = true;
    return false;
}

void MainModel::aboutQt()
{
    QMessageBox::aboutQt(nullptr);
}

// public slots
void MainModel::programExit()
{
    auto res(QMessageBox::question(nullptr,
                                   tr("Exit"),
                                   tr("Are you sure to exit?"),
                                   QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel));
    if (res == QMessageBox::Yes)
    {
        QApplication::quit();
    }
}

// private slots
// Tray Icon
void MainModel::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger ||
        reason == QSystemTrayIcon::DoubleClick)
    {
        emit showWindow();
    }
}

// private member functions
void MainModel::connectHook()
{
    //trayIcon
    if (QSystemTrayIcon::isSystemTrayAvailable())
    {
        connect(m_icon,
                &QSystemTrayIcon::activated,
                this,
                &MainModel::iconActivated);

        connect(m_showAction,
                &QAction::triggered,
                this,
                &MainModel::showWindow);

        connect(m_exitAction,
                &QAction::triggered,
                this,
                &MainModel::programExit);
    }
}
