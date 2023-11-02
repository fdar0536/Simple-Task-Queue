/* Simple Task Queue
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

#include <new>

#include "QCloseEvent"
#include "QMenu"
#include "QStandardPaths"

#include "controller/global/init.hpp"
#include "controller/gui/logsink.hpp"

#include "clientconfig.hpp"
#include "log.hpp"
#include "serverconfig.hpp"

#include "mainwindow.hpp"
#include "../../view/gui/ui_mainwindow.h"

namespace Controller
{

namespace GUI
{

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(nullptr)
{}

MainWindow::~MainWindow()
{
    if (m_ui) delete m_ui;
    trayIconFin();

    spdlog::set_default_logger(m_defaultLogger);
}

uint_fast8_t MainWindow::init()
{
    if (m_ui)
    {
        spdlog::error("{}:{} MainWindow is already initialized.",
                      __FILE__, __LINE__);
        return 1;
    }

    sqliteInit();
    spdlogInit();

    m_ui = new (std::nothrow) Ui::MainWindow;
    if (!m_ui)
    {
        spdlog::error("{}:{} Fail to allocate memory",
                      __FILE__, __LINE__);
        return 1;
    }

    try
    {
        m_ui->setupUi(this);
    }
    catch(...)
    {
        delete m_ui;
        m_ui = nullptr;
        spdlog::error("{}:{} Fail to allocate memory",
                      __FILE__, __LINE__);
        return 1;
    }

    if (trayIconInit())
    {
        spdlog::error("{}:{} trayIconInit failed", __FILE__, __LINE__);
        delete m_ui;
        m_ui = nullptr;
        trayIconFin();
        return 1;
    }

    connectHook();
    ServerConfig *serverConfig = new (std::nothrow) ServerConfig(this);
    if (!serverConfig)
    {
        delete m_ui;
        m_ui = nullptr;
        spdlog::error("{}:{} Fail to allocate memory",
                      __FILE__, __LINE__);
        return 1;
    }

    if (serverConfig->init())
    {
        delete m_ui;
        m_ui = nullptr;
        delete serverConfig;
        serverConfig = nullptr;
        trayIconFin();
        spdlog::error("{}:{} Fail to initialize server config",
                      __FILE__, __LINE__);
        return 1;
    }

    setCentralWidget(serverConfig);
    return 0;
}

// protected member functions
void MainWindow::closeEvent(QCloseEvent *event)
{
    event->ignore();
    hide();
}

// privatew slots
void
MainWindow::onIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger ||
        reason == QSystemTrayIcon::DoubleClick)
    {
        show();
    }
}

void MainWindow::onServerConfigActionTriggered(bool)
{
    updateCentralWidget<ServerConfig>();
}

void MainWindow::onClientConfigActionTriggered(bool)
{
    updateCentralWidget<ClientConfig>();
}

void MainWindow::onLogActionTriggered(bool)
{
    updateCentralWidget<Log>();
}

void MainWindow::onAboutQtActionTriggered(bool)
{
    QMessageBox::aboutQt(this);
}

void MainWindow::onExitActionTriggered(bool)
{
    show();
    auto res = QMessageBox::question(this,
                                     tr("Exit"),
                                     tr("Are you sure to exit?"),
                                     QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

    if (res != QMessageBox::Yes)
    {
        return;
    }

    QApplication::exit(0);
}

void MainWindow::onShowActionTriggered(bool)
{
    show();
}

// private member functions
void MainWindow::sqliteInit()
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

void MainWindow::spdlogInit()
{
    m_defaultLogger = spdlog::default_logger();
    auto sink = std::make_shared<LogSink_mt>();
    auto logger = std::make_shared<spdlog::logger>("STQLogger", sink);

    spdlog::set_default_logger(logger);
}

uint_fast8_t MainWindow::trayIconInit()
{
    if (QSystemTrayIcon::isSystemTrayAvailable())
    {
        m_icon = new (std::nothrow) QSystemTrayIcon(
            QIcon(":/icons/computer.svg"),
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
            trayIconFin();
            spdlog::error("{}:{} Fail to allocate memory", __FILE__, __LINE__);
            return 1;
        }

        m_showAction = new (std::nothrow) QAction("Show", this);
        if (!m_showAction)
        {
            trayIconFin();
            spdlog::error("{}:{} Fail to allocate memory", __FILE__, __LINE__);
            return 1;
        }

        m_iconContextMenu->addAction(m_showAction);
        m_iconContextMenu->addAction(m_ui->actionExit);
        m_icon->setContextMenu(m_iconContextMenu);
    }

    return 0;
}

void MainWindow::trayIconFin()
{
    if (QSystemTrayIcon::isSystemTrayAvailable())
    {
        if (m_icon)
        {
            delete m_icon;
            m_icon = nullptr;
        }

        if (m_iconContextMenu)
        {
            delete m_iconContextMenu;
            m_iconContextMenu = nullptr;
        }

        if (m_showAction)
        {
            delete m_showAction;
            m_showAction = nullptr;
        }
    }
}

void MainWindow::connectHook()
{
    connect(
        m_ui->actionServer_Config,
        &QAction::triggered,
        this,
        &MainWindow::onServerConfigActionTriggered
    );

    connect(
        m_ui->actionClient_Config,
        &QAction::triggered,
        this,
        &MainWindow::onClientConfigActionTriggered
    );

    connect(
        m_ui->actionLog,
        &QAction::triggered,
        this,
        &MainWindow::onLogActionTriggered
    );

    connect(m_ui->actionAbout_Qt,
            &QAction::triggered,
            this,
            &MainWindow::onAboutQtActionTriggered);

    connect(m_ui->actionExit,
            &QAction::triggered,
            this,
            &MainWindow::onExitActionTriggered);

    if (QSystemTrayIcon::isSystemTrayAvailable())
    {
        connect(m_icon,
                &QSystemTrayIcon::activated,
                this,
                &MainWindow::onIconActivated);

        connect(m_showAction,
                &QAction::triggered,
                this,
                &MainWindow::onShowActionTriggered);
    }
}

} // namespace GUI

} // end namespace Controller
