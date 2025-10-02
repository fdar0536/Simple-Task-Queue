if(ENABLE_GUI)
    set(STQ_GUI_LIBS
        protobuf::libprotobuf
        gRPC::grpc++
        cxxopts::cxxopts

        grpc_common
        spdlog::spdlog
        SQLite::SQLite3
        yaml-cpp::yaml-cpp

        Qt6::Widgets
        Qt6::Quick
    )

    add_executable(STQGUI
        guimain.cpp)

    add_dependencies(STQGUI grpc_common stqmodel stqcontroller)

    target_link_libraries(STQGUI
        PRIVATE

        ${STQ_GUI_LIBS}
        stqmodel
        stqcontroller
    )

    if (MSVC AND WIN32 AND NOT MSVC_VERSION VERSION_LESS 142)
        target_link_options(STQGUI PRIVATE $<$<CONFIG:Debug>:/INCREMENTAL>)
        target_compile_options(STQGUI PRIVATE $<$<CONFIG:Debug>:/ZI>)
    endif (MSVC AND WIN32 AND NOT MSVC_VERSION VERSION_LESS 142)
endif (ENABLE_GUI)
