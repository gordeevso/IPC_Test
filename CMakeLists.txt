cmake_minimum_required(VERSION 3.22)
project(IPC_Test)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DNET_DEBUG)
endif()

enable_testing()

add_subdirectory(3rdParty)
add_subdirectory(src)
add_subdirectory(tests)
