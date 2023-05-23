cmake_minimum_required(VERSION 3.16)
project(Concurrency_Project LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -Wextra -Wpedantic -Werror")
    # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
elseif(MSVC)
    add_compile_options(/O2)
endif()

enable_testing()

add_subdirectory(src)
add_subdirectory(tst)
