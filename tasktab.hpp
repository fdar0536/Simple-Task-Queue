/*
 * Simple Task Queue
 * Copyright (c) 2020 fdar0536
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

#include <memory>
#include <map>

#include "QWidget"
#include "QProcess"
#include "QTableWidgetItem"

#include "inputdialog.hpp"

namespace Ui
{
    class TaskTab;
}

typedef enum STATUS
{
    idle, running
} STATUS;

class TaskTab : public QWidget
{
    Q_OBJECT

public:

    static TaskTab *create(QWidget *parent = nullptr);

    ~TaskTab();

signals:

    void renameRequried(TaskTab *, QString &);

private slots:

    void on_workDirBrowseBtn_clicked();

    void on_pname_textChanged(const QString);

    void on_AddQueueBtn_clicked();

    void on_TriggleStartBtn_clicked();

    void on_AbortBtn_clicked();

    void on_CancelSelectedBtn_clicked();

    void on_CancelAllBtn_clicked();

    void on_CleanDoneBtn_clicked();

    void on_consoleClearBtn_clicked();

    void onProcessExited(int, QProcess::ExitStatus);

    void handle_process_stderr();

    void handle_process_stdout();

    // InputDialog
    void onInputAccepted();

private:

    explicit TaskTab(QWidget *parent = nullptr) :
        QWidget(parent),
        m_ui(nullptr),
        m_mainProcess(nullptr),
        m_status(idle),
        m_prevData("")
    {}

    TaskTab(const TaskTab &) = delete;

    TaskTab() = delete;

    TaskTab &operator=(TaskTab) = delete;

    Ui::TaskTab *m_ui;

    QProcess *m_mainProcess;

    InputDialog *m_inputDialog;

    STATUS m_status;

    void connectHook();

    QByteArray m_prevData;

    void writeConsole(const QByteArray &);

    QTableWidgetItem *getTableWidgetItem(const QString &);

    void killProcess();
};
