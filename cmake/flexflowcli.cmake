if(ENABLE_CLI)
    set(FF_CLI_LIBS
        protobuf::libprotobuf
        gRPC::grpc++
        cxxopts::cxxopts
        SQLite::SQLite3

        grpc_common
        spdlog::spdlog
    )

    set(CLI_CONTROLLER_SRC
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

    add_executable(FlexFlowCLI
        ${CLI_CONTROLLER_SRC}

        climain.cpp)

    add_dependencies(FlexFlowCLI grpc_common ffmodel)

    target_link_libraries(FlexFlowCLI
        PRIVATE

        ${FF_CLI_LIBS}
        ffmodel
    )

    target_include_directories(FlexFlowCLI PRIVATE ${INIPP_INCLUDE_DIRS})

    if (MSVC AND WIN32 AND NOT MSVC_VERSION VERSION_LESS 142)
        target_link_options(FlexFlowCLI PRIVATE $<$<CONFIG:Debug>:/INCREMENTAL>)
        target_compile_options(FlexFlowCLI PRIVATE $<$<CONFIG:Debug>:/ZI>)
    endif()
endif(ENABLE_SERVER)
