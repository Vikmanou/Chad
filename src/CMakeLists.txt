add_library(chad_core
    core/error.cpp
    core/file.cpp
    core/text.cpp
    lang/instruction.cpp
    lang/lexer.cpp
    lang/program.cpp
    runtime/io.cpp
    runtime/utf8.cpp
    runtime/input.cpp
)
add_library(chad::core ALIAS chad_core)

target_include_directories(chad_core PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(chad_core PRIVATE chad_options)