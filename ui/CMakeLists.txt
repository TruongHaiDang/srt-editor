file(GLOB_RECURSE UI_FILES "./*.ui")

add_library("${EXEC_NAME}_UI" INTERFACE)
set_target_properties("${EXEC_NAME}_UI" PROPERTIES INTERFACE_AUTOUIC_SEARCH_PATHS "${CMAKE_SOURCE_DIR}/ui")
target_sources("${EXEC_NAME}_UI" INTERFACE ${UI_FILES})

file(GLOB_RECURSE UI_SOURCES "./*.cpp")
target_sources(${EXEC_NAME} PUBLIC ${UI_SOURCES})
