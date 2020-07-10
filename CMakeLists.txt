cmake_minimum_required(VERSION 3.0.0)

project(SystemStatus VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_FLAGS "-g -Wall")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5 COMPONENTS
    Core
    Concurrent
    DBus
REQUIRED)


set(SRCS
    processstat.h
    procstat.h
)

add_executable(${PROJECT_NAME} "main.cpp" ${SRCS})

target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Concurrent
    Qt5::DBus
)
