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
    Multimedia
    Widgets
)

include(FetchContent)

set(BUILD_CURL_EXE OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(CURL_USE_LIBPSL OFF CACHE BOOL "" FORCE)
set(CURL_DISABLE_TESTS ON CACHE BOOL "" FORCE)
set(CURL_DISABLE_INSTALL ON CACHE BOOL "" FORCE)
set(HTTP_ONLY ON CACHE BOOL "" FORCE)
set(USE_LIBIDN2 OFF CACHE BOOL "" FORCE)
set(CURL_USE_LIBSSH2 OFF CACHE BOOL "" FORCE)

if(APPLE)
    set(CURL_USE_OPENSSL OFF CACHE BOOL "" FORCE)
    set(CURL_USE_SECTRANSP ON CACHE BOOL "" FORCE)
endif()

FetchContent_Declare(
    curl
    URL https://curl.se/download/curl-8.11.1.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(curl)

set(APP_SOURCES resources.qrc)

if(APPLE)
    set(MACOS_APP_ICON_NAME "SubtitleEditFree.icns")
    set(MACOS_APP_ICON "${CMAKE_CURRENT_SOURCE_DIR}/assets/${MACOS_APP_ICON_NAME}")
    set_source_files_properties(${MACOS_APP_ICON} PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources"
    )
    list(APPEND APP_SOURCES ${MACOS_APP_ICON})
endif()

qt_add_executable(${CMAKE_PROJECT_NAME}
    MACOSX_BUNDLE
    ${APP_SOURCES}
)

if(APPLE)
    set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE_BUNDLE_NAME "SubtitleEdit Free"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.truonghaidang.subtitleeditfree"
        MACOSX_BUNDLE_ICON_FILE "${MACOS_APP_ICON_NAME}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    )
endif()

target_link_libraries(${CMAKE_PROJECT_NAME}
    PRIVATE
        CURL::libcurl
        Qt6::Core
        Qt6::Gui
        Qt6::Multimedia
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
