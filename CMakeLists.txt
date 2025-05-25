# Pin minimum CMake version to latest Ubuntu LTS release.
# https://packages.ubuntu.com/search?searchon=names&keywords=cmake
cmake_minimum_required(VERSION 3.28)
project(sacnlogger
        DESCRIPTION "Logs sACN activity across multiple universes and sources."
        VERSION 0.1.0
        HOMEPAGE_URL "https://danielskeenan.github.io/sacnlogger/"
        LANGUAGES CXX
)
set(PROJECT_DISPLAY_NAME "sACN Logger")
set(PROJECT_AUTHOR "Dan Keenan")
set(PROJECT_ORGANIZATION_NAME "dankeenan")
set(PROJECT_ORGANIZATION_DOMAIN "dankeenan.org")
string(TIMESTAMP PROJECT_BUILD_TIMESTAMP "%s" UTC)
find_package(Git REQUIRED)
execute_process(COMMAND "${GIT_EXECUTABLE}" log -n1 --format=%H
        OUTPUT_VARIABLE PROJECT_COMMIT_SHA
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        COMMAND_ERROR_IS_FATAL ANY
)
string(STRIP "${PROJECT_COMMIT_SHA}" PROJECT_COMMIT_SHA)
execute_process(COMMAND "${GIT_EXECUTABLE}" tag --points-at=HEAD
        OUTPUT_VARIABLE PROJECT_COMMIT_TAG
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        COMMAND_ERROR_IS_FATAL ANY
)
string(STRIP "${PROJECT_COMMIT_TAG}" PROJECT_COMMIT_TAG)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 20)

option(BUILD_EXEC "Build the executable program.  You probably want to do this." ON)
if (BUILD_EXEC)
    option(EMBEDDED_BUILD "Include features for running the program as part of an appliance." OFF)

    # Platform detection.
    string(TOUPPER "PLATFORM_${CMAKE_SYSTEM_NAME}" PLATFORM_DEFINITION)
    add_compile_definitions("${PLATFORM_DEFINITION}")

    include_directories("${PROJECT_SOURCE_DIR}/include" "${PROJECT_BINARY_DIR}/include")

    # Shared files directory, relative to binary directory.
    include(GNUInstallDirs)
    set(SHARE_DIR "../${CMAKE_INSTALL_DATADIR}/sacnlogger")

    add_subdirectory(messages)
    add_subdirectory(src)
    add_subdirectory(webui)

    include(CTest)
    if (BUILD_TESTING)
        add_subdirectory(test)
    endif ()

    option(BUILD_PACKAGE "Build packages/installers usable on this platform." OFF)
    if (BUILD_PACKAGE)
        add_subdirectory(install)
    endif ()
endif ()

option(BUILD_DOC "Build documentation" ON)
if (BUILD_DOC)
    add_subdirectory(doc)
endif ()

option(BUILD_RASPI "Build Raspberry Pi image" OFF)
if (BUILD_RASPI)
    add_subdirectory(raspi)
endif ()
