cmake_minimum_required(VERSION 3.28)

project(yatop LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

add_executable(yatop
    backend/src/HttpServer.cpp
    backend/src/ProcReader.cpp
    backend/src/ResourceSampler.cpp
    backend/src/main.cpp
)

target_include_directories(yatop PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/src
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/libs
)

target_link_libraries(yatop PRIVATE
    Threads::Threads
)

target_compile_options(yatop PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)
