# set(VIEW_SRC
#
#     # cli
#     view/cli/main.cpp
#     view/cli/main.hpp
#     view/cli/queue.hpp
#     view/cli/queuelist.hpp
#     view/cli/utils.cpp
#     view/cli/utils.hpp
# )

set(CLI_CONTROLLER_SRC

    # grpc
    controller/grpcserver/accessimpl.cpp
    controller/grpcserver/accessimpl.hpp
)

add_executable(STQ
    ${VIEW_SRC}
    ${CONTROLLER_SRC}

    stq.cpp)

add_dependencies(STQ grpc_common STQModel)

target_link_libraries(STQ
    PRIVATE

    grpc_common
    spdlog::spdlog
    SQLite::SQLite3
    rapidjson
)

if (MSVC AND WIN32 AND NOT MSVC_VERSION VERSION_LESS 142)
    target_link_options(STQ PRIVATE $<$<CONFIG:Debug>:/INCREMENTAL>)
    target_compile_options(STQ PRIVATE $<$<CONFIG:Debug>:/ZI>)
endif()
