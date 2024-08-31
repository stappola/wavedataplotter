cmake_minimum_required(VERSION 3.10)
project(client2)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread -g -Wl,--no-as-needed")

set(COMMON "../common")
set(COMMON_INC ${COMMON}/inc)
set(COMMON_SRC ${COMMON}/src)

include_directories(.)
include_directories(${COMMON_INC})

set(SOURCE_FILES
    main.cpp
    socketdataprinter.cpp
    controlchannel.cpp
    ${COMMON_SRC}/socketreaderthread.cpp
)

add_executable(client2 ${SOURCE_FILES})

target_link_libraries(client2
    dl
    -static-libstdc++
)


