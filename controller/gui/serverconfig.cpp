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

#include <new>

#include "QCoreApplication"
#include "QKeyEvent"
#include "QMessageBox"

#include "controller/global/init.hpp"
#include "controller/gui/global.hpp"
#include "serverconfig.hpp"

#include "../../view/gui/ui_serverconfig.h"

namespace Controller
{

namespace GUI
{

ServerConfig::ServerConfig(QWidget *parent) :
    QWidget(parent),
    m_ui(nullptr),
    m_waitForInit(nullptr)
{
}

ServerConfig::~ServerConfig()
{
    if (m_ui) delete m_ui;
    if (m_waitForInit) delete m_waitForInit;
}

uint_fast8_t ServerConfig::init()
{
    if (m_ui)
    {
        spdlog::error("{}:{} Server config is already initialized",
                      __FILE__, __LINE__);
        return 1;
    }

    m_ui = new (std::nothrow) Ui::ServerConfig;
    if (!m_ui)
    {
        spdlog::error("{}:{} Fail to allocate memory",
                      __FILE__, __LINE__);
        return 1;
    }

    m_waitForInit = new (std::nothrow) WaitForInit(this);
    if (!m_waitForInit)
    {
        delete m_ui;
        m_ui = nullptr;
        spdlog::error("{}:{} Fail to allocate memory",
                      __FILE__, __LINE__);
        return 1;
    }

    m_ui->setupUi(this);
    connectHook();
    m_waitForInit->start();
    return 0;
}

// private slots
void ServerConfig::onWaitForInitDone()
{
    m_ui->ip->setText(QString::fromStdString(Controller::Global::config.listenIP()));
    m_ui->port->setValue(Controller::Global::config.listenPort());
    if (!Controller::Global::config.autoStartServer())
    {
        goto exit;
    }

    m_ui->autostart->setChecked(true);
    onStartClicked(true);

exit:
    setEnabled(true);
}

void ServerConfig::onAutoStartClicked(bool)
{
    if (m_ui->autostart->checkState() == Qt::Unchecked)
    {
        Controller::Global::config.setAutoStartServer(false);
    }
    else
    {
        Controller::Global::config.setAutoStartServer(true);
    }
}

void ServerConfig::onServerEditFinished()
{
    if (m_ui->ip->text().isEmpty())
    {
        return;
    }

    onStartClicked(true);
}

void ServerConfig::onClearClicked(bool)
{
    m_ui->ip->clear();
    m_ui->port->setValue(12345);
}

void ServerConfig::onStartClicked(bool)
{
    if (Controller::Global::config.setListenIP(m_ui->ip->text().toUtf8().toStdString()))
    {
        QMessageBox::critical(this, tr("Error"), tr("Invalid ip"));
        return;
    }

    if (Controller::Global::server.start())
    {
        QMessageBox::critical(this, tr("Error"), tr("Fail to start server"));
        return;
    }

    m_ui->ip->setEnabled(false);
    m_ui->port->setEnabled(false);
    m_ui->clear->setEnabled(false);
    m_ui->start->setEnabled(false);
    m_ui->stop->setEnabled(true);
}

void ServerConfig::onStopClicked(bool)
{
    Controller::Global::server.stop();
    m_ui->ip->setEnabled(true);
    m_ui->port->setEnabled(true);
    m_ui->clear->setEnabled(true);
    m_ui->start->setEnabled(true);
    m_ui->stop->setEnabled(false);
}

// private member functions
void ServerConfig::connectHook()
{
    connect(
        m_waitForInit,
        &WaitForInit::done,
        this,
        &ServerConfig::onWaitForInitDone
    );

    connect(
        m_ui->autostart,
        &QCheckBox::clicked,
        this,
        &ServerConfig::onAutoStartClicked
    );

    connect(
        m_ui->ip,
        &QLineEdit::returnPressed,
        this,
        &ServerConfig::onServerEditFinished
    );

    connect(
        m_ui->clear,
        &QPushButton::clicked,
        this,
        &ServerConfig::onClearClicked
    );

    connect(
        m_ui->start,
        &QPushButton::clicked,
        this,
        &ServerConfig::onStartClicked
    );

    connect(
        m_ui->stop,
        &QPushButton::clicked,
        this,
        &ServerConfig::onStopClicked
    );
}

} // namespace GUI

} // end namespace Controller
