configure_file(
    "${CMAKE_SOURCE_DIR}/ui/configure.h.in"
    "${CMAKE_SOURCE_DIR}/ui/configure.h"
)

file(GLOB_RECURSE UI_FILES "./*.ui")

add_library("${EXEC_NAME}_UI" INTERFACE)
set_target_properties("${EXEC_NAME}_UI" PROPERTIES INTERFACE_AUTOUIC_SEARCH_PATHS "${CMAKE_SOURCE_DIR}/ui")
target_sources("${EXEC_NAME}_UI" INTERFACE ${UI_FILES})

file(GLOB_RECURSE UI_SOURCES "./*.cpp" "./*.qrc")
target_sources(${EXEC_NAME} PUBLIC ${UI_SOURCES})
set_target_properties(${EXEC_NAME} PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)