set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB PROJECT_SOURCES ${PROJECT_DIR}/*.cpp)

include(FetchContent)

FetchContent_Declare(
    ReCpp
    GIT_REPOSITORY https://github.com/RedFox20/ReCpp.git
    GIT_TAG        3a2e2b0
)
FetchContent_MakeAvailable(ReCpp)

add_library(
    Concurrency_Src
    ${PROJECT_SOURCES}
)

target_include_directories(Concurrency_Src PUBLIC include) # PUBLIC include/rpp)
target_link_libraries(Concurrency_Src PUBLIC ReCpp)

add_executable(
    Concurrency_Exec
    ${PROJECT_DIR}/main.cpp
)

target_link_libraries(Concurrency_Exec PUBLIC Concurrency_Src)