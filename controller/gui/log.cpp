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

#include <fstream>

#include "spdlog/spdlog.h"
#include "QFileDialog"
#include "QDir"

#include "controller/global/init.hpp"
#include "controller/gui/global.hpp"

#include "log.hpp"

#include "../../view/gui/ui_log.h"

#define LOG_UPDATE_INTERVAL 3000

namespace Controller
{

namespace GUI
{

// public member function
Log::Log(QWidget *parent) :
    QWidget(parent),
    m_ui(nullptr),
    m_timer(nullptr)
{}

Log::~Log()
{
    if (m_ui) delete m_ui;
    if (m_timer) delete m_timer;
}

uint_fast8_t Log::init()
{
    if (m_ui)
    {
        spdlog::error("{}:{} Server config is already initialized",
                      __FILE__, __LINE__);
        return 1;
    }

    m_ui = new (std::nothrow) Ui::Log;
    if (!m_ui)
    {
        spdlog::error("{}:{} Fail to allocate memory",
                      __FILE__, __LINE__);
        return 1;
    }

    m_ui->setupUi(this);

    m_timer = new (std::nothrow) QTimer(this);
    if (!m_timer)
    {
        delete m_ui;
        m_ui = nullptr;
        spdlog::error("{}:{} Fail to allocate memory",
                      __FILE__, __LINE__);
        return 1;
    }

    connectHook();
    onTimerTimeout();
    m_timer->start(LOG_UPDATE_INTERVAL);
    return 0;
}

// private slots
void Log::onSaveClicked(bool)
{
    std::unique_lock<std::mutex> lock(m_logMutex);
    if (m_ui->log->toPlainText().isEmpty())
    {
        return;
    }

#ifdef Q_OS_WIN32
    QString path = QFileDialog::getSaveFileName(this, tr("Save log"), QDir::currentPath());
#else
    QString path = QFileDialog::getSaveFileName(this, tr("Save log"), QDir::homePath());
#endif

    if (path.isEmpty()) return;

    std::fstream f;
    f.open(path.toUtf8().toStdString(),
           std::ifstream::out | std::ifstream::trunc);
    if (f.fail())
    {
        spdlog::error("{}:{} Fail to open file", __FILE__, __LINE__);
        return;
    }

    std::string output = m_ui->log->toPlainText().toUtf8().toStdString();
    f.write(output.c_str(), output.size());
    f.close();
}

void Log::onClearClicked(bool)
{
    std::unique_lock<std::mutex> lock(m_logMutex);
    m_ui->log->clear();
    m_ui->save->setEnabled(false);
    m_ui->clear->setEnabled(false);
}

void Log::onTimerTimeout()
{
    std::unique_lock<std::mutex> lock(m_logMutex);
    QList<QString> log;
    Controller::Global::guiGlobal.getLog(log);
    if (!log.length())
    {
        return;
    }

    for (auto it = log.begin(); it != log.end(); ++it)
    {
        m_ui->log->appendPlainText(*it);
        m_ui->log->appendPlainText("\n");
    }

    m_ui->save->setEnabled(true);
    m_ui->clear->setEnabled(true);
}

void Log::connectHook()
{
    connect(
        m_ui->save,
        &QPushButton::clicked,
        this,
        &Log::onSaveClicked
    );

    connect(
        m_ui->clear,
        &QPushButton::clicked,
        this,
        &Log::onClearClicked
    );

    connect(
        m_timer,
        &QTimer::timeout,
        this,
        &Log::onTimerTimeout
    );
}

} // namespace GUI

} // end namespace Controller
