if(ENABLE_CLI)
    set(FF_CLI_LIBS
        protobuf::libprotobuf
        gRPC::grpc++
        cxxopts::cxxopts
        SQLite::SQLite3

        grpc_common
        spdlog::spdlog
    )

    add_executable(FlexFlowCLI
        climain.cpp)

    add_dependencies(FlexFlowCLI grpc_common ffmodel ffcontroller)

    target_link_libraries(FlexFlowCLI
        PRIVATE

        ${FF_CLI_LIBS}
        ffmodel
        ffcontroller
    )

    target_include_directories(FlexFlowCLI PRIVATE ${INIPP_INCLUDE_DIRS})

    if (MSVC AND WIN32 AND NOT MSVC_VERSION VERSION_LESS 142)
        target_link_options(FlexFlowCLI PRIVATE $<$<CONFIG:Debug>:/INCREMENTAL>)
        target_compile_options(FlexFlowCLI PRIVATE $<$<CONFIG:Debug>:/ZI>)
    endif()
endif(ENABLE_SERVER)
