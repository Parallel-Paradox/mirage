include(FetchContent)

# Introduce GoogleTest
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.x)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

link_libraries(mirage_engine)
link_libraries(GTest::gtest_main)

enable_testing()
include(GoogleTest)

add_executable(test.mirage_base
    mirage_base/array_tests.cpp
    mirage_base/auto_ptr_tests.cpp
    mirage_base/hash_map_tests.cpp
    mirage_base/map_tests.cpp
    mirage_base/set_tests.cpp
    mirage_base/util_tests.cpp
    mirage_base/linked_list_tests.cpp
)
gtest_discover_tests(test.mirage_base)
