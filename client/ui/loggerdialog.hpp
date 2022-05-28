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

#pragma once

#include <mutex>

#include "QDialog"

namespace Ui
{
class LoggerDialog;
}

class LoggerDialog : public QDialog
{
    Q_OBJECT

public:

    typedef enum Level
    {
        Verbose, Debug, Info, Warning, Error, Off
    } Level;

    static LoggerDialog *create(QWidget *parent = nullptr);

    ~LoggerDialog();

    void write(Level, QString &);

    void write(Level, const char *);

private slots:

    void on_saveBtn_clicked();

    void on_clearBtn_clicked();

private:

    LoggerDialog(QWidget *parent = nullptr);

    void printTimeStamp(time_t);

    Ui::LoggerDialog *m_ui;

    Level m_logLevel;

    char m_log[128];

    std::mutex m_mutex;
};

