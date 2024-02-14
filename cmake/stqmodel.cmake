set(STQ_MODEL_LIBS
    protobuf::libprotobuf
    gRPC::grpc++

    grpc_common
    spdlog::spdlog
)

set(MODEL_SRC

    # DAO
    model/dao/iconnect.cpp
    model/dao/iconnect.hpp
    model/dao/iqueuelist.hpp
    model/dao/iqueue.hpp

    model/errmsg.cpp
    model/errmsg.hpp
    model/utils.cpp
    model/utils.hpp

    # global
    controller/global/defines.hpp
)

if(ENABLE_SERVER)
    list(APPEND MODEL_SRC
        model/dao/dirutils.cpp
        model/dao/dirutils.hpp

        #sqlite
        model/dao/sqliteconnect.cpp
        model/dao/sqliteconnect.hpp
        model/dao/sqlitequeuelist.cpp
        model/dao/sqlitequeuelist.hpp
        model/dao/sqlitequeue.cpp
        model/dao/sqlitequeue.hpp

        # proc
        model/proc/iproc.cpp
        model/proc/iproc.hpp
        model/proc/task.cpp
        model/proc/task.hpp
    )

    if (WIN32)
        list(APPEND MODEL_SRC
            model/proc/winproc.cpp
            model/proc/winproc.hpp

            model/win32-code/getopt.c
            model/win32-code/getopt.h
            model/win32-code/getopt_long.c
        )
        elseif ((NOT WIN32))
            list(APPEND MODEL_SRC
                model/posixprocess.cpp
                model/posixprocess.hpp
            )
    endif (WIN32)
endif(ENABLE_SERVER)

add_library(stqmodel STATIC
    ${MODEL_SRC}
)

add_dependencies(stqmodel grpc_common)

if (ENABLE_SERVER)
    list(APPEND STQ_MODEL_LIBS
        SQLite::SQLite3
    )
endif (ENABLE_SERVER)

target_link_libraries(stqmodel
    PRIVATE

    ${STQ_MODEL_LIBS}
)
