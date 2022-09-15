#include "global.hpp"
#include "grpccommon.hpp"

//#include "pendinglistmodel.hpp"

/*
// public member function
PendingListModel *PendingListModel::create(QObject *parent)
{
    std::shared_ptr<Global> global = Global::instance();
    if (global == nullptr)
    {
        return nullptr;
    }

    PendingListModel *ret(nullptr);
    try
    {
        ret = new PendingListModel(parent);
    }
    catch(...)
    {
        return nullptr;
    }

    try
    {
        ret->m_stub = stq::Pending::NewStub(global->grpcChannel());
        if (ret->m_stub == nullptr)
        {
            delete ret;
            return nullptr;
        }
    }
    catch (...)
    {
        delete ret;
        return nullptr;
    }

    if (GrpcCommon::getQueueName(global, ret->m_queueName))
    {
        delete ret;
        return nullptr;
    }

    return ret;
}

uint8_t PendingListModel::hasError(bool &out)
{
    if (m_isRunning.load(std::memory_order_relaxed)) return 1;
    out = m_hasError;
    return 0;
}

uint8_t PendingListModel::lastError(QString &out)
{
    if (m_isRunning.load(std::memory_order_relaxed)) return 1;
    out = m_lastError;
    return 0;
}

uint8_t PendingListModel::pendingList(std::vector<uint32_t> &out)
{
    if (m_isRunning.load(std::memory_order_relaxed)) return 1;
    out = m_pendingList;
    return 0;
}

uint8_t PendingListModel::startList()
{
    if (m_isRunning.load(std::memory_order_relaxed)) return 1;
    m_func = List;
    start();
    return 0;
}

void PendingListModel::run()
{
    (this->*m_handler[m_func])();
}

// private member functions
PendingListModel::PendingListModel(QObject *parent) :
    QThread(parent),
    m_queueName(""),
    m_hasError(false),
    m_lastError("")
{
    m_isRunning.store(false, std::memory_order_relaxed);
}

void PendingListModel::listImpl()
{

}
*/
