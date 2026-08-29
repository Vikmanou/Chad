add_executable(chad
    cli.cpp
    main.cpp
)

target_include_directories(chad PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(chad PRIVATE chad::core chad_options)
target_compile_definitions(chad PRIVATE CHAD_VERSION="${PROJECT_VERSION}")
