set(CONTROLLER_SRC

    # global
    controller/global/defines.hpp
    controller/global/config.cpp
    controller/global/config.hpp
    controller/global/init.cpp
    controller/global/init.hpp

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

if(ENABLE_GUI)

    list(APPEND CONTROLLER_SRC

        "controller/gui/global.cpp"
        "controller/gui/global.hpp"
        "controller/gui/logsink.hpp"
        "controller/gui/mainwindow.cpp"
        "controller/gui/mainwindow.hpp"
    )

    set(VIEW_SRC
        "view/gui/mainwindow.ui"
    )

    qt_add_executable(STQ
        WIN32
        MACOSX_BUNDLE

        ${MODEL_SRC}
        ${CONTROLLER_SRC}
        ${VIEW_SRC}

        main.cpp)

    add_dependencies(STQ grpc_common)

    list(APPEND STQ_LIBS

        Qt6::Quick
        Qt6::QuickControls2
    )

    target_link_libraries(STQ
        PRIVATE

        ${STQ_LIBS}
        SQLite::SQLite3

        # Qt
        Qt6::Widgets
    )
else()
    add_executable(STQ
        ${MODEL_SRC}
        ${CONTROLLER_SRC}

        main.cpp)

    add_dependencies(STQ grpc_common)

    target_link_libraries(STQ
        PRIVATE

        ${STQ_LIBS}
        SQLite::SQLite3
    )
endif(ENABLE_GUI)

target_include_directories(STQ PRIVATE ${INIPP_INCLUDE_DIRS})

if (MSVC AND WIN32 AND NOT MSVC_VERSION VERSION_LESS 142)
    target_link_options(STQ PRIVATE $<$<CONFIG:Debug>:/INCREMENTAL>)
    target_compile_options(STQ PRIVATE $<$<CONFIG:Debug>:/ZI>)
endif()
