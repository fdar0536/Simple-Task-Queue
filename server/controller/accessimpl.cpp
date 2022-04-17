#include "../common.hpp"

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
