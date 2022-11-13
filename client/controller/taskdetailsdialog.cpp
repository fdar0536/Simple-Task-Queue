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

#include "QKeyEvent"

#include "taskdetailsdialog.hpp"
#include "../view/ui_taskdetailsdialog.h"

static std::vector<QString> priorityString =
{
    "High", "Abrove normal", "Normal", "Below normal", "Low"
};

TaskDetailsDialog::TaskDetailsDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(nullptr)
{
}

TaskDetailsDialog *TaskDetailsDialog::create(QWidget *parent)
{
    TaskDetailsDialog *ret(nullptr);

    try
    {
        ret = new TaskDetailsDialog(parent);
    }
    catch(...)
    {
        return nullptr;
    }

    try
    {
        ret->m_ui = new Ui::TaskDetailsDialog;
    }
    catch(...)
    {
        delete ret;
        return nullptr;
    }

    ret->m_ui->setupUi(ret);
    ret->connectHook();
    return ret;
}

TaskDetailsDialog::~TaskDetailsDialog()
{
    if (m_ui) delete m_ui;
}

void TaskDetailsDialog::openDialog(TaskDetails &task, bool isExitCodeValid)
{
    m_ui->workDir->setText(QString::fromStdString(task.workDir));
    m_ui->programName->setText(QString::fromStdString(task.programName));
    m_ui->args->clear();

    size_t size = task.args.size();
    QString args = "";
    if (size)
    {
        for (size_t i = 0; i < size; ++i)
        {
            args += QString::fromUtf8(task.args.at(i));
            args += '\n';
        }
    }

    if (isExitCodeValid)
    {
        m_ui->exitCode->setText(QString::number(task.exitCode));
        m_ui->priority->setText("Priority is not available.");
        m_ui->dlBtn->setEnabled(true);
    }
    else
    {
        m_ui->exitCode->setText("Exit code is not available.");
        m_ui->priority->setText(priorityString.at(task.priority));
        m_ui->dlBtn->setEnabled(false);
    }

    QDialog::open();
}

// protected member functions
void TaskDetailsDialog::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
    case Qt::Key_Escape:
    {
        reject();
        break;
    }
    default:
    {
        QDialog::keyPressEvent(e);
        break;
    }
    }
}

// private member functions
void TaskDetailsDialog::connectHook()
{
    connect(m_ui->closeBtn,
            &QPushButton::clicked,
            this,
            &TaskDetailsDialog::reject);
}
