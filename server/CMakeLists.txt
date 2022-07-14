option(BUILD_SERVER_DEV "build binary for server develop" ON)

set(PROCESS_SRC
    model/process/abstractprocess.cpp
    model/process/abstractprocess.hpp
)

if (WIN32)
    LIST(APPEND PROCESS_SRC
        model/process/winprocess.cpp
        model/process/winprocess.hpp
    )

    if (MINGW)
       set(GET_OPT_SRC "")
    else()
        set(GET_OPT_SRC
            win32-code/getopt.c
            win32-code/getopt.h
            win32-code/getopt_long.c
        )
    endif(MINGW)
else()
    LIST(APPEND PROCESS_SRC
        model/process/nixprocess.cpp
        model/process/nixprocess.hpp
    )

    set(GET_OPT_SRC "")
endif(WIN32)

set(SERVER_SRC
    ${PROCESS_SRC}
    ${GET_OPT_SRC}

    controller/accessimpl.cpp
    controller/accessimpl.hpp
    controller/consoleimpl.cpp
    controller/consoleimpl.hpp
    controller/doneimpl.cpp
    controller/doneimpl.hpp
    controller/implcommon.cpp
    controller/implcommon.hpp
    controller/pendingimpl.cpp
    controller/pendingimpl.hpp
    controller/queueimpl.cpp
    controller/queueimpl.hpp

    model/stqqueue.cpp
    model/stqqueue.hpp
    model/stqqueuelist.cpp
    model/stqqueuelist.hpp
    model/stqtask.cpp
    model/stqtask.hpp

    common.hpp
    global.cpp
    global.hpp
    logger.cpp
    logger.hpp
    main.cpp
)

add_executable(STQServer
    ${SERVER_SRC}
)

add_dependencies(STQServer grpc_common)

target_link_libraries(STQServer
    PRIVATE
    gRPC::grpc
    gRPC::grpc++
    gRPC::grpc++_reflection
    absl::synchronization
    grpc_common
    nlohmann_json::nlohmann_json
)

if(BUILD_SERVER_DEV)
    add_executable(ServerTest
        ${PROCESS_SRC}
        logger.cpp
        logger.hpp
        test.cpp
    )
endif(BUILD_SERVER_DEV)
