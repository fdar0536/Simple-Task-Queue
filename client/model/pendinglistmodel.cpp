#include "global.hpp"
#include "grpccommon.hpp"

#include "pendinglistmodel.hpp"

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

uint8_t PendingListModel::lastError(QString &out)
{
    if (m_isRunning.load(std::memory_order_relaxed)) return 1;
    out = m_lastError;
    return 0;
}

uint8_t PendingListModel::startList()
{
    if (m_isRunning.load(std::memory_order_relaxed)) return 1;
    m_func = List;
    start();
    return 0;
}

uint8_t PendingListModel::pendingList(std::vector<uint32_t> &out)
{
    if (m_isRunning.load(std::memory_order_relaxed)) return 1;
    out = m_pendingList;
    return 0;
}

uint8_t PendingListModel::startDetails(uint32_t id)
{
    if (m_isRunning.load(std::memory_order_relaxed)) return 1;

    m_reqID = id;
    m_func = Details;
    start();
    return 0;
}

uint8_t PendingListModel::taskDetails(TaskDetails &out)
{
    if (m_isRunning.load(std::memory_order_relaxed)) return 1;

    out = m_taskDetailsRes;
    return 0;
}

uint8_t PendingListModel::startCurrent()
{
    if (m_isRunning.load(std::memory_order_relaxed)) return 1;
    m_func = Current;
    start();
    return 0;
}

uint8_t PendingListModel::startAdd(const std::string &workDir,
                                   const std::string &programName,
                                   const std::vector<std::string> &args,
                                   const uint32_t priority)
{
    if (m_isRunning.load(std::memory_order_relaxed)) return 1;

    m_taskDetailsReq.workDir = workDir;
    m_taskDetailsReq.programName = programName;

    m_taskDetailsReq.args.clear();
    m_taskDetailsReq.args.reserve(args.size());
    std::copy(args.begin(),
              args.end(),
              m_taskDetailsReq.args.begin());
    m_taskDetailsReq.priority = priority;
    m_func = Add;
    start();
    return 0;

    return 0;
}

uint8_t PendingListModel::resID(uint32_t &out)
{
    if (m_isRunning.load(std::memory_order_relaxed)) return 1;

    out = m_resID;
    return 0;
}

uint8_t PendingListModel::startRemove(std::vector<uint32_t> &list)
{
    if (m_isRunning.load(std::memory_order_relaxed)) return 1;

    if (list.empty()) return 1;

    m_toRemove = list;
    m_func = Remove;
    start();
    return 0;
}

uint8_t PendingListModel::startStart()
{
    if (m_isRunning.load(std::memory_order_relaxed)) return 1;

    m_func = Start;
    start();
    return 0;
}

uint8_t PendingListModel::startStop()
{
    if (m_isRunning.load(std::memory_order_relaxed)) return 1;

    m_func = Stop;
    start();
    return 0;
}

void PendingListModel::run()
{
    m_isRunning.store(true, std::memory_order_relaxed);
    reset();
    (this->*m_handler[m_func])();
}

// private member functions
PendingListModel::PendingListModel(QObject *parent) :
    QThread(parent),
    m_queueName("")
{
    m_isRunning.store(false, std::memory_order_relaxed);
    m_pendingList.reserve(128);
    m_toRemove.reserve(128);
    m_taskDetailsRes.args.reserve(128);
}

void PendingListModel::listImpl()
{
    stq::QueueReq req;
    req.set_name(m_queueName.toLocal8Bit().toStdString());

    grpc::ClientContext ctx;
    stq::ListTaskRes res;

    GrpcCommon::setupCtx(ctx);
    auto reader = m_stub->List(&ctx, req);
    while(reader->Read(&res))
    {
        m_pendingList.push_back(res.id());
    }

    grpc::Status status = reader->Finish();
    if (status.ok())
    {
        currentImpl();
        return;
    }

    GrpcCommon::buildErrMsg(status, m_lastError);
    m_isRunning.store(false, std::memory_order_relaxed);
    emit errorOccurred();
}

