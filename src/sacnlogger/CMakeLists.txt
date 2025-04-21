add_executable(${PROJECT_NAME}
        main.cpp
        EtcPalLogHandler.cpp
)

find_package(argparse CONFIG REQUIRED)
find_package(date CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE
        argparse::argparse
        date::date
        date::date-tz
        EtcPal
        sacnloggerlib
)

install(TARGETS ${PROJECT_NAME} COMPONENT main)
list(APPEND CPACK_COMPONENTS_ALL main)
set(CPACK_DEBIAN_MAIN_PACKAGE_NAME "sacnlogger")
set(CPACK_RPM_MAIN_PACKAGE_NAME "sacnlogger")
set(CPACK_DEBIAN_MAIN_FILE_NAME "DEB-DEFAULT")
set(CPACK_RPM_MAIN_FILE_NAME "RPM-DEFAULT")
set(CPACK_DEBIAN_MAIN_PACKAGE_DEPENDS "libc6:arm64 (>= 2.36), libstdc++6")
set(CPACK_DEBIAN_MAIN_PACKAGE_SECTION "net")
