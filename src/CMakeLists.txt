include_directories(${CURL_INCLUDE_DIRS})

include(FetchContent)

FetchContent_Declare(
    curl
    GIT_REPOSITORY https://github.com/curl/curl.git
    GIT_TAG curl-8_8_0 # Or a specific commit hash
)
FetchContent_MakeAvailable(curl)

FetchContent_Declare(
    json 
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)
FetchContent_MakeAvailable(json)

file(GLOB_RECURSE SOURCES "./*.cpp")

add_executable(${EXEC_NAME} ${SOURCES})
target_link_libraries(${EXEC_NAME} PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets "${EXEC_NAME}_UI")
target_link_libraries(${EXEC_NAME} PRIVATE CURL::libcurl nlohmann_json::nlohmann_json)
