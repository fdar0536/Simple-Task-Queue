#pragma once

#include <QThread>

class PendingListModel : public QThread
{
    Q_OBJECT

public:

    static PendingListModel *create(QObject * = nullptr);

private:

    PendingListModel(QObject *parent = nullptr);

};

