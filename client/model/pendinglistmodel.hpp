#pragma once

#include <atomic>
#include <vector>

#include "QThread"

#include "pending.grpc.pb.h"

class PendingListModel : public QThread
{
    Q_OBJECT

public:

    static PendingListModel *create(QObject * = nullptr);

    uint8_t hasError(bool &);

    uint8_t lastError(QString &);

    uint8_t pendingList(std::vector<uint32_t> &);

    uint8_t startList();

    //void run() override;

signals:

    void done();

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

    QString m_queueName;

    bool m_hasError;

    QString m_lastError;

    std::vector<uint32_t> m_pendingList;

    std::atomic<bool> m_isRunning;
};

