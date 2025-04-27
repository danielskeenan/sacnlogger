add_executable(sacnlogger_monitor
        main.cpp
)

find_package(argparse CONFIG REQUIRED)
# Current boost version in target Debian version (https://packages.debian.org/search?keywords=libboost-dev).
find_package(Boost 1.81 CONFIG COMPONENTS filesystem REQUIRED)
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

include(GNUInstallDirs)
install(TARGETS sacnlogger_monitor COMPONENT monitor)
install(FILES sacnlogger-monitor.service DESTINATION "${CMAKE_INSTALL_LIBDIR}/systemd/system" COMPONENT monitor)
