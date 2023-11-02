/* This file is modified from spdlog
 * Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
 * Distributed under the MIT License (http://opensource.org/licenses/MIT)
 *
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

#include "spdlog/spdlog.h"
#include "QMessageBox"
#include "QSettings"

#include "controller/global/init.hpp"
#include "model/utils.hpp"
#include "global.hpp"

#include "clientconfig.hpp"
#include "../../view/gui/ui_clientconfig.h"

namespace Controller
{

namespace GUI
{

class _Data
{
public:

    ~_Data()
    {
        QSettings settings;
        settings.setValue("Server List", data);
    }

    QHash<QString, QVariant> data;

    std::atomic<bool> isInit = false;

    std::mutex mutex;
};

_Data _data;

ClientConfig::ClientConfig(QWidget *parent) :
    QWidget(parent),
    m_ui(nullptr),
    m_thread(nullptr)
{}

ClientConfig::~ClientConfig()
{
    if (m_ui) delete m_ui;

    if (m_thread) delete m_thread;
}

uint_fast8_t ClientConfig::init()
{
    m_ui = new (std::nothrow) Ui::ClientConfig;
    if (!m_ui)
    {
        spdlog::error("{}:{} Fail to allocate memory", __FILE__, __LINE__);
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

    m_thread = new (std::nothrow) ClientConfigThread(this);
    if (!m_thread)
    {
        delete m_ui;
        m_ui = nullptr;
        spdlog::error("{}:{} Fail to allocate memory",
                      __FILE__, __LINE__);
        return 1;
    }

    connectHook();
    setEnabled(false);
    if (_data.isInit.load(std::memory_order_relaxed))
    {
        onParseConfigDone();
        return 0;
    }

    if (m_thread->startParseConfig(&_data.data))
    {
        delete m_ui;
        m_ui = nullptr;
        delete m_thread;
        m_thread = nullptr;
        spdlog::error("{}:{} Fail to parse config",
                      __FILE__, __LINE__);
        return 1;
    }

    setEnabled(false);
    return 0;
}

// private slots
// ui
void ClientConfig::onUseRemoteStateChanged(int state)
{
    bool res = (state == Qt::Checked);
    m_ui->remoteConfig->setEnabled(res);
    if (res) Controller::Global::guiGlobal.setBackendMode(Global::GRPC);
    else Controller::Global::guiGlobal.setBackendMode(Global::SQLITE);
}

void ClientConfig::onRemoteListItemActivated(QListWidgetItem *item)
{
    if (!item)
    {
        return;
    }

    QHash<QString, QVariant> hash;

    {
        std::lock_guard<std::mutex> lock(_data.mutex);
        hash = _data.data[item->text()].toHash();
    }

    m_ui->remoteName->setText(item->text());
    m_ui->remoteHost->setText(hash["ip"].toString());
    m_ui->remotePort->setValue(hash["port"].toInt());
}

void ClientConfig::onRemoveClicked(bool)
{
    if (!m_ui->remoteList->count()) // count == 0
    {
        spdlog::warn("{}:{} no item to remove.", __FILE__, __LINE__);
        return;
    }

    auto item = m_ui->remoteList->currentItem();
    if (!item)
    {
        spdlog::warn("{}:{} item is nullptr.", __FILE__, __LINE__);
        return;
    }

    {
        std::lock_guard<std::mutex> lock(_data.mutex);
        _data.data.remove(m_ui->remoteList->currentItem()->text());
    }

    m_ui->remoteList->removeItemWidget(item);
    delete item;
}

void ClientConfig::onSaveClicked(bool)
{
    if (Model::Utils::verifyIP(m_ui->remoteHost->text().toUtf8().toStdString()))
    {
        UNUSED(QMessageBox::critical(this, tr("Error"), tr("Invalid IP")));
        return;
    }

    if (addItemIntoRemoteList(true))
    {
        return;
    }

    if (QMessageBox::question(this,
                              tr("Overwrite"),
                              tr("The key has existed, overwrite it?"),
                              QMessageBox::StandardButton::Yes |
                              QMessageBox::StandardButton::No |
                              QMessageBox::StandardButton::Cancel)
        == QMessageBox::StandardButton::Yes)
    {
        UNUSED(addItemIntoRemoteList(false));
    }
}

void ClientConfig::onClearClicked(bool)
{
    m_ui->remoteName->clear();
    m_ui->remoteHost->clear();
    m_ui->remotePort->setValue(12345);
}

void ClientConfig::onConnectClicked(bool)
{
    if (Model::Utils::verifyIP(m_ui->remoteHost->text().toUtf8().toStdString()))
    {
        UNUSED(QMessageBox::critical(this, tr("Error"), tr("Invalid IP")));
        return;
    }

    if (m_thread->startConnect(m_ui->remoteHost->text(), m_ui->remotePort->value()))
    {
        UNUSED(QMessageBox::critical(this, tr("Error"), tr("Connect failed")));
        return;
    }
    setEnabled(false);
}

// threads
void ClientConfig::onParseConfigDone()
{
    m_ui->remoteList->clear();

    if (_data.data.isEmpty())
    {
        goto exit;
    }

    for (auto it = _data.data.begin(); it != _data.data.end(); ++it)
    {
        m_ui->remoteList->addItem(it.key());
    }

exit:

    if (Controller::Global::sqliteQueueList == nullptr)
    {
        m_ui->useRemote->setEnabled(false);
        m_ui->useRemote->setChecked(true);
    }
    else
    {
        m_ui->remoteConfig->setEnabled(false);
    }

    setEnabled(true);
    _data.isInit.store(true, std::memory_order_relaxed);
}

void ClientConfig::onConnnectDone(bool res)
{
    setEnabled(true);
    if (res)
    {
        QMessageBox::information(this, tr("Done"), tr("Connect successful"));
        return;
    }

    QMessageBox::critical(this, tr("Error"), tr("Connect failed"));
}

// private member function
void ClientConfig::connectHook()
{
    // ui
    connect(
        m_ui->useRemote,
        &QCheckBox::stateChanged,
        this,
        &ClientConfig::onUseRemoteStateChanged
    );

    connect(
        m_ui->remoteList,
        &QListWidget::itemActivated,
        this,
        &ClientConfig::onRemoteListItemActivated
    );

    connect(
        m_ui->remove,
        &QPushButton::clicked,
        this,
        &ClientConfig::onRemoveClicked
    );

    connect(
        m_ui->save,
        &QPushButton::clicked,
        this,
        &ClientConfig::onSaveClicked
    );

    connect(
        m_ui->clear,
        &QPushButton::clicked,
        this,
        &ClientConfig::onClearClicked
    );

    connect(
        m_ui->Connect,
        &QPushButton::clicked,
        this,
        &ClientConfig::onConnectClicked
    );

    // thread
    connect(
        m_thread,
        &ClientConfigThread::parseConfigDone,
        this,
        &ClientConfig::onParseConfigDone
    );

    connect(
        m_thread,
        &ClientConfigThread::connectDone,
        this,
        &ClientConfig::onConnnectDone
    );
}

bool ClientConfig::addItemIntoRemoteList(bool notOverwrite)
{
    if (_data.data.find(m_ui->remoteName->text()) == _data.data.end())
    {
        m_ui->remoteList->addItem(m_ui->remoteName->text());
        goto add_data;
    }

    if (notOverwrite)
    {
        return false;
    }

add_data:

    {
        std::lock_guard<std::mutex> lock(_data.mutex);
        // overwrite the value
        QHash<QString, QVariant> data;
        data["ip"] = m_ui->remoteHost->text();
        data["port"] = m_ui->remotePort->value();
        _data.data[m_ui->remoteName->text()] = data;
    }

    return true;
}

} // end namespace GUI

} // end namespace Controller
