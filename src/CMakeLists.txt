# Config library.
configure_file(sacnlogger_config.h.in "${PROJECT_BINARY_DIR}/include/sacnlogger_config.h")
add_library(sacnlogger_config STATIC
        "${PROJECT_SOURCE_DIR}/include/sacnlogger_share.h"
        sacnlogger_share.cpp
        "${PROJECT_BINARY_DIR}/include/sacnlogger_config.h"
)
find_package(Boost REQUIRED COMPONENTS dll)
target_link_libraries(sacnlogger_config PRIVATE Boost::dll)

add_subdirectory(sacnloggerlib)
add_subdirectory(sacnlogger)
