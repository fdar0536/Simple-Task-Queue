# all source for gui are depend on Qt
# so sperate controller source into gui

if(ENABLE_GUI)
    set(FF_GUI_LIBS
        protobuf::libprotobuf
        gRPC::grpc++
        cxxopts::cxxopts

        grpc_common
        spdlog::spdlog
        SQLite::SQLite3
        yaml-cpp::yaml-cpp

        Qt6::Widgets
    )

    set(GUI_SRC
        guimain.cpp
        guimain.qrc

        controller/gui/config.cpp
        controller/gui/config.hpp
        controller/gui/global.cpp
        controller/gui/global.hpp
        controller/gui/hostinfo.hpp
        controller/gui/main.cpp
        controller/gui/main.hpp

        view/configform.cpp
        view/configform.hpp
        view/configform.ui
        view/mainwindow.cpp
        view/mainwindow.hpp
        view/mainwindow.ui
    )

    qt_add_executable(FlexFlowGUI
        ${GUI_SRC}
    )

    add_dependencies(FlexFlowGUI grpc_common ffmodel ffcontroller)

    target_link_libraries(FlexFlowGUI
        PRIVATE

        ${FF_GUI_LIBS}
        ffmodel
        ffcontroller
    )

    if (MSVC AND WIN32 AND NOT MSVC_VERSION VERSION_LESS 142)
        target_link_options(FlexFlowGUI PRIVATE $<$<CONFIG:Debug>:/INCREMENTAL>)
        target_compile_options(FlexFlowGUI PRIVATE $<$<CONFIG:Debug>:/ZI>)
    endif (MSVC AND WIN32 AND NOT MSVC_VERSION VERSION_LESS 142)
endif (ENABLE_GUI)
