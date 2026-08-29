cmake_minimum_required(VERSION 3.16)

project(chad VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

add_library(chad_options INTERFACE)

if(MSVC)
    target_compile_options(chad_options INTERFACE /W4 /wd4018 /wd4389 /wd4267 /utf-8)
else()
    target_compile_options(chad_options INTERFACE -Wall -Wextra -Wno-sign-compare)
endif()

if(MINGW)
    target_link_options(chad_options INTERFACE -static-libstdc++ -static-libgcc)
endif()

add_subdirectory(src)
add_subdirectory(app)

#enable_testing()
#add_subdirectory(tests)
