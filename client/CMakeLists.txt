set(MODEL_SRC
    model/grpccommon.cpp
    model/grpccommon.hpp

    model/global.cpp
    model/global.hpp
    model/donelistmodel.cpp
    model/donelistmodel.hpp
    model/fakeconsolemodel.cpp
    model/fakeconsolemodel.hpp
    model/pendinglistmodel.cpp
    model/pendinglistmodel.hpp
    model/queuelistmodel.cpp
    model/queuelistmodel.hpp
    model/settingsmodel.cpp
    model/settingsmodel.hpp
    model/taskdetails.cpp
    model/taskdetails.hpp
)

set(CONTROLLER_SRC
    controller/addtaskdialog.cpp
    controller/addtaskdialog.hpp
    controller/donelist.cpp
    controller/donelist.hpp
    controller/fakeconsole.cpp
    controller/fakeconsole.hpp
    controller/mainwindow.cpp
    controller/mainwindow.hpp
    controller/pendinglist.cpp
    controller/pendinglist.hpp
    controller/queuelist.cpp
    controller/queuelist.hpp
    controller/settings.cpp
    controller/settings.hpp
    controller/taskdetailsdialog.cpp
    controller/taskdetailsdialog.hpp
)

set(VIEW_SRC
    view/addtaskdialog.ui
    view/donelist.ui
    view/fakeconsole.ui
    view/mainwindow.ui
    view/pendinglist.ui
    view/queuelist.ui
    view/settings.ui
    view/taskdetailsdialog.ui
)

add_executable(STQClient WIN32
    main.cpp
    resources.qrc

    ${MODEL_SRC}
    ${VIEW_SRC}
    ${CONTROLLER_SRC}
)

add_dependencies(STQClient grpc_common)

target_link_libraries(STQClient
    PRIVATE
    grpc_common
    gRPC::grpc
    gRPC::grpc++
    Qt6::Widgets
)
