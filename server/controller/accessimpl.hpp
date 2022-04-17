#pragma once

#include "access.grpc.pb.h"

class AccessImpl: public stq::Access::Service
{
public:

    ::grpc::Status Echo(::grpc::ServerContext* context,
                        const ::stq::Empty* request,
                        ::stq::EchoRes* response) override;
};

