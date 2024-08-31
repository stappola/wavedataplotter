cmake_minimum_required(VERSION 3.6)
project(client1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread -g -Wl,--no-as-needed")

include_directories(.)

set(SOURCE_FILES
    main.cpp
    socketdataprinter.cpp
    socketreaderthread.cpp
)

add_executable(client1 ${SOURCE_FILES})

target_link_libraries(client1
    dl
    -static-libstdc++
)

