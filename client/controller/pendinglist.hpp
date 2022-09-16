#pragma once

#include "QWidget"

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

private:

    PendingList(QWidget * = nullptr);

    Ui::PendingList *m_ui;

    PendingListModel *m_model;
};

