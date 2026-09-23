set(CHAD_PRELUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lang/prelude.chad)
file(READ ${CHAD_PRELUDE_PATH} CHAD_PRELUDE)
configure_file(lang/prelude.h.in ${CMAKE_CURRENT_BINARY_DIR}/generated/prelude.h @ONLY)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${CHAD_PRELUDE_PATH})

add_library(chad_core
    core/error.cpp
    core/file.cpp
    core/text.cpp
    lang/lexer.cpp
    lang/parser.cpp
    lang/compile.cpp
    net/engine.cpp
    net/expr.cpp
    net/net.cpp
    runtime/io.cpp
    runtime/utf8.cpp
    runtime/input.cpp
)
add_library(chad::core ALIAS chad_core)

target_include_directories(chad_core
    PUBLIC ${PROJECT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/generated
)
target_link_libraries(chad_core PRIVATE chad_options)