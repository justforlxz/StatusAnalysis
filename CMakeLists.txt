cmake_minimum_required(VERSION 3.0.0)

project(SystemStatus VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(SRCS
    processstat.h
    processstat.cpp
    procstat.h
)

add_executable(${PROJECT_NAME} "main.cpp" ${SRCS})

target_link_libraries(${PROJECT_NAME} stdc++fs)
