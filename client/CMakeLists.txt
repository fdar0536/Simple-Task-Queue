add_executable(STQClient WIN32
    main.cpp
    ui/logger.cpp
    ui/logger.hpp
    ui/logger.ui
    ui/mainwindow.cpp
    ui/mainwindow.hpp
    ui/mainwindow.ui
)

target_link_libraries(STQClient PRIVATE Qt6::Widgets)
