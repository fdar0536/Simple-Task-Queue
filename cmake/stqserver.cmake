if(ENABLE_SERVER)
    set(STQ_SERVER_LIBS
        protobuf::libprotobuf
        gRPC::grpc++
        cxxopts::cxxopts

        grpc_common
        spdlog::spdlog
        SQLite::SQLite3
        yaml-cpp::yaml-cpp
    )

    add_executable(STQSERVER
        servermain.cpp)

    add_dependencies(STQSERVER grpc_common stqmodel stqcontroller)

    target_link_libraries(STQSERVER
        PRIVATE

        ${STQ_SERVER_LIBS}
        stqmodel
        stqcontroller
    )

    if (MSVC AND WIN32 AND NOT MSVC_VERSION VERSION_LESS 142)
        target_link_options(STQSERVER PRIVATE $<$<CONFIG:Debug>:/INCREMENTAL>)
        target_compile_options(STQSERVER PRIVATE $<$<CONFIG:Debug>:/ZI>)
    endif()
endif(ENABLE_SERVER)
