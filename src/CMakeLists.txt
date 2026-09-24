add_library(chad_core
    core/error.cpp
    core/file.cpp
    core/text.cpp
    compiler/instruction.cpp
    compiler/lexer.cpp
    compiler/program.cpp
    runtime/interpreter.cpp
    runtime/io.cpp
)
add_library(chad::core ALIAS chad_core)

target_include_directories(chad_core PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(chad_core PRIVATE chad_options)
