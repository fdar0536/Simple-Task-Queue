if(ENABLE_CLI)
    set(STQ_CLI_LIBS
        protobuf::libprotobuf
        gRPC::grpc++
        cxxopts::cxxopts
        SQLite::SQLite3

        grpc_common
        spdlog::spdlog
    )

    add_executable(STQCLI
        climain.cpp)

    add_dependencies(STQCLI grpc_common stqmodel stqcontroller)

    target_link_libraries(STQCLI
        PRIVATE

        ${STQ_CLI_LIBS}
        stqmodel
        stqcontroller
    )

    if (MSVC AND WIN32 AND NOT MSVC_VERSION VERSION_LESS 142)
        target_link_options(STQCLI PRIVATE $<$<CONFIG:Debug>:/INCREMENTAL>)
        target_compile_options(STQCLI PRIVATE $<$<CONFIG:Debug>:/ZI>)
    endif (MSVC AND WIN32 AND NOT MSVC_VERSION VERSION_LESS 142)
endif (ENABLE_CLI)
