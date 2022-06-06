set(MODEL_SRC
    model/grpccommon.cpp
    model/grpccommon.hpp

    model/global.cpp
    model/global.hpp
    model/mainmodel.cpp
    model/mainmodel.hpp
    model/queuelistmodel.cpp
    model/queuelistmodel.hpp
    model/settingsmodel.cpp
    model/settingsmodel.hpp
)

add_executable(STQClient WIN32
    main.cpp
    resources.qrc

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
