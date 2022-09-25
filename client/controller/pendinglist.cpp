#include "QMessageBox"

#include "pendinglist.hpp"
#include "../view/ui_pendinglist.h"

PendingList *PendingList::create(QWidget *parent)
{
    PendingList *ret(nullptr);

    try
    {
        ret = new PendingList(parent);
        ret->m_ui = new Ui::PendingList;
    }
    catch(...)
    {
        if (ret) delete ret;
        return nullptr;
    }

    ret->m_global = Global::instance();
    if (ret->m_global == nullptr)
    {
        delete ret;
        return nullptr;
    }

    if (ret->m_global->taskDetailsDialog(ret->m_taskDetailsDialog))
    {
        delete ret;
        return nullptr;
    }

    ret->m_model = PendingListModel::create(ret);
    if (!ret->m_model)
    {
        delete ret;
        return nullptr;
    }

    ret->m_addTaskDialog = AddTaskDialog::create(ret->m_model, ret);
    if (!ret->m_addTaskDialog)
    {
        delete ret;
        return nullptr;
    }

    ret->m_ui->setupUi(ret);
    ret->connectHook();
    ret->m_taskDetailsDialog->setParent(ret);
    ret->on_refreshBtn_clicked();
    return ret;
}

PendingList::~PendingList()
{
    if (m_ui) delete m_ui;
    if (m_addTaskDialog) delete m_addTaskDialog;
    if (m_model) delete m_model;

    m_taskDetailsDialog->setParent(nullptr);
    m_global->freeTaskDetailsDialog();
}

// private slots
void PendingList::onModelErrorOccurred()
{
    QString err;
    if (m_model->lastError(err))
    {
        err = "Fail to get last error.";
    }

    QMessageBox::critical(this, "Error", err);
    m_ui->status->setText(err);
    setEnabled(true);
}

void PendingList::onModelDetailsDone()
{
    setEnabled(true);
    TaskDetails taskDetails;
    QString err;
    if (m_model->taskDetails(taskDetails))
    {
        err = "Fail to get task details.";
        QMessageBox::critical(this, "Error", err);
        m_ui->status->setText(err);
        return;
    }

    m_taskDetailsDialog->openDialog(taskDetails, false);
}

void PendingList::onModelListDone()
{
    QString err;
    if (m_model->pendingList(m_list))
    {
        err = "Fail to get pending list.";
        QMessageBox::critical(this, "Error", err);
        m_ui->status->setText(err);
        return;
    }

    if (m_model->taskDetails(m_taskDetails))
    {
        err = "Fail to get current task.";
        QMessageBox::critical(this, "Error", err);
        m_ui->status->setText(err);
        return;
    }

    if (m_list.size())
    {
        for (auto it = m_list.begin(); it != m_list.end(); ++it)
        {
            m_ui->pending->addItem(QString::number(*it));
        }
    }

    m_ui->current->setText(QString::fromStdString(m_taskDetails.programName));
    setEnabled(true);
}

void PendingList::onAddTaskDialogClosed()
{
    connect(m_model,
            &PendingListModel::errorOccurred,
            this,
            &PendingList::onModelErrorOccurred);
    on_refreshBtn_clicked();
}

void PendingList::on_currentDetail_clicked()
{
    m_taskDetailsDialog->openDialog(m_taskDetails, false);
}

void PendingList::on_pending_itemDoubleClicked(QListWidgetItem *item)
{
    if (!item) return;

    m_model->startDetails(item->text().toUInt());
    setEnabled(false);
}

void PendingList::on_addBtn_clicked()
{
    disconnect(m_model,
               &PendingListModel::errorOccurred,
               this,
               &PendingList::onModelErrorOccurred);
    m_addTaskDialog->open();
}

void PendingList::on_deleteBtn_clicked()
{
    auto list = m_ui->pending->selectedItems();
    if (list.empty()) return;

    std::vector<uint32_t> toDelete;
    size_t listSize(list.size());
    toDelete.reserve(listSize);
    for (size_t i = 0; i < listSize; ++i)
    {
        toDelete.push_back(list.at(i)->text().toUInt());
    }

    m_model->startRemove(toDelete);
    setEnabled(false);
}

void PendingList::on_refreshBtn_clicked()
{
    setEnabled(false);
    m_model->startList();
}

void PendingList::on_startBtn_clicked()
{
    if (!m_ui->pending->count()) return;
    m_model->startStart();
}

void PendingList::on_stopBtn_clicked()
{
    if (m_ui->current->text().isEmpty()) return;
    m_model->startStop();
}

// private number functions
PendingList::PendingList(QWidget *parent) :
    QWidget(parent),
    m_ui(nullptr),
    m_model(nullptr),
    m_addTaskDialog(nullptr)
{
    m_list.reserve(32);
}

void PendingList::connectHook()
{
    connect(m_model,
            &PendingListModel::errorOccurred,
            this,
            &PendingList::onModelErrorOccurred);
    connect(m_model,
            &PendingListModel::listDone,
            this,
            &PendingList::onModelListDone);
    connect(m_model,
            &PendingListModel::detailsDone,
            this,
            &PendingList::onModelDetailsDone);

    connect(m_addTaskDialog, &AddTaskDialog::accepted,
            this, &PendingList::onAddTaskDialogClosed);
    connect(m_addTaskDialog, &AddTaskDialog::rejected,
            this, &PendingList::onAddTaskDialogClosed);
}
