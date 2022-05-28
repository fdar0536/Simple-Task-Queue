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

#include <new>

#include "QDir"
#include "QFileDialog"
#include "QMessageBox"

#include "loggerdialog.hpp"
#include "ui_loggerdialog.h"

LoggerDialog *LoggerDialog::create(QWidget *parent)
{
    LoggerDialog *ret = new (std::nothrow) LoggerDialog(parent);
    if (!ret) return nullptr;

    ret->m_ui = new (std::nothrow) Ui::LoggerDialog;
    if (!ret->m_ui)
    {
        delete ret;
        return nullptr;
    }

    ret->m_ui->setupUi(ret);
    return ret;
}

LoggerDialog::~LoggerDialog()
{
    if (m_ui) delete m_ui;
}

void LoggerDialog::write(Level level, QString &msg)
{
    write(level, msg.toLocal8Bit().constData());
}

void LoggerDialog::write(Level level, const char *in)
{
    if (!in || m_logLevel == Level::Off || level < m_logLevel) return;

    std::unique_lock<std::mutex> lock(m_mutex);
    time_t current = time(nullptr);
    printTimeStamp(current);
    char label[8192];
    label[0] = '\0';
    switch (level)
    {
    case Verbose:
    {
        sprintf(label, "%s Verbose: %s\n", m_log, in);
        break;
    }
    case Debug:
    {
        sprintf(label, "%s Debug: %s\n", m_log, in);
        break;
    }
    case Info:
    {
        sprintf(label, "%s Info: %s\n", m_log, in);
        break;
    }
    case Warning:
    {
        sprintf(label, "%s Warning: %s\n", m_log, in);
        break;
    }
    case Error:
    {
        sprintf(label, "%s Error: %s\n", m_log, in);
        break;
    }
    default:
    {
        break;
    }
    }

    m_ui->log->appendPlainText(label);
}

// private slots
void LoggerDialog::on_saveBtn_clicked()
{
#ifdef Q_OS_WINDOWS
    QString res = QFileDialog::getSaveFileName(this,
                                               "Save log",
                                               QDir::currentPath(),
                                               "log (*.log)");
#else
    QString res = QFileDialog::getSaveFileName(this,
                                               "Save log",
                                               QDir::homePath(),
                                               "log (*.log)");
#endif

    if (res.isEmpty()) return;

    std::unique_lock<std::mutex> lock(m_mutex);
    FILE *f(nullptr);
    f = fopen(res.toLocal8Bit().constData(), "w");
    if (!f)
    {
        QMessageBox::critical(this, "Error", "Fail to open file.");
        return;
    }

    auto text = m_ui->log->toPlainText().toLocal8Bit();
    fwrite(text.constData(), 1, text.size(), f);
    fclose(f);
}

void LoggerDialog::on_clearBtn_clicked()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_ui->log->clear();
}

// private member functions
LoggerDialog::LoggerDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(nullptr),
    m_logLevel(Info)
{}

void LoggerDialog::printTimeStamp(time_t in)
{
    struct tm *tm = localtime(&in);
    if (!tm) return;

    sprintf(m_log,
            "%d-%02d-%02d %02d:%02d:%02d",
            tm->tm_year + 1900,
            tm->tm_mon + 1,
            tm->tm_mday,
            tm->tm_hour,
            tm->tm_min,
            tm->tm_sec);
}
