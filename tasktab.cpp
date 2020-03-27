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

#include "QDir"
#include "QStandardPaths"
#include "QMessageBox"
#include "QFileDialog"

#ifdef Q_OS_WIN32
#include "windows.h"
#include "tlhelp32.h"
#else
#include <csignal>
#include "sys/types.h"
#include "unistd.h"
#endif // Q_OS_WIN32

#include "tasktab.hpp"
#include "ui_tasktab.h"

TaskTab *TaskTab::create(QWidget *parent)
{
    TaskTab *ret(new (std::nothrow) TaskTab(parent));
    if (!ret)
    {
        return nullptr;
    }

    ret->m_ui = new (std::nothrow) Ui::TaskTab();
    if (!ret->m_ui)
    {
        delete ret;
        return nullptr;
    }

    ret->m_ui->setupUi(ret);

    ret->m_mainProcess = new (std::nothrow) QProcess(ret);
    if (!ret->m_mainProcess)
    {
        delete ret;
        return nullptr;
    }

    ret->m_inputDialog = InputDialog::create(ret);
    if (!ret->m_inputDialog)
    {
        delete ret;
        return nullptr;
    }

    QStringList titles;
    titles << "Working directory" << "Program Name" << "Argument(s)";
    titles << "Status" << "Priority";
    ret->m_ui->TaskQueueTable->setHorizontalHeaderLabels(titles);
    titles.clear();
    titles << "Working directory" << "Program Name" << "Argument(s)";
    titles << "Exit code";
    ret->m_ui->DoneList->setHorizontalHeaderLabels(titles);
#ifdef Q_OS_WIN32
    ret->m_ui->workDirPath->setText(QDir::currentPath());
#else
    ret->m_ui->workDirPath->setText(QDir::homePath());
#endif
    ret->connectHook();

    return ret;
}

TaskTab::~TaskTab()
{
    if (m_mainProcess)
    {
        if (m_status == running)
        {
            m_ui->TaskQueueTable->setRowCount(0);
            killProcess();
        }

        delete m_mainProcess;
    }

    if (m_inputDialog) delete m_inputDialog;

    if (m_ui) delete m_ui;
}

// private slots
void TaskTab::on_workDirBrowseBtn_clicked()
{
    QString res(QFileDialog::getExistingDirectory(this,
                                                  "Open working dir",
                                                m_ui->workDirPath->text(),
                                                QFileDialog::ShowDirsOnly |
                                                QFileDialog::DontResolveSymlinks)
                );

    if (!res.isEmpty())
    {
        m_ui->workDirPath->setText(res);
    }
}

void TaskTab::on_pname_textChanged(const QString)
{
    m_ui->AddQueueBtn->setEnabled(m_ui->pname->text() != "");
}

void TaskTab::on_AddQueueBtn_clicked()
{
    QTableWidgetItem *dir, *name, *args, *status, *priority;
    dir = getTableWidgetItem(m_ui->workDirPath->text());
    name = getTableWidgetItem(m_ui->pname->text());
    args = getTableWidgetItem(m_ui->pargs->toPlainText());
    status = getTableWidgetItem("Pending");
    priority = getTableWidgetItem(
                QString::number(m_ui->priorityBox->currentIndex() + 1));

    m_ui->TaskQueueTable->insertRow(m_ui->TaskQueueTable->rowCount());
    int last_row(m_ui->TaskQueueTable->rowCount() - 1);
    m_ui->TaskQueueTable->setItem(last_row, 0, dir);
    m_ui->TaskQueueTable->setItem(last_row, 1, name);
    m_ui->TaskQueueTable->setItem(last_row, 2, args);
    m_ui->TaskQueueTable->setItem(last_row, 3, status);
    m_ui->TaskQueueTable->setItem(last_row, 4, priority);
    m_ui->TaskQueueTable->sortItems(4, Qt::AscendingOrder);

    if (!m_ui->CancelSelectedBtn->isEnabled())
    {
        m_ui->CancelSelectedBtn->setEnabled(true);
    }

    if (!m_ui->CancelAllBtn->isEnabled())
    {
        m_ui->CancelAllBtn->setEnabled(true);
    }

    if (this->m_status == idle &&
        m_ui->TaskQueueTable->rowCount() != 0)
    {
        on_TriggleStartBtn_clicked(); //for autostart
    }
}

