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

#include "QMenu"
#include "QStandardPaths"

#include "controller/global/init.hpp"
#include "controller/gui/logsink.hpp"

#include "mainwindow.hpp"
#include "serverconfig.hpp"
#include "../../view/gui/ui_mainwindow.h"

namespace Controller
{

namespace GUI
{

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(nullptr),
    m_centerWidget(CenterWidget::ServerConfig)
{}

MainWindow::~MainWindow()
{
    if (m_ui) delete m_ui;
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

    m_ui->setupUi(this);

    ServerConfig *serverConfig = new (std::nothrow) ServerConfig(this);
    if (!serverConfig)
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
        return 1;
    }

    if (serverConfig->init())
    {
        delete m_ui;
        m_ui = nullptr;
        delete serverConfig;
        serverConfig = nullptr;
        spdlog::error("{}:{} Fail to initialize server config",
                      __FILE__, __LINE__);
        return 1;
    }

    setCentralWidget(serverConfig);
    return 0;
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
            spdlog::error("{}:{} Fail to allocate memory", __FILE__, __LINE__);
            return 1;
        }

        m_showAction = new (std::nothrow) QAction("Show", this);
        if (!m_showAction)
        {
            spdlog::error("{}:{} Fail to allocate memory", __FILE__, __LINE__);
            return 1;
        }

        m_iconContextMenu->addAction(m_showAction);
        m_iconContextMenu->addAction(m_ui->actionExit);
        m_icon->setContextMenu(m_iconContextMenu);
    }

    return 0;
}

} // namespace GUI

} // end namespace Controller
