# all source for gui are depend on Qt
# so sperate controller source into gui

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

    set(GUI_SRC
        guimain.cpp

        controller/gui/config.cpp
        controller/gui/config.hpp
        controller/gui/hostinfo.hpp
        controller/gui/main.cpp
        controller/gui/main.hpp
    )

    qt_add_executable(STQGUI
        ${GUI_SRC}
    )

    add_dependencies(STQGUI grpc_common stqmodel stqcontroller)

    qt_add_qml_module(STQGUI
        URI stq
        QML_FILES
            view/main.qml
    )

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
