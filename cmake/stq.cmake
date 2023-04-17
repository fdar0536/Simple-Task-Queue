set(CONTROLLER_SRC

    # global
    controller/global/defines.hpp
    controller/global/config.cpp
    controller/global/config.hpp
    controller/global/init.cpp
    controller/global/init.hpp
)

if (NOT ENABLE_MOBILE)
    list(APPEND CONTROLLER_SRC

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
endif (NOT ENABLE_MOBILE)

set(STQ_LIBS
    protobuf::libprotobuf
    gRPC::grpc++

    grpc_common
    spdlog::spdlog
)

if(ENABLE_GUI)

    list(APPEND CONTROLLER_SRC

        "controller/gui/config.cpp"
        "controller/gui/config.hpp"
        "controller/gui/global.cpp"
        "controller/gui/global.hpp"
        "controller/gui/logsink.hpp"
        "controller/gui/main.cpp"
        "controller/gui/main.hpp"
    )

    qt_add_executable(STQ
        WIN32
        MACOSX_BUNDLE

        ${MODEL_SRC}
        ${CONTROLLER_SRC}

        main.cpp)

    qt_add_qml_module(STQ
        URI "stq"
        NO_RESOURCE_TARGET_PATH

        QML_FILES
            "view/gui/main.qml"

            # components
            "view/gui/components/MainMenu.qml"
            "view/gui/components/MenuBar.qml"
            "view/gui/components/TitleText.qml"
            "view/gui/components/ToolbarIcon.qml"
            "view/gui/components/ToolTipButton.qml"

            # pages
            "view/gui/pages/Config.qml"
            "view/gui/pages/Logger.qml"

        RESOURCES
            "view/gui/stq.ico"

            # icons
            "view/gui/icons/close.svg"
            "view/gui/icons/computer.svg"
            "view/gui/icons/event.svg"
            "view/gui/icons/info.svg"
            "view/gui/icons/menu.svg"
    )

    add_dependencies(STQ grpc_common)

    list(APPEND STQ_LIBS

        Qt6::Quick
        Qt6::QuickControls2
    )

    if(ENABLE_MOBILE)
        target_link_libraries(STQ
            PRIVATE

            ${STQ_LIBS}
        )
    else()
        target_link_libraries(STQ
            PRIVATE

            ${STQ_LIBS}
            SQLite::SQLite3

            # Qt
            Qt6::Widgets
        )
    endif()
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
