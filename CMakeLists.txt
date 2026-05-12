cmake_minimum_required(VERSION 3.28)

project(yatop LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

add_executable(yatop
    backend/HttpServer.cpp
    backend/ProcReader.cpp
    backend/ResourceSampler.cpp
    backend/main.cpp
)

target_include_directories(yatop PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/backend
    ${CMAKE_CURRENT_SOURCE_DIR}/libs
)

target_link_libraries(yatop PRIVATE
    Threads::Threads
)

target_compile_options(yatop PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)
