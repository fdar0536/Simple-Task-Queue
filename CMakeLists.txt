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

configure_file(config.h.in config.h @ONLY)
include_directories(After SYSTEM ${CMAKE_CURRENT_BINARY_DIR})
include(GNUInstallDirs)

find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)
find_package(Qt6 COMPONENTS Widgets REQUIRED)
find_package(spdlog REQUIRED)
find_package(SQLite3 REQUIRED)

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

# add_subdirectory(client)

set(MODEL_SRC

    # DAO
    model/dao/iconnect.hpp
    model/dao/iqueuelist.hpp
    model/dao/iqueue.hpp

    model/dao/sqliteconnect.cpp
    model/dao/sqliteconnect.hpp
    model/dao/sqlitequeuelist.cpp
    model/dao/sqlitequeuelist.hpp
    model/dao/sqlitequeue.cpp
    model/dao/sqlitequeue.hpp

    model/dao/dirutils.cpp
    model/dao/dirutils.hpp

    model/iprocess.cpp
    model/iprocess.hpp

    model/task.cpp
    model/task.hpp
    model/utils.cpp
    model/utils.hpp
)

if (WIN32)
    list(APPEND MODEL_SRC
        model/winprocess.cpp
        model/winprocess.hpp
    )
endif (WIN32)

set(VIEW_SRC
    
    # cli
    view/cli/main.cpp
    view/cli/main.hpp
    view/cli/queue.hpp
    view/cli/queuelist.hpp
    view/cli/utils.cpp
    view/cli/utils.hpp
)

add_executable(STQ
    ${MODEL_SRC}
    ${VIEW_SRC}

    main.cpp
    main.hpp
)

add_dependencies(STQ grpc_common)

target_link_libraries(STQ
    PRIVATE

    grpc_common
    spdlog::spdlog
    SQLite::SQLite3
)

if (MSVC AND WIN32 AND NOT MSVC_VERSION VERSION_LESS 142)
    target_link_options(STQ PRIVATE $<$<CONFIG:Debug>:/INCREMENTAL>)
    target_compile_options(STQ PRIVATE $<$<CONFIG:Debug>:/ZI>)
endif()