void TaskTab::on_TriggleStartBtn_clicked()
{
    if (m_ui->TaskQueueTable->rowCount() == 0) //queue是空的
    {
        if (m_status == running)
        {
            m_status = idle;
            m_ui->TriggleStartBtn->setEnabled(false);
            m_ui->AbortBtn->setEnabled(false);
            m_ui->CancelAllBtn->setEnabled(false);
            m_ui->CancelSelectedBtn->setEnabled(false);
            writeConsole("Queued process(s) is(are) done.");
        }

        return;
    }

    m_ui->TriggleStartBtn->setEnabled(false);
    m_mainProcess->setWorkingDirectory(m_ui->TaskQueueTable->item(0, 0)->text());

    //get process name and argument(s)
    QString processName(m_ui->TaskQueueTable->item(0, 1)->text());
    //check executable is exist or not
    if (QStandardPaths::findExecutable(processName) == "")
    {
        QFileInfo info(m_mainProcess->workingDirectory() + "/" + processName);
        if (!info.isFile() || !info.isExecutable())
        {
            writeConsole("Program is not exist!");
            m_ui->TriggleStartBtn->setEnabled(true);
            m_status = idle;
            return;
        }
    }

    QStringList args(m_ui->TaskQueueTable->item(0, 2)->text().split("\n",
                                                  QString::SkipEmptyParts));
    m_mainProcess->start(processName, args);
#ifndef Q_OS_WIN32
    setpgid(static_cast<int>(m_mainProcess->pid()), 0);
#endif
    m_ui->TaskQueueTable->item(0, 3)->setText("Running");
    m_ui->TaskQueueTable->item(0, 4)->setText("0");
    if (m_status == idle && m_ui->TaskQueueTable->rowCount() != 0)
    {
        m_status = running;
        m_ui->AbortBtn->setEnabled(true);
    }
}

void TaskTab::on_AbortBtn_clicked()
{
    QMessageBox::StandardButton res(
                QMessageBox::question(
                    this,
                    "Abort",
                    "Are you sure to abort?",
                    QMessageBox::Cancel |
                    QMessageBox::No |
                    QMessageBox::Yes));

    if (res == QMessageBox::Yes)
    {
        if (m_ui->TaskQueueTable->rowCount() == 0)
            m_ui->AbortBtn->setEnabled(false);

        killProcess();
    }
}

void TaskTab::on_CancelSelectedBtn_clicked()
{
    QItemSelectionModel *select = m_ui->TaskQueueTable->selectionModel();
    QModelIndexList selected = select->selectedRows();
    if (selected.size() > 0)
    {
        for (int i = 0; i < selected.count(); ++i)
        {
            int row = selected.at(i).row();
            if (m_status == running)
            {
                if (row != 0) m_ui->TaskQueueTable->removeRow(row);
            }
            else
            {
                m_ui->TaskQueueTable->removeRow(row);
            }
        }
    }

    if (m_ui->TaskQueueTable->rowCount() == 0)
    {
        m_ui->TriggleStartBtn->setEnabled(false);
        m_ui->CancelSelectedBtn->setEnabled(false);
        m_ui->CancelAllBtn->setEnabled(false);
    }
}

void TaskTab::on_CancelAllBtn_clicked()
{
    QMessageBox::StandardButton res(
                QMessageBox::question(
                    this,
                    "Abort",
                    "Are you sure to cancel all?",
                    QMessageBox::Cancel |
                    QMessageBox::No |
                    QMessageBox::Yes));

    if (res == QMessageBox::Yes)
    {
        m_ui->AbortBtn->setEnabled(false);
        m_ui->TriggleStartBtn->setEnabled(false);
        m_ui->CancelAllBtn->setEnabled(false);
        m_ui->CancelSelectedBtn->setEnabled(false);
        if (m_status == idle)
        {
            m_ui->TaskQueueTable->setRowCount(0);
        }
        else
        {
            for (int i = 1; i < m_ui->TaskQueueTable->rowCount(); ++i)
            {
                m_ui->TaskQueueTable->removeRow(1);
            }

            killProcess();
        }
    }
}

void TaskTab::on_CleanDoneBtn_clicked()
{
    m_ui->DoneList->setRowCount(0);
}

void TaskTab::on_consoleClearBtn_clicked()
{
    m_ui->console->clear();
}

