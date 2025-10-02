set(STQ_CONTROLLER_LIBS
    protobuf::libprotobuf
    gRPC::grpc++
    SQLite::SQLite3
    spdlog::spdlog

    grpc_common
    stqmodel
)

set(CONTROLLER_SRC
    # global
    controller/global/defines.hpp
    controller/global/global.cpp
    controller/global/global.hpp
)

if (ENABLE_SERVER)
    list(APPEND CONTROLLER_SRC
        #config
        controller/grpcserver/config.cpp
        controller/grpcserver/config.hpp
        controller/grpcserver/init.cpp
        controller/grpcserver/init.hpp

        # grpc
        controller/grpcserver/accessimpl.cpp
        controller/grpcserver/accessimpl.hpp
        controller/grpcserver/queueimpl.cpp
        controller/grpcserver/queueimpl.hpp
        controller/grpcserver/queuelistimpl.cpp
        controller/grpcserver/queuelistimpl.hpp
        controller/grpcserver/server.cpp
        controller/grpcserver/server.hpp
    )
endif (ENABLE_SERVER)

if (ENABLE_CLI)
    list(APPEND CONTROLLER_SRC
        controller/cli/args.cpp
        controller/cli/args.hpp
        controller/cli/config.hpp
        controller/cli/global.cpp
        controller/cli/global.hpp
        controller/cli/main.cpp
        controller/cli/main.hpp
        controller/cli/queue.cpp
        controller/cli/queue.hpp
        controller/cli/queuelist.cpp
        controller/cli/queuelist.hpp
    )
endif (ENABLE_CLI)

if (ENABLE_GUI)
    list(APPEND CONTROLLER_SRC
        controller/cli/main.cpp
        controller/cli/main.hpp
    )
endif (ENABLE_GUI)

add_library(stqcontroller STATIC
    ${CONTROLLER_SRC}
)

add_dependencies(stqcontroller stqmodel grpc_common)

target_link_libraries(stqcontroller
    PRIVATE

    ${STQ_CONTROLLER_LIBS}
)

