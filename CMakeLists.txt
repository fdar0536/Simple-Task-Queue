cmake_minimum_required(VERSION 3.14)
project(SimpleTaskQueue LANGUAGES C CXX)

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

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(STQ_VERSION r12)
set(STQ_NAME SimpleTaskQueue)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

configure_file(config.h.in config.h @ONLY)
include_directories(After SYSTEM ${CMAKE_CURRENT_BINARY_DIR})
include(GNUInstallDirs)

option(BUILD_SERVER "build server" ON)
option(BUILD_CLIENT "build client" ON)

if ((NOT BUILD_SERVER) AND (NOT BUILD_CLIENT))
    set(BUILD_CLIENT ON)
endif((NOT BUILD_SERVER) AND (NOT BUILD_CLIENT))

find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

set(PROTOC_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/protos)
include(cmake/protoc.cmake)

if (BUILD_SERVER)
    find_package(nlohmann_json 3.10.5 REQUIRED)
    find_package(OpenSSL 1.1.1 REQUIRED)
    find_package(absl REQUIRED)
endif(BUILD_SERVER)

if (BUILD_CLIENT)
    find_package(Qt6 COMPONENTS Widgets REQUIRED)
endif(BUILD_CLIENT)

set(PROTOS
    ${CMAKE_CURRENT_SOURCE_DIR}/protos/queue.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/protos/types.proto
)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${CMAKE_CURRENT_BINARY_DIR} ${PROTOS})
grpc_generate_cpp(GRPC_SRCS GRPC_HDRS ${CMAKE_CURRENT_BINARY_DIR} ${PROTOS})

add_library(logger STATIC
    logger.cpp
    logger.hpp
)

add_library(grpc_common STATIC
    ${PROTO_HDRS}
    ${PROTO_SRCS}
    ${GRPC_HDRS}
    ${GRPC_SRCS}
)

target_link_libraries(grpc_common
    PRIVATE
    protobuf::libprotobuf
    gRPC::grpc++
)

if (BUILD_SERVER)
    add_subdirectory(server)
endif(BUILD_SERVER)

# add_executable(
#     SimpleTaskQueue WIN32
#     main.cpp
#     main.qrc
#     mainwindow.cpp
#     mainwindow.hpp
#     mainwindow.ui
#     tasktab.cpp
#     tasktab.hpp
#     tasktab.ui
#     inputdialog.cpp
#     inputdialog.hpp
#     inputdialog.ui
# )
#
# target_link_libraries(SimpleTaskQueue PRIVATE Qt6::Widgets)
# install(
#     TARGETS SimpleTaskQueue
#     DESTINATION ${CMAKE_INSTALL_BINDIR}
# )
