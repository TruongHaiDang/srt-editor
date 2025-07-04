cmake_minimum_required(VERSION 3.25)
project(srt_editor VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Wall -Wpedantic")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)

set(EXEC_NAME "srt_editor")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

include_directories(include)
include_directories(ui)

add_subdirectory(src)
add_subdirectory(ui)
