set(PROCESS_SRC
    process/abstractprocess.cpp
    process/abstractprocess.hpp
)

if (WIN32)
    LIST(APPEND PROCESS_SRC
        process/winprocess.cpp
        process/winprocess.hpp
    )
else()
    LIST(APPEND PROCESS_SRC
        process/nixprocess.cpp
        process/nixprocess.hpp
    )
endif(WIN32)

set(SERVER_SRC
    ${PROCESS_SRC}
    main.cpp
)

add_executable(STQServer
    ${SERVER_SRC}
)

target_link_libraries(STQServer
    gRPC::grpc++
)
