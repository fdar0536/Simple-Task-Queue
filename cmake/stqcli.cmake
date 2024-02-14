if(ENABLE_CLI)
    set(STQ_CLI_LIBS
        protobuf::libprotobuf
        gRPC::grpc++

        grpc_common
        spdlog::spdlog
    )

    if (ENABLE_SERVER)
        list(APPEND STQ_CLI_LIBS
            SQLite::SQLite3
        )
    endif (ENABLE_SERVER)

    set(CLI_CONTROLLER_SRC

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

    add_executable(STQCLI
        ${CLI_CONTROLLER_SRC}

        climain.cpp)

    add_dependencies(STQCLI grpc_common stqmodel)

    target_link_libraries(STQCLI
        PRIVATE

        ${STQ_CLI_LIBS}
        stqmodel
    )

    target_include_directories(STQCLI PRIVATE ${INIPP_INCLUDE_DIRS})

    if (MSVC AND WIN32 AND NOT MSVC_VERSION VERSION_LESS 142)
        target_link_options(STQCLI PRIVATE $<$<CONFIG:Debug>:/INCREMENTAL>)
        target_compile_options(STQCLI PRIVATE $<$<CONFIG:Debug>:/ZI>)
    endif()
endif(ENABLE_SERVER)
