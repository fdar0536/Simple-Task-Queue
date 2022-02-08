set(PROCESS_SRC
    model/process/abstractprocess.cpp
    model/process/abstractprocess.hpp
)

if (WIN32)
    LIST(APPEND PROCESS_SRC
        model/process/winprocess.cpp
        model/process/winprocess.hpp
    )
else()
    LIST(APPEND PROCESS_SRC
        model/process/nixprocess.cpp
        model/process/nixprocess.hpp
    )
endif(WIN32)

set(SERVER_SRC
    ${PROCESS_SRC}
    model/stqqueue.cpp
    model/stqqueue.hpp
    model/stqtask.hpp

    main.cpp
)

add_executable(STQServer
    ${SERVER_SRC}
)

add_dependencies(STQServer global)

target_link_libraries(STQServer
    gRPC::grpc++
    global
)
