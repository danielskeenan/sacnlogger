add_executable(sacnlogger_monitor
        main.cpp
)

find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
target_link_libraries(sacnlogger_monitor PRIVATE
        fmt::fmt
        spdlog::spdlog
)

install(TARGETS sacnlogger_monitor COMPONENT monitor)
list(APPEND CPACK_COMPONENTS_ALL monitor)
set(CPACK_DEBIAN_MONITOR_PACKAGE_NAME "sacnlogger-monitor")
set(CPACK_RPM_MONITOR_PACKAGE_NAME "sacnlogger-monitor")
set(CPACK_DEBIAN_MONITOR_FILE_NAME "DEB-DEFAULT")
set(CPACK_RPM_MONITOR_FILE_NAME "RPM-DEFAULT")
set(CPACK_DEBIAN_MONITOR_PACKAGE_DEPENDS "libc6:arm64 (>= 2.36), libstdc++6")
set(CPACK_DEBIAN_MONITOR_PACKAGE_SECTION "net")
