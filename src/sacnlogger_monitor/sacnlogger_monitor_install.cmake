list(APPEND CPACK_COMPONENTS_ALL monitor)
set(CPACK_DEBIAN_MONITOR_PACKAGE_NAME "sacnlogger-monitor")
set(CPACK_RPM_MONITOR_PACKAGE_NAME "sacnlogger-monitor")
set(CPACK_DEBIAN_MONITOR_FILE_NAME "DEB-DEFAULT")
set(CPACK_RPM_MONITOR_FILE_NAME "RPM-DEFAULT")
set(CPACK_DEBIAN_MONITOR_PACKAGE_DEPENDS "libc6 (>= 2.36), libstdc++6, libboost-filesystem1.74.0")
set(CPACK_DEBIAN_MONITOR_PACKAGE_SECTION "net")

# Need extra scripts to make systemd service behave properly.
set(SACNLOGGER_MONITOR_SERVICE "sacnlogger-monitor.service")
foreach (DEB_EXTRA postinst postrm)
    configure_file(${PROJECT_SOURCE_DIR}/src/sacnlogger_monitor/debian/${DEB_EXTRA} "${PROJECT_BINARY_DIR}/src/sacnlogger_monitor/debian/${DEB_EXTRA}" @ONLY)
    list(APPEND CPACK_DEBIAN_MONITOR_PACKAGE_CONTROL_EXTRA "${PROJECT_BINARY_DIR}/src/sacnlogger_monitor/debian/${DEB_EXTRA}")
endforeach ()
