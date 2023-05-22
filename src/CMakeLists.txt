set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB PROJECT_SOURCES ${PROJECT_DIR}/*.cpp)

add_library(
    Concurrency_Src
    ${PROJECT_SOURCES}
)

target_include_directories(Concurrency_Src PUBLIC include)