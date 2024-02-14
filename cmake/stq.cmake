set(CONTROLLER_SRC

    # global
    controller/global/defines.hpp

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

set(STQ_LIBS
    protobuf::libprotobuf
    gRPC::grpc++

    grpc_common
    spdlog::spdlog
)

if(ENABLE_SERVER)
    add_executable(STQSERVER
        ${MODEL_SRC}
        ${CONTROLLER_SRC}

        servermain.cpp)

    add_dependencies(STQSERVER grpc_common)

    target_link_libraries(STQSERVER
        PRIVATE

        ${STQ_LIBS}
        SQLite::SQLite3
    )

    target_include_directories(STQSERVER PRIVATE ${INIPP_INCLUDE_DIRS})

    if (MSVC AND WIN32 AND NOT MSVC_VERSION VERSION_LESS 142)
        target_link_options(STQSERVER PRIVATE $<$<CONFIG:Debug>:/INCREMENTAL>)
        target_compile_options(STQSERVER PRIVATE $<$<CONFIG:Debug>:/ZI>)
    endif()
endif(ENABLE_SERVER)
