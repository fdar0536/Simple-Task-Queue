#include "../common.hpp"
#include "../global.hpp"

#include "accessimpl.hpp"

::grpc::Status AccessImpl::Echo(::grpc::ServerContext* context,
                                const ::stq::Empty* request,
                                ::stq::EchoRes* response)
{
    UNUSED(context);
    UNUSED(request);
    UNUSED(response);
    return ::grpc::Status::OK;
}

::grpc::Status AccessImpl::Stop(::grpc::ServerContext* context,
                                const ::stq::Empty* request,
                                ::stq::EchoRes* response)
{
    UNUSED(context);
    UNUSED(request);
    UNUSED(response);
    Global::logger.write(Logger::Info, "Shutdown request acknowledged");
    Global::exit_requested.set_value();
    return ::grpc::Status::OK;
}
