#pragma once

#include "QDialog"

#include "model/pendinglistmodel.hpp"

namespace Ui
{
class AddTaskDialog;
}

class AddTaskDialog : public QDialog
{
    Q_OBJECT

public:

    static AddTaskDialog *create(PendingListModel *, QWidget * = nullptr);

    ~AddTaskDialog();

    void accept() override;

    void done(int) override;

    void open() override;

    void reject() override;

protected:

    void keyPressEvent(QKeyEvent *) override;

private slots:

    void onModelErrorOccurred();

    void onModelAddDone();

    void on_workDir_textChanged(const QString &);

    void on_programName_textChanged(const QString &);

    void on_addBtn_clicked();

    void on_clearBtn_clicked();

    void on_closeBtn_clicked();

private:

    AddTaskDialog(QWidget * = nullptr);

    Ui::AddTaskDialog *m_ui;

    PendingListModel *m_model;

    void handleCloseWindow();

    void checkInputText();
};

