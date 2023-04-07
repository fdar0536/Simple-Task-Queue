set(MODEL_SRC

    # DAO
    model/dao/iconnect.hpp
    model/dao/iqueuelist.hpp
    model/dao/iqueue.hpp

    model/dao/dirutils.cpp
    model/dao/dirutils.hpp

    # proc
    model/proc/iproc.cpp
    model/proc/iproc.hpp
    model/proc/task.cpp
    model/proc/task.hpp

    model/utils.cpp
    model/utils.hpp
)

if (ENABLE_GUI)
    list(APPEND MODEL_SRC

        #grpc
        model/dao/grpcconnect.cpp
        model/dao/grpcconnect.hpp
        model/dao/grpcutils.cpp
        model/dao/grpcutils.hpp
    )
endif (ENABLE_GUI)

if (WIN32 AND (NOT ENABLE_MOBILE))
    list(APPEND MODEL_SRC
        model/proc/winproc.cpp
        model/proc/winproc.hpp

        model/win32-code/getopt.c
        model/win32-code/getopt.h
        model/win32-code/getopt_long.c
    )
elseif ((NOT WIN32) AND (NOT ENABLE_MOBILE))
    list(APPEND MODEL_SRC
        model/posixprocess.cpp
        model/posixprocess.hpp
    )
endif (WIN32 AND (NOT ENABLE_MOBILE))

if (NOT ENABLE_MOBILE)
    list(APPEND MODEL_SRC

        #sqlite
        model/dao/sqliteconnect.cpp
        model/dao/sqliteconnect.hpp
        model/dao/sqlitequeuelist.cpp
        model/dao/sqlitequeuelist.hpp
        model/dao/sqlitequeue.cpp
        model/dao/sqlitequeue.hpp
    )
endif (NOT ENABLE_MOBILE)

add_library(STQModel STATIC
    ${MODEL_SRC}
)

add_dependencies(STQModel grpc_common)

target_link_libraries(STQModel
    PRIVATE
    protobuf::libprotobuf
    gRPC::grpc++

    grpc_common
    spdlog::spdlog
    SQLite::SQLite3
)
