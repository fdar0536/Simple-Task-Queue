set(CONTROLLER_SRC

    # global
    controller/global/defines.hpp
    controller/global/config.cpp
    controller/global/config.hpp
    controller/global/init.cpp
    controller/global/init.hpp
)

if (NOT ENABLE_MOBILE)
    list(APPEND CONTROLLER_SRC

        # grpc
        controller/grpcserver/accessimpl.cpp
        controller/grpcserver/accessimpl.hpp
        controller/grpcserver/server.cpp
        controller/grpcserver/server.hpp
    )
endif (NOT ENABLE_MOBILE)

add_executable(STQ
    ${VIEW_SRC}
    ${CONTROLLER_SRC}

    stq.cpp)

add_dependencies(STQ grpc_common STQModel)

target_link_libraries(STQ
    PRIVATE

    STQModel
    grpc_common
    spdlog::spdlog
    SQLite::SQLite3
    rapidjson
)

if (MSVC AND WIN32 AND NOT MSVC_VERSION VERSION_LESS 142)
    target_link_options(STQ PRIVATE $<$<CONFIG:Debug>:/INCREMENTAL>)
    target_compile_options(STQ PRIVATE $<$<CONFIG:Debug>:/ZI>)
endif()
