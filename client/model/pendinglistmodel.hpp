#pragma once

#include <atomic>
#include <vector>

#include "QThread"

#include "pending.grpc.pb.h"

#include "taskdetails.hpp"

class PendingListModel : public QThread
{
    Q_OBJECT

public:

    static PendingListModel *create(QObject * = nullptr);

    uint8_t hasError(bool &);

    uint8_t lastError(QString &);

    uint8_t startList();

    uint8_t pendingList(std::vector<uint32_t> &);

    uint8_t startDetails(uint32_t id);

    uint8_t taskDetails(TaskDetails &);

    uint8_t startCurrent();

    uint8_t startAdd(std::string &workDir,
                     std::string &programName,
                     std::vector<std::string> &args);

    uint8_t resID(uint32_t &);

    uint8_t startRemove(uint32_t);

    uint8_t startStart();

    uint8_t startStop();

    void run() override;

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

    void startStopImpl(uint8_t);

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

    std::atomic<bool> m_isRunning;

    std::vector<uint32_t> m_pendingList;

    TaskDetails m_taskDetailsReq;

    TaskDetails m_taskDetailsRes;

    uint32_t m_reqID;

    uint32_t m_resID;

    void reset();
};

