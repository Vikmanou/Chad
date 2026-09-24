add_library(chad_core
    core/file.cpp
    compiler/instruction.cpp
    compiler/program.cpp
)
add_library(chad::core ALIAS chad_core)

target_include_directories(chad_core PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(chad_core PRIVATE chad_options)
