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
        controller/grpcserver/server.cpp
        controller/grpcserver/server.hpp
    )
endif (NOT ENABLE_MOBILE)

if(ENABLE_GUI)

    list(APPEND CONTROLLER_SRC

        # global
        "controller/gui/global.cpp"
        "controller/gui/global.hpp"
    )

    qt_add_executable(STQ
        WIN32
        MACOSX_BUNDLE

        ${CONTROLLER_SRC}

        stq.cpp)

    qt_add_qml_module(STQ
        URI stq
        NO_RESOURCE_TARGET_PATH

        QML_FILES
            "view/gui/stq.qml"
        RESOURCES
            "view/gui/stq.ico"
    )

    add_dependencies(STQ grpc_common STQModel)

    if(ENABLE_MOBILE)
        target_link_libraries(STQ
            PRIVATE

            STQModel
            grpc_common
            spdlog::spdlog
            rapidjson

            Qt6::Gui
            Qt6::Quick
        )
    else()
        target_link_libraries(STQ
            PRIVATE

            STQModel
            grpc_common
            spdlog::spdlog
            SQLite::SQLite3
            rapidjson

            # Qt
            Qt6::Widgets
            Qt6::Quick
        )
    endif()
else()
    add_executable(STQ
        ${CONTROLLER_SRC}

        stq.cpp)

    add_dependencies(STQ grpc_common STQModel)

    target_link_libraries(STQ
        PRIVATE

        STQModel
        grpc_common
        spdlog::spdlog
        SQLite::SQLite3
        rapidjson
    )
endif(ENABLE_GUI)

if (MSVC AND WIN32 AND NOT MSVC_VERSION VERSION_LESS 142)
    target_link_options(STQ PRIVATE $<$<CONFIG:Debug>:/INCREMENTAL>)
    target_compile_options(STQ PRIVATE $<$<CONFIG:Debug>:/ZI>)
endif()
