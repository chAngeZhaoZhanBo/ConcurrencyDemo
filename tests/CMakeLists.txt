
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
add_executable(TestThreadPool TestThreadPool.cpp)

target_link_libraries(TestThreadPool LibThreadDemo)
target_link_libraries(TestThreadPool ${GTEST_BOTH_LIBRARIES})

target_compile_options(TestThreadPool PRIVATE "-g")


gtest_discover_tests(TestThreadPool)


