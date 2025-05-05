cmake_minimum_required(VERSION 3.10)

project(PipeWireMixer)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PIPEWIRE REQUIRED libpipewire-0.3)

include_directories(${PIPEWIRE_INCLUDE_DIRS})
link_directories(${PIPEWIRE_LIBRARY_DIRS})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_executable(matrix_mixer matrix_mixer.cpp)
add_executable(stream_app stream_app.cpp)

include_directories(PRIVATE "/usr/include/pipewire")

target_link_libraries(matrix_mixer PRIVATE ${PIPEWIRE_LIBRARIES})
target_link_libraries(stream_app PRIVATE ${PIPEWIRE_LIBRARIES})