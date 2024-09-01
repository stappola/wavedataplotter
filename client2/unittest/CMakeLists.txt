cmake_minimum_required(VERSION 3.10)
project(client2_unittest)

include(FetchContent) 

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        main
)
FetchContent_MakeAvailable(googletest)

set(CMAKE_CXX_STANDARD 17)

include_directories(.)
include_directories(..)
include_directories(../../common/inc)

enable_testing()

add_executable(
    client2_unittest.app
    testmain.cpp
    fakesocketreader.cpp
    testprinterbounds.cpp
    ../socketdataprinter.cpp
    ../../common/src/defaulttimestamp.cpp
)

target_link_libraries(
    client2_unittest.app
    gtest
    gmock
    gtest_main
    pthread
    dl
    -static-libstdc++
)

include(GoogleTest)


