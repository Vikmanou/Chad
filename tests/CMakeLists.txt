add_executable(chad_tests
    run_cases.cpp
)

target_link_libraries(chad_tests PRIVATE chad::core chad_options)
target_compile_definitions(chad_tests PRIVATE
    CHAD_ROOT_DIR="${PROJECT_SOURCE_DIR}"
    CHAD_EXE="$<TARGET_FILE:chad>"
    CHAD_TEMP_DIR="${CMAKE_CURRENT_BINARY_DIR}"
)
add_dependencies(chad_tests chad)

add_test(NAME cases COMMAND chad_tests)