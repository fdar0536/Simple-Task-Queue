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

    controller/grpc/accessimpl.cpp
    controller/grpc/accessimpl.hpp
    controller/grpc/consoleimpl.cpp
    controller/grpc/consoleimpl.hpp
    controller/grpc/doneimpl.cpp
    controller/grpc/doneimpl.hpp
    controller/grpc/implcommon.cpp
    controller/grpc/implcommon.hpp
    controller/grpc/pendingimpl.cpp
    controller/grpc/pendingimpl.hpp
    controller/grpc/queueimpl.cpp
    controller/grpc/queueimpl.hpp

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

if (BUILD_RESTFUL_SERVER)
    list(APPEND SERVER_SRC

        restfulserver/picohttpparser/picohttpparser.c
        restfulserver/picohttpparser/picohttpparser.h

        restfulserver/rax/rax.c
        restfulserver/rax/rax.h
        restfulserver/rax/rax_malloc.h

        restfulserver/abstractserver.cpp
        restfulserver/abstractserver.hpp
        restfulserver/httputils.cpp
        restfulserver/httputils.hpp
        restfulserver/raxadapter.cpp
        restfulserver/raxadapter.hpp
    )

    if (WIN32)
        list(APPEND SERVER_SRC
            restfulserver/winserver.cpp
            restfulserver/winserver.hpp
        )
    endif (WIN32)
endif (BUILD_RESTFUL_SERVER)

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
    rapidjson
)

if(BUILD_SERVER_DEV)
    add_executable(ServerTest
        ${PROCESS_SRC}
        logger.cpp
        logger.hpp
        test.cpp
    )
endif(BUILD_SERVER_DEV)

install(TARGETS STQServer)
