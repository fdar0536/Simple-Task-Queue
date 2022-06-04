set(UI_SRC
    ui/loggerdialog.cpp
    ui/loggerdialog.hpp
    ui/loggerdialog.ui
    ui/mainwindow.cpp
    ui/mainwindow.hpp
    ui/mainwindow.ui
    ui/saveconfigdialog.cpp
    ui/saveconfigdialog.hpp
    ui/saveconfigdialog.ui
    ui/settingsdata.hpp
    ui/settingsdialog.cpp
    ui/settingsdialog.hpp
    ui/settingsdialog.ui
)

set(MODEL_SRC
    model/abstractclient.cpp
    model/abstractclient.hpp
    model/accessimpl.cpp
    model/accessimpl.hpp
    model/grpcchannelcreator.cpp
    model/grpcchannelcreator.hpp
    model/grpccommon.cpp
    model/grpccommon.hpp

    model/global.cpp
    model/global.hpp
)

add_executable(STQClient WIN32
    main.cpp
    resources.qrc

    ${UI_SRC}
    ${MODEL_SRC}
)

add_dependencies(STQClient grpc_common)

target_link_libraries(STQClient
    PRIVATE
    grpc_common
    gRPC::grpc
    gRPC::grpc++
    Qt6::Widgets
    Qt6::Quick
    Qt6::Qml
    Qt6::QuickControls2
)
