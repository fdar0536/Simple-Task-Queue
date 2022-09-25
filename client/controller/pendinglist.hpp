#pragma once

#include "QListWidgetItem"
#include "QWidget"

#include "addtaskdialog.hpp"
#include "model/global.hpp"
#include "model/pendinglistmodel.hpp"

namespace Ui
{
class PendingList;
}

class PendingList : public QWidget
{
    Q_OBJECT

public:

    static PendingList *create(QWidget * = nullptr);

    ~PendingList();

private slots:

    void onModelErrorOccurred();

    void onModelDetailsDone();

    void onModelListDone();

    void onAddTaskDialogClosed();

    void on_currentDetail_clicked();

    void on_pending_itemDoubleClicked(QListWidgetItem *item);

    void on_addBtn_clicked();

    void on_deleteBtn_clicked();

    void on_refreshBtn_clicked();

    void on_startBtn_clicked();

    void on_stopBtn_clicked();

private:

    PendingList(QWidget * = nullptr);

    Ui::PendingList *m_ui;

    PendingListModel *m_model;

    AddTaskDialog *m_addTaskDialog;

    TaskDetails m_taskDetails;

    std::vector<uint32_t> m_list;

    std::shared_ptr<Global> m_global;

    std::shared_ptr<TaskDetailsDialog> m_taskDetailsDialog;

    void connectHook();
};

