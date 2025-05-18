add_library(sacnloggerlib STATIC
        AbbreviationMap.cpp
        AddressOrHostname.cpp
        Config.cpp
        CsvRow.cpp
        UniverseMonitor.cpp
)

find_package(fmt CONFIG REQUIRED)
include(sACN)
find_package(spdlog CONFIG REQUIRED)
include(yaml-cpp)
target_link_libraries(sacnloggerlib PUBLIC
        sacnlogger_config
        fmt::fmt
        sACN
        spdlog::spdlog
        yaml-cpp
)

if (EMBEDDED_BUILD)
    target_sources(sacnloggerlib PRIVATE
            SystemConfig/NetworkConfig.cpp
            SystemConfig.cpp
    )

    find_package(boost_bimap REQUIRED CONFIG)
    find_package(boost_endian REQUIRED CONFIG)
    find_package(inja CONFIG REQUIRED)
    find_package(nlohmann_json CONFIG REQUIRED)
    include(sdbus-cpp)
    target_link_libraries(sacnloggerlib PRIVATE
            Boost::bimap
            Boost::endian
            nlohmann_json::nlohmann_json
            pantor::inja
            sdbus-c++
    )

    include(GNUInstallDirs)
    install(DIRECTORY "${PROJECT_SOURCE_DIR}/resources/system_config" DESTINATION "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}" COMPONENT main)
endif ()
