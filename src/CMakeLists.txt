configure_file(sacnlogger_config.h.in "${PROJECT_BINARY_DIR}/include/sacnlogger_config.h")

add_subdirectory(sacnloggerlib)
add_subdirectory(sacnlogger)
