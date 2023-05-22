set(BINARY ${CMAKE_PROJECT_NAME}_tst)

set(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})

include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        release-1.11.0
)

FetchContent_MakeAvailable(googletest)

file(GLOB TEST_SOURCES ${TEST_DIR}/*.cpp)

include(GoogleTest)

add_executable(
    ${BINARY}
    ${TEST_SOURCES}
)

target_link_libraries(
    ${BINARY}
    GTest::gtest_main
    Concurrency_Src
)

gtest_discover_tests(${BINARY})
