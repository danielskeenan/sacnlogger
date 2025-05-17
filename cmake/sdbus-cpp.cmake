include(FetchContent)
FetchContent_Declare(sdbus-cpp
        GIT_REPOSITORY "https://github.com/Kistler-Group/sdbus-cpp.git"
        GIT_TAG "v2.1.0"
        EXCLUDE_FROM_ALL
)
set(SDBUSCPP_BUILD_DOCS OFF CACHE INTERNAL "")
set(SDBUSCPP_INSTALL OFF CACHE INTERNAL "")
execute_process(COMMAND systemd --version
        OUTPUT_VARIABLE SYSTEMD_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
        COMMAND_ERROR_IS_FATAL ANY
)
if (NOT "${SYSTEMD_VERSION}" MATCHES "^systemd ([0-9A-Za-z]+)")
    message(FATAL_ERROR "Cannot determine running systemd version.")
endif ()
set(SDBUSCPP_LIBSYSTEMD_VERSION "${CMAKE_MATCH_1}" CACHE INTERNAL "")
FetchContent_MakeAvailable(sdbus-cpp)