void PendingListModel::detailsImpl()
{
    stq::TaskDetailsReq req;
    req.set_queuename(m_queueName.toLocal8Bit().toStdString());
    req.set_id(m_reqID);

    grpc::ClientContext ctx;
    stq::TaskDetailsRes res;

    GrpcCommon::setupCtx(ctx);
    grpc::Status status = m_stub->Details(&ctx, req, &res);
    if (status.ok())
    {
        GrpcCommon::buildTaskDetails(res, m_taskDetailsRes);
        m_isRunning.store(false, std::memory_order_relaxed);
        emit detailsDone();
        return;
    }

    GrpcCommon::buildErrMsg(status, m_lastError);
    m_isRunning.store(false, std::memory_order_relaxed);
    emit errorOccurred();
}

void PendingListModel::currentImpl()
{
    stq::QueueReq req;
    req.set_name(m_queueName.toLocal8Bit().toStdString());

    grpc::ClientContext ctx;
    stq::TaskDetailsRes res;

    GrpcCommon::setupCtx(ctx);
    grpc::Status status = m_stub->Current(&ctx, req, &res);
    if (status.ok())
    {
        GrpcCommon::buildTaskDetails(res, m_taskDetailsRes);
        m_isRunning.store(false, std::memory_order_relaxed);
        emit listDone();
        return;
    }

    GrpcCommon::buildErrMsg(status, m_lastError);
    m_isRunning.store(false, std::memory_order_relaxed);
    emit errorOccurred();
}

void PendingListModel::addImpl()
{
    stq::AddTaskReq req;
    req.set_queuename(m_queueName.toLocal8Bit().toStdString());
    req.set_workdir(m_taskDetailsReq.workDir);
    req.set_programname(m_taskDetailsReq.programName);
    for (auto it = m_taskDetailsReq.args.begin();
         it != m_taskDetailsReq.args.end();
         ++it)
    {
        req.add_args(*it);
    }

    req.set_priority(m_taskDetailsReq.priority);

    grpc::ClientContext ctx;
    stq::ListTaskRes res;

    GrpcCommon::setupCtx(ctx);
    grpc::Status status = m_stub->Add(&ctx, req, &res);
    if (status.ok())
    {
        m_resID = res.id();
        m_isRunning.store(false, std::memory_order_relaxed);
        emit addDone();
        return;
    }

    GrpcCommon::buildErrMsg(status, m_lastError);
    m_isRunning.store(false, std::memory_order_relaxed);
    emit errorOccurred();
}

void PendingListModel::removeImpl()
{
    stq::TaskDetailsReq req;
    req.set_queuename(m_queueName.toLocal8Bit().toStdString());

    grpc::ClientContext ctx;
    stq::Empty res;

    GrpcCommon::setupCtx(ctx);
    grpc::Status status;
    for (size_t i = 0; i < m_toRemove.size(); ++i)
    {
        req.set_id(m_toRemove.at(i));
        status = m_stub->Remove(&ctx, req, &res);

        if (!status.ok())
        {
            GrpcCommon::buildErrMsg(status, m_lastError);
            m_isRunning.store(false, std::memory_order_relaxed);
            emit errorOccurred();
            return;
        }
    }

    listImpl();
}

void PendingListModel::startImpl()
{
    startStopImpl(1);
}

void PendingListModel::stopImpl()
{
    startStopImpl(0);
}

void PendingListModel::startStopImpl(uint8_t shouldStart)
{
    stq::QueueReq req;
    req.set_name(m_queueName.toLocal8Bit().toStdString());

    grpc::ClientContext ctx;
    stq::Empty res;

    GrpcCommon::setupCtx(ctx);
    grpc::Status status;
    if (shouldStart)
        status = m_stub->Start(&ctx, req, &res);
    else
        status = m_stub->Stop(&ctx, req, &res);

    if (status.ok())
    {
        listImpl();
        return;
    }

    GrpcCommon::buildErrMsg(status, m_lastError);
    m_isRunning.store(false, std::memory_order_relaxed);
    emit errorOccurred();
}

void PendingListModel::reset()
{
    m_lastError = "";
    m_pendingList.clear();

    // reset task details
    m_taskDetailsRes.workDir = "";
    m_taskDetailsRes.programName = "";
    m_taskDetailsRes.args.clear();
    m_taskDetailsRes.exitCode = 0;
    m_taskDetailsRes.priority = 2;
}
