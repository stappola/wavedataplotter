cmake_minimum_required(VERSION 3.10)
project(client2_unittest)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 17)

include_directories(.)
include_directories(..)
include_directories(../../common/inc)
include_directories(../../googletest/googletest/include)
include_directories(../../googletest/googlemock/include)
link_directories(../../googletest/lib)

enable_testing()

add_executable(
    client2_unittest.app

    # Test framework
    testmain.cpp
    fakesocketreader.cpp
    testprinterbounds.cpp
  
    # System under test
    ../socketdataprinter.cpp
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


