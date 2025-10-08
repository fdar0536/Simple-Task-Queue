if(ENABLE_SERVER)
    set(FF_SERVER_LIBS
        protobuf::libprotobuf
        gRPC::grpc++
        cxxopts::cxxopts

        grpc_common
        spdlog::spdlog
        SQLite::SQLite3
        yaml-cpp::yaml-cpp
    )

    add_executable(FlexFlowServer
        servermain.cpp)

    add_dependencies(FlexFlowServer grpc_common ffmodel ffcontroller)

    target_link_libraries(FlexFlowServer
        PRIVATE

        ${FF_SERVER_LIBS}
        ffmodel
        ffcontroller
    )

    if (MSVC AND WIN32 AND NOT MSVC_VERSION VERSION_LESS 142)
        target_link_options(FlexFlowServer PRIVATE $<$<CONFIG:Debug>:/INCREMENTAL>)
        target_compile_options(FlexFlowServer PRIVATE $<$<CONFIG:Debug>:/ZI>)
    endif()
endif(ENABLE_SERVER)
