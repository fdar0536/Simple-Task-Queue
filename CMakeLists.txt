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

option(ENABLE_CLI "enable CLI" on)
option(ENABLE_SERVER "enable server" on)

include(cmake/getGitInfo.cmake)

set(STQ_COMMIT "")
get_git_hash(STQ_COMMIT)
message(STATUS "Git hash is ${STQ_COMMIT}")

# git_BRANCh
set(STQ_BRANCH "")
get_git_branch(STQ_BRANCH)
message(STATUS "Git branch is ${STQ_BRANCH}")

configure_file(config.h.in config.h @ONLY)
include_directories(After SYSTEM ${CMAKE_CURRENT_BINARY_DIR})
include(GNUInstallDirs)

find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)
find_package(spdlog REQUIRED)
find_package(cxxopts REQUIRED)

if (ENABLE_SERVER)
    find_path(INIPP_INCLUDE_DIRS "inipp.h" REQUIRED)
    find_package(SQLite3 REQUIRED)
endif (ENABLE_SERVER)

if (ENABLE_GUI)
    find_package(Qt6 6.5 COMPONENTS Widgets REQUIRED)
endif (ENABLE_GUI)

set(PROTOC_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/protos)
include(cmake/protoc.cmake)

set(PROTOS
    ${CMAKE_CURRENT_SOURCE_DIR}/protos/access.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/protos/queue.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/protos/queuelist.proto
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
include(cmake/stqserver.cmake)
include(cmake/stqcli.cmake)
