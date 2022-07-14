#pragma once

#include "QThread"

#include "pending.grpc.pb.h"

class PendingListModel : public QThread
{
    Q_OBJECT

public:

    static PendingListModel *create(QObject * = nullptr);

private:

    PendingListModel(QObject *parent = nullptr);

    typedef enum funcs
    {
        List, Details, Current, Add, Remove, Start, Stop
    } funcs;

    funcs m_func;

    std::unique_ptr<stq::Pending::Stub> m_stub;

    typedef void (PendingListModel::*Handler)();

    void listImpl();

    void detailsImpl();

    void currentImpl();

    void addImpl();

    void removeImpl();

    void startImpl();

    void stopImpl();

    Handler m_handler[7] =
    {
        &PendingListModel::listImpl,
        &PendingListModel::detailsImpl,
        &PendingListModel::currentImpl,
        &PendingListModel::addImpl,
        &PendingListModel::removeImpl,
        &PendingListModel::startImpl,
        &PendingListModel::stopImpl
    };
};

