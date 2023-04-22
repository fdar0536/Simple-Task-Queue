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

#include "controller/gui/logsink.hpp"
#include "spdlog/spdlog.h"

#include "QThreadPool"

#ifdef STQ_MOBILE
#include "QGuiApplication"
#else
#include "QApplication"
#include "QMenu"
#include "QStandardPaths"
#endif

#include "controller/global/init.hpp"

#include "main.hpp"

namespace Controller
{

namespace GUI{
Main::Main(QObject *parent) :
    QObject(parent),
    m_defaultLogger(nullptr)
#ifndef STQ_MOBILE
    ,
    m_icon(nullptr),
    m_iconContextMenu(nullptr),
    m_showAction(nullptr),
    m_exitAction(nullptr)
#endif
{}

Main::~Main()
{
#ifndef STQ_MOBILE
    if (QSystemTrayIcon::isSystemTrayAvailable())
    {
        if (m_icon) delete m_icon;
        if (m_iconContextMenu) delete m_iconContextMenu;
        if (m_showAction) delete m_showAction;
        if (m_exitAction) delete m_exitAction;
    }
#endif

    spdlog::set_default_logger(m_defaultLogger);
}

bool Main::init()
{
    spdlogInit();

#ifdef STQ_MOBILE
    return true;
#else

    sqliteInit();

    if ((Controller::Global::sqliteQueueList != nullptr) &&
        (Controller::Global::config.autoStartServer()))
    {
        if (Controller::Global::server.start())
        {
            spdlog::error("{}:{} Fail to start grpc server", __FILE__, __LINE__);
        }
    }

    if (trayIconInit())
    {
        spdlog::error("{}:{} trayIconInit failed", __FILE__, __LINE__);
        return false;
    }

    return true;
#endif
}

// public slots
void Main::AboutQt()
{
    qApp->aboutQt();
}

// private slots
#ifndef STQ_MOBILE
void Main::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger ||
        reason == QSystemTrayIcon::DoubleClick)
    {
        emit Show();
    }
}
#endif

// private member functions
#ifndef STQ_MOBILE
uint_fast8_t Main::trayIconInit()
{
    if (QSystemTrayIcon::isSystemTrayAvailable())
    {
        m_icon = new (std::nothrow) QSystemTrayIcon(
            QIcon(":/view/gui/icons/computer.svg"),
            this);
        if (!m_icon)
        {
            spdlog::error("{}:{} Fail to allocate memory", __FILE__, __LINE__);
            return 1;
        }

        m_icon->show();

        //iconContextMenu
        m_iconContextMenu = new (std::nothrow) QMenu;
        if (!m_iconContextMenu)
        {
            spdlog::error("{}:{} Fail to allocate memory", __FILE__, __LINE__);
            return 1;
        }

        m_showAction = new (std::nothrow) QAction("Show", this);
        if (!m_showAction)
        {
            spdlog::error("{}:{} Fail to allocate memory", __FILE__, __LINE__);
            return 1;
        }

        m_exitAction = new (std::nothrow) QAction("Exit", this);
        if (!m_exitAction)
        {
            spdlog::error("{}:{} Fail to allocate memory", __FILE__, __LINE__);
            return 1;
        }

        m_iconContextMenu->addAction(m_showAction);
        m_iconContextMenu->addAction(m_exitAction);
        m_icon->setContextMenu(m_iconContextMenu);

        // connect
        connect(m_icon,
                &QSystemTrayIcon::activated,
                this,
                &Main::iconActivated);

        connect(m_showAction,
                &QAction::triggered,
                this,
                &Main::Show);

        connect(m_exitAction,
                &QAction::triggered,
                this,
                &Main::Exit);
    }

    return 0;
}

void Main::sqliteInit()
{
    if (Controller::Global::sqliteQueueList != nullptr)
    {
        return;
    }

    QString dataPath = QStandardPaths::writableLocation(
        QStandardPaths::AppLocalDataLocation);
    if (dataPath.isEmpty())
    {
        spdlog::warn("{}:{} No writable location", __FILE__, __LINE__);
        return;
    }

    Controller::Global::config.setDbPath(dataPath.toUtf8().toStdString());
    if (Controller::Global::initSQLiteQueueList())
    {
        spdlog::error("{}:{} Fail to initialize SQLite", __FILE__, __LINE__);
    }
}

#endif

void Main::spdlogInit()
{
    m_defaultLogger = spdlog::default_logger();
    auto sink = std::make_shared<LogSink_mt>();
    auto logger = std::make_shared<spdlog::logger>("STQLogger", sink);

    spdlog::set_default_logger(logger);
}

} // namespace GUI

} // end namespace Controller
