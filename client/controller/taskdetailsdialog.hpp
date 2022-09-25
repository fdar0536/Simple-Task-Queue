#pragma once

#include "QDialog"

#include "model/taskdetails.hpp"

namespace Ui {
class TaskDetailsDialog;
}

class TaskDetailsDialog : public QDialog
{

    Q_OBJECT

public:

    TaskDetailsDialog(QWidget * = nullptr);

    static std::shared_ptr<TaskDetailsDialog> create();

    ~TaskDetailsDialog();

    void openDialog(TaskDetails &, bool);

protected:

    void keyPressEvent(QKeyEvent *) override;

private slots:

    void on_closeBtn_clicked();

private:

    Ui::TaskDetailsDialog *m_ui;
};

