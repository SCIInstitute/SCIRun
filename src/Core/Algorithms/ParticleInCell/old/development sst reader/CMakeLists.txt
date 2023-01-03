cmake_minimum_required(VERSION 3.15.0)
set(CMAKE_CXX_STANDARD 17)

project(openpmd_test)

find_package(openPMD REQUIRED)

add_executable(test_1 test_1.cpp)

target_link_libraries(test_1 PRIVATE openPMD::openPMD)
