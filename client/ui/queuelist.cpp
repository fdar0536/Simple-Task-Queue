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

#include "queuelist.hpp"
#include "ui_queuelist.h"

QueueList *QueueList::create(QWidget *parent)
{
    QueueList *ret(nullptr);

    try
    {
        ret = new QueueList(parent);
    }
    catch (...)
    {
        return nullptr;
    }

    try
    {
        ret->m_ui = new Ui::QueueList;
    }
    catch (...)
    {
        delete ret;
        return nullptr;
    }

    ret->m_ui->setupUi(ret);
    ret->m_global = Global::instance();
    if (ret->m_global == nullptr)
    {
        delete ret;
        return nullptr;
    }

    ret->m_model = QueueListModel::create(ret);
    if (!ret->m_model)
    {
        delete ret;
        return nullptr;
    }

    ret->connect(ret->m_model,
                 &QueueListModel::done,
                 ret,
                 &QueueList::onModelDone);

    QHash<QString, QVariant> state;
    if (ret->m_global->state("queueListState", state))
    {
        ret->on_refreshBtn_clicked();
        return ret;
    }

    QStringList list = state["list"].toStringList();
    if (!list.size()) return ret;

    ret->m_ui->queueList->insertItems(0, list);
    ret->m_ui->queueList->setCurrentIndex(state["index"].toInt());
    ret->m_ui->deleteBtn->setEnabled(true);
    return ret;
}

QueueList::~QueueList()
{
    if (m_ui->queueList->count())
    {
        QHash<QString, QVariant> state;
        state["index"] = m_ui->queueList->currentIndex();
        QStringList list;
        list.reserve(m_ui->queueList->count());
        for (int i = 0; i < m_ui->queueList->count(); ++i)
        {
            m_ui->queueList->setCurrentIndex(i);
            list.append(m_ui->queueList->currentText());
        }

        state["list"] = list;
        m_global->setState("queueListState", state);
    }

    if (m_ui) delete m_ui;
    if (m_model) delete m_model;
}

// private slots
void QueueList::onModelDone()
{
    m_ui->queueList->clear();
    setEnabled(true);
    if (m_model->hasError())
    {
        m_ui->status->setText(m_model->lastError());
        m_ui->deleteBtn->setEnabled(false);
        on_newName_editingFinished();
        return;
    }

    m_ui->status->setText("Done");
    m_ui->queueList->insertItems(0, m_model->result());
    m_ui->queueList->setCurrentIndex(0);
    m_ui->deleteBtn->setEnabled(true);
    on_newName_editingFinished();
}

void QueueList::on_deleteBtn_clicked()
{
    setEnabled(false);
    m_model->startDelete(m_ui->queueList->currentText());
}

void QueueList::on_refreshBtn_clicked()
{
    setEnabled(false);
    m_model->startList();
}

void QueueList::on_newName_editingFinished()
{
    bool res(!m_ui->newName->text().isEmpty());
    m_ui->renameBtn->setEnabled(res);
    m_ui->createBtn->setEnabled(res);
}

void QueueList::on_createBtn_clicked()
{
    setEnabled(false);
    m_model->startCreate(m_ui->newName->text());
}


void QueueList::on_renameBtn_clicked()
{
    setEnabled(false);
    m_model->startRename(m_ui->queueList->currentText(),
                         m_ui->newName->text());
}

// private member functions
QueueList::QueueList(QWidget *parent) :
    QWidget(parent),
    m_ui(nullptr),
    m_global(nullptr),
    m_model(nullptr)
{}
