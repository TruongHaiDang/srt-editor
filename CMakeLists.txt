cmake_minimum_required(VERSION 3.21)

project(SubtitleEditFree
    VERSION 1.0.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
)

include(FetchContent)

set(BUILD_CURL_EXE OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(CURL_USE_LIBPSL OFF CACHE BOOL "" FORCE)
set(CURL_DISABLE_TESTS ON CACHE BOOL "" FORCE)
set(CURL_DISABLE_INSTALL ON CACHE BOOL "" FORCE)

FetchContent_Declare(
    curl
    URL https://curl.se/download/curl-8.11.1.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(curl)

qt_add_executable(${CMAKE_PROJECT_NAME} resources.qrc)

target_link_libraries(${CMAKE_PROJECT_NAME}
    PRIVATE
        CURL::libcurl
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
)

target_include_directories(${CMAKE_PROJECT_NAME}
    PRIVATE
        ${CMAKE_SOURCE_DIR}/inc
)

if(MSVC)
    target_compile_options(${CMAKE_PROJECT_NAME}
        PRIVATE
            /W4
            /permissive-
    )
else()
    target_compile_options(${CMAKE_PROJECT_NAME}
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
    )
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(${CMAKE_PROJECT_NAME}
        PRIVATE
            QT_NO_DEBUG
            NDEBUG
    )

    if(MSVC)
        target_compile_options(${CMAKE_PROJECT_NAME}
            PRIVATE
                /O2
        )
    else()
        target_compile_options(${CMAKE_PROJECT_NAME}
            PRIVATE
                -O3
                -DNDEBUG
        )
    endif()
endif()

add_subdirectory(src)
add_subdirectory(ui)
