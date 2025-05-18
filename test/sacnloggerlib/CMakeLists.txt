add_executable(sacnloggerlib_test
        main.cpp
        AbbreviationMapTest.cpp
        ConfigTest.cpp
        CsvRowTest.cpp
        FileMatcher.h
)

if (EMBEDDED_BUILD)
    include(sdbus-cpp)
    target_sources(sacnloggerlib_test PRIVATE
            SystemConfigTest.cpp
    )
    target_link_libraries(sacnloggerlib_test PRIVATE
            sdbus-c++
    )
endif ()

find_package(Catch2 3 REQUIRED)
find_package(fmt CONFIG REQUIRED)
target_link_libraries(sacnloggerlib_test PRIVATE Catch2::Catch2 fmt::fmt sacnloggerlib)
target_compile_definitions(sacnloggerlib_test PRIVATE
        # File paths for use inside tests.
        "RESOURCES_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/resources\""
        "SACNLOGGER_SHAREDIR=\"${CMAKE_INSTALL_PREFIX}/share/sacnlogger\""
        "SACNLOGGER_SYS_PREFIX=\"${CMAKE_CURRENT_BINARY_DIR}/prefix\""
)
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/prefix")

include(CTest)
include(Catch)
catch_discover_tests(sacnloggerlib_test)
