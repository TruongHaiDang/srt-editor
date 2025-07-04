file(GLOB_RECURSE SOURCES "./*.cpp")

add_executable(${EXEC_NAME} ${SOURCES})
target_link_libraries(${EXEC_NAME} PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets "${EXEC_NAME}_UI")
