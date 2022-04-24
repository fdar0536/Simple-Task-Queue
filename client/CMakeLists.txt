add_executable(STQClient WIN32
    main.cpp
    ui/loggerdialog.cpp
    ui/loggerdialog.hpp
    ui/loggerdialog.ui
    ui/mainwindow.cpp
    ui/mainwindow.hpp
    ui/mainwindow.ui
    ui/settingsdialog.cpp
    ui/settingsdialog.hpp
    ui/settingsdialog.ui
)

target_link_libraries(STQClient PRIVATE Qt6::Widgets)
