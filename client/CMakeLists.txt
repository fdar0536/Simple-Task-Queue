set(MODEL_SRC
    model/grpccommon.cpp
    model/grpccommon.hpp

    model/global.cpp
    model/global.hpp
    model/pendinglistmodel.cpp
    model/pendinglistmodel.hpp
    model/queuelistmodel.cpp
    model/queuelistmodel.hpp
    model/settingsmodel.cpp
    model/settingsmodel.hpp
)

set(UI_SRC
    controller/mainwindow.cpp
    controller/mainwindow.hpp
    view/mainwindow.ui
    controller/pendinglist.cpp
    controller/pendinglist.hpp
    view/pendinglist.ui
    controller/queuelist.cpp
    controller/queuelist.hpp
    view/queuelist.ui
    controller/settings.cpp
    controller/settings.hpp
    view/settings.ui
)

add_executable(STQClient WIN32
    main.cpp
    resources.qrc

    ${MODEL_SRC}
    ${UI_SRC}
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
