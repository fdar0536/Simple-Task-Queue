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
#include "QApplication"
#include "QIcon"
#include "QMenu"
#include "QMessageBox"

#include "main.hpp"

namespace View
{

// public member functions
Main::Main(QObject *parent):
    QObject{parent}
{}

Main::~Main()
{
    cleanMemory();
}

bool Main::init()
{
    if (setupTrayIcon())
    {
        qCritical(
            fmt::format("{}:{} Setup tray icon failed",
                        __FILE__, __LINE__).c_str());
        cleanMemory();
        return 1;
    }

    connectHook();
    return 0;
}

void Main::exit()
{
    qApp->exit(0);
}

// public slots
void Main::exitProcess(bool)
{
    emit show();
    if (QMessageBox::question(nullptr, tr("Exit"),
                              tr("Are you sure to exit?\n"
                                 "All unfinished process in embadded mode "
                                 "will be killed"),
                              QMessageBox::Yes |
                              QMessageBox::Cancel |
                              QMessageBox::No)
        != QMessageBox::Yes)
    {
        return;
    }

    exit();
}

// private slots
void Main::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick)
    {
        emit show();
    }
}

// private member functions
u8 Main::setupTrayIcon()
{
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

        m_menu = new (std::nothrow) QMenu(nullptr);
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
        m_trayIcon->setIcon(QIcon("://original-icon.jpg"));
        m_trayIcon->show();
    }

    return 0;
}

void Main::connectHook()
{
    if (QSystemTrayIcon::isSystemTrayAvailable())
    {
        // tray icons
        connect(m_trayIcon,
                &QSystemTrayIcon::activated,
                this,
                &Main::onTrayIconActivated);

        connect(m_showAction,
                &QAction::triggered,
                this,
                [&](bool)
                {
                    emit show();
                });

        connect(m_exitAction,
                &QAction::triggered,
                this,
                &Main::exitProcess);
    }
}

void Main::cleanMemory()
{
    // tray icon
    if (m_trayIcon) delete m_trayIcon;
    if (m_menu) delete m_menu;
    if (m_showAction) delete m_showAction;
    if (m_exitAction) delete m_exitAction;
}

} // namespace View
