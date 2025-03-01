add_executable(sacnloggerlib_test
        ConfigTest.cpp
)
find_package(Catch2 3 REQUIRED)
find_package(fmt CONFIG REQUIRED)
target_link_libraries(sacnloggerlib_test PRIVATE Catch2::Catch2WithMain fmt::fmt sacnloggerlib)
target_compile_definitions(sacnloggerlib_test PRIVATE "RESOURCES_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/resources\"")

include(CTest)
include(Catch)
catch_discover_tests(sacnloggerlib_test)