void TaskTab::onProcessExited(int exitcode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus)
    QTableWidgetItem *dir, *name, *args, *exit;
    dir = getTableWidgetItem(m_ui->TaskQueueTable->item(0, 0)->text());
    name = getTableWidgetItem(m_ui->TaskQueueTable->item(0, 1)->text());
    args = getTableWidgetItem(m_ui->TaskQueueTable->item(0, 2)->text());
    exit = getTableWidgetItem(QString::number(exitcode));

    m_ui->DoneList->insertRow(m_ui->DoneList->rowCount());
    int last_row(m_ui->DoneList->rowCount() - 1);
    m_ui->DoneList->setItem(last_row, 0, dir);
    m_ui->DoneList->setItem(last_row, 1, name);
    m_ui->DoneList->setItem(last_row, 2, args);
    m_ui->DoneList->setItem(last_row, 3, exit);
    m_ui->TaskQueueTable->removeRow(0);
    m_ui->AbortBtn->setEnabled(false);
    on_TriggleStartBtn_clicked(); //這個函數會善後
}

void TaskTab::handle_process_stderr()
{
    writeConsole(m_mainProcess->readAllStandardError());
}

void TaskTab::handle_process_stdout()
{
    writeConsole(m_mainProcess->readAllStandardOutput());
}

void TaskTab::onInputAccepted()
{
    QString tmpString(m_inputDialog->getText());
    m_inputDialog->close();
    m_inputDialog->reset();
    emit renameRequried(this, tmpString);
}

// private member functions
void TaskTab::connectHook()
{
    connect(m_mainProcess,
            SIGNAL(finished(int, QProcess::ExitStatus)),
            this,
            SLOT(onProcessExited(int, QProcess::ExitStatus)));

    connect(m_mainProcess,
            SIGNAL(readyReadStandardError()),
            this,
            SLOT(handle_process_stderr()));

    connect(m_mainProcess,
            SIGNAL(readyReadStandardOutput()),
            this,
            SLOT(handle_process_stdout()));

    // InputDialog
    connect(m_inputDialog,
            SIGNAL(accepted()),
            this,
            SLOT(onInputAccepted()));

    connect(m_ui->renameBtn,
            SIGNAL(clicked()),
            m_inputDialog,
            SLOT(open()));
}

QTableWidgetItem *TaskTab::getTableWidgetItem(const QString &input)
{
    QTableWidgetItem *res = new QTableWidgetItem(input);
    res->setFlags(Qt::ItemIsSelectable |
                  Qt::ItemIsUserCheckable |
                  Qt::ItemIsEnabled);
    return res;
}

void TaskTab::writeConsole(const QByteArray &data)
{
    if (data.endsWith('\n'))
    {
#ifdef Q_OS_WIN32
        QByteArray input(data);
        input.replace("\r\n", "\n");
        m_ui->console->insertPlainText(input);
#else
        m_ui->console->insertPlainText(data);
#endif
    }
    else // \r
    {
        QByteArray input(data);
        input.replace("\r", "");
        if (m_prevData.endsWith('\r'))
        {
            QTextCursor cursor = m_ui->console->textCursor();
            cursor.movePosition(QTextCursor::Start);
            cursor.movePosition(QTextCursor::Down,
                                QTextCursor::MoveAnchor,
                                m_ui->console->document()->blockCount() - 1);
            cursor.select(QTextCursor::LineUnderCursor);
            cursor.removeSelectedText();
            m_ui->console->setTextCursor(cursor);
        }
        m_ui->console->insertPlainText(input);
    }

    m_ui->console->ensureCursorVisible();

    m_prevData = data;
}

void TaskTab::killProcess()
{
#ifdef Q_OS_WIN32
    PROCESSENTRY32 pe;

    memset(&pe, 0, sizeof(PROCESSENTRY32));
    pe.dwSize = sizeof(PROCESSENTRY32);
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (Process32First(hSnap, &pe))
    {
        BOOL bContinue = TRUE;

        // kill child processes
        while (bContinue)
        {
            // only kill child processes
            if (pe.th32ParentProcessID == m_mainProcess->processId())
            {
                HANDLE hChildProc = OpenProcess(PROCESS_ALL_ACCESS,
                                                FALSE,
                                                pe.th32ProcessID);

                if (hChildProc)
                {
                    TerminateProcess(hChildProc, 1);
                    CloseHandle(hChildProc);
                }
            }

            bContinue = Process32Next(hSnap, &pe);
        }

        // kill the main process
        HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS,
                                   FALSE,
                                   m_mainProcess->processId());

        if (hProc)
        {
            TerminateProcess(hProc, 1);
            CloseHandle(hProc);
        }
    }
#else
    kill(static_cast<int>(m_mainProcess->processId()) * -1, SIGKILL);
#endif
}
