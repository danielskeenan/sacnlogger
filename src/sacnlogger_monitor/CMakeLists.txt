add_executable(sacnlogger_monitor
        main.cpp
)

find_package(argparse CONFIG REQUIRED)
# Current boost version in target Debian version (https://packages.debian.org/search?keywords=libboost-dev).
find_package(Boost 1.74 CONFIG COMPONENTS filesystem REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
target_link_libraries(sacnlogger_monitor PRIVATE
        argparse::argparse
        Boost::boost
        Boost::filesystem
        fmt::fmt
        spdlog::spdlog
)
add_dependencies(sacnlogger_monitor ${PROJECT_NAME})

install(TARGETS sacnlogger_monitor COMPONENT monitor)
list(APPEND CPACK_COMPONENTS_ALL monitor)
set(CPACK_DEBIAN_MONITOR_PACKAGE_NAME "sacnlogger-monitor")
set(CPACK_RPM_MONITOR_PACKAGE_NAME "sacnlogger-monitor")
set(CPACK_DEBIAN_MONITOR_FILE_NAME "DEB-DEFAULT")
set(CPACK_RPM_MONITOR_FILE_NAME "RPM-DEFAULT")
set(CPACK_DEBIAN_MONITOR_PACKAGE_DEPENDS "libc6:arm64 (>= 2.36), libstdc++6")
set(CPACK_DEBIAN_MONITOR_PACKAGE_SECTION "net")
