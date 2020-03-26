cmake_minimum_required(VERSION 3.14)
project(SimpleTaskQueue LANGUAGES CXX)

if (UNIX)
    add_compile_options(-Wall -Werror -Wextra)

    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        add_compile_options(-Weffc++)
    endif ()

    add_link_options(-Wl,--unresolved-symbols=report-all)
else () # Windows
    if (MINGW)
        add_compile_options(-Wall -Werror -Wextra)

        if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
            add_compile_options(-Weffc++)
        endif ()

        add_link_options(-Wl,--unresolved-symbols=report-all)
    else () # msvc or icc
        add_compile_options(/W4)
    endif (MINGW)
endif (UNIX) # end if (UNIX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(STQ_VERSION r5)
set(STQ_NAME SimpleTaskQueue)

configure_file(config.h.in config.h @ONLY)
include_directories(After SYSTEM ${CMAKE_CURRENT_BINARY_DIR})

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_executable(
    SimpleTaskQueue WIN32
    main.cpp
    main.qrc
    mainwindow.cpp
    mainwindow.hpp
    mainwindow.ui
    tasktab.cpp
    tasktab.hpp
    tasktab.ui
    inputdialog.cpp
    inputdialog.hpp
    inputdialog.ui
)

target_link_libraries(SimpleTaskQueue PRIVATE Qt5::Widgets)
