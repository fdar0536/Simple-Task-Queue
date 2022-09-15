#pragma once

#include <QWidget>

namespace Ui {
class PendingList;
}

class PendingList : public QWidget
{
    Q_OBJECT

public:

    PendingList *create(QWidget * = nullptr);

    ~PendingList();

private:

    PendingList(QWidget * = nullptr);

    Ui::PendingList *ui;
};

