add_subdirectory(googletest)
add_library(GTest::GTest INTERFACE IMPORTED)
target_link_libraries(GTest::GTest INTERFACE gtest_main)

add_executable(tests tests.cpp)

target_link_libraries(tests
        PRIVATE
        GTest::GTest
        Common
        EntriesProcessing)

add_test(common_gtests tests)
