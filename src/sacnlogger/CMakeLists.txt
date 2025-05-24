add_executable(${PROJECT_NAME}
        main.cpp
        EtcPalLogHandler.cpp
)

find_package(argparse CONFIG REQUIRED)
find_package(date CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE
        sacnlogger_config
        argparse::argparse
        date::date
        date::date-tz
        EtcPal
        sacnloggerlib
        spdlog::spdlog
)
add_dependencies(${PROJECT_NAME} sacnlogger_webui)

install(TARGETS ${PROJECT_NAME} COMPONENT main)
