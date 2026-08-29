add_executable(chad
    cli.cpp
    main.cpp
)

target_include_directories(chad PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(chad PRIVATE CHAD_VERSION="${PROJECT_VERSION}")
