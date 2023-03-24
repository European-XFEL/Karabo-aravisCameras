cmake_minimum_required (VERSION 3.14)

project (aravisCameras LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

find_package(aravisCameras REQUIRED)
find_package(GTest REQUIRED)

add_executable(
   test-${CMAKE_PROJECT_NAME}
   testrunner.cc   # The test runner entry point
   testAravisCameras.cc
   # Add any other source file in here.

)

target_compile_options(
    test-${CMAKE_PROJECT_NAME}
    PUBLIC -Wfatal-errors -Wno-unused-local-typedefs
           -Wno-deprecated-declarations -Wall)

target_link_libraries(
    test-${CMAKE_PROJECT_NAME}
    PRIVATE
    Threads::Threads
    aravisCameras::aravisCameras
    GTest::gtest
)

enable_testing()
add_test(
    ${CMAKE_PROJECT_NAME}Tests
    test-${CMAKE_PROJECT_NAME}
    # "--gtest_list_tests or other test arguments"
)
