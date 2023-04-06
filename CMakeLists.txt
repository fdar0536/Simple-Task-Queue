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
        add_compile_options(/W4 /utf-8)
    endif (MINGW)
endif (UNIX) # end if (UNIX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(STQ_VERSION r12)
set(STQ_NAME SimpleTaskQueue)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(NOT DEFINED CLIENT_TIMEOUT)
    set(CLIENT_TIMEOUT 31)
endif(NOT DEFINED CLIENT_TIMEOUT)

if((CLIENT_TIMEOUT GREATER 61) OR (CLIENT_TIMEOUT LESS 1))
    set(CLIENT_TIMEOUT 31)
endif((CLIENT_TIMEOUT GREATER 61) OR (CLIENT_TIMEOUT LESS 1))

option(ENABLE_GUI "enable GUI" on)
option(ENABLE_MOBILE "enable qt for mobile device" off)

if (ENABLE_GUI)
    set(STQ_GUI ON)
endif (ENABLE_GUI)

if (ENABLE_MOBILE)
    set(STQ_GUI ON)
    set(STQ_MOBILE ON)
endif (ENABLE_MOBILE)

configure_file(config.h.in config.h @ONLY)
include_directories(After SYSTEM ${CMAKE_CURRENT_BINARY_DIR})
include(GNUInstallDirs)

find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)
find_package(spdlog REQUIRED)
find_path(INIPP_INCLUDE_DIRS "inipp.h" REQUIRED)

if (ENABLE_GUI)
    if(ENABLE_MOBILE)
        find_package(Qt6 COMPONENTS Quick Gui REQUIRED)
    else()
        find_package(Qt6 COMPONENTS Widgets Quick REQUIRED)
    endif(ENABLE_MOBILE)
endif (ENABLE_GUI)

if (NOT ENABLE_MOBILE)
    find_package(SQLite3 REQUIRED)
endif (NOT ENABLE_MOBILE)

set(PROTOC_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/protos)
include(cmake/protoc.cmake)

set(PROTOS
    ${CMAKE_CURRENT_SOURCE_DIR}/protos/access.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/protos/console.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/protos/done.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/protos/pending.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/protos/queue.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/protos/types.proto
)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${CMAKE_CURRENT_BINARY_DIR} ${PROTOS})
grpc_generate_cpp(GRPC_SRCS GRPC_HDRS ${CMAKE_CURRENT_BINARY_DIR} ${PROTOS})

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

include(cmake/stqmodel.cmake)
include(cmake/stq.cmake)
