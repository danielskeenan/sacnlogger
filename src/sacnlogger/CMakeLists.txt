add_executable(${PROJECT_NAME}
        main.cpp
        EtcPalLogHandler.cpp
)

find_package(argparse CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE
        argparse::argparse
        EtcPal
        sacnloggerlib
)

install(TARGETS ${PROJECT_NAME})
