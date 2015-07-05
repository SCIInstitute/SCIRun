project (TeemSampleProject)

cmake_minimum_required(VERSION 2.4)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

find_package(Teem REQUIRED)
include(${Teem_USE_FILE})

add_executable(sample sanity.c)
target_link_libraries(sample teem)
