add_library(sacnloggerlib STATIC
        AbbreviationMap.cpp
        AddressOrHostname.cpp
        Config.cpp
        CsvRow.cpp
        DiskSpaceMonitor.cpp
        Runner.cpp
        UniverseMonitor.cpp
)


find_package(boost_signals2 CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(nlohmann_json_schema_validator CONFIG REQUIRED)
include(sACN)
find_package(spdlog CONFIG REQUIRED)
target_link_libraries(sacnloggerlib PUBLIC
        Boost::signals2
        fmt::fmt
        nlohmann_json::nlohmann_json
        nlohmann_json_schema_validator::validator
        sACN
        sacnlogger_config
        spdlog::spdlog
)

if (EMBEDDED_BUILD)
    target_sources(sacnloggerlib PRIVATE
            SystemConfig/NetworkConfig.cpp
            SystemConfig.cpp
    )
    target_compile_definitions(sacnloggerlib PUBLIC "SACNLOGGER_EMBEDDED_BUILD")

    find_package(boost_bimap REQUIRED CONFIG)
    find_package(boost_endian REQUIRED CONFIG)
    find_package(inja CONFIG REQUIRED)
    include(sdbus-cpp)
    target_link_libraries(sacnloggerlib PRIVATE
            Boost::bimap
            Boost::endian
            pantor::inja
    )
    target_link_libraries(sacnloggerlib PUBLIC
            sdbus-c++
    )
endif ()

include(GNUInstallDirs)
install(DIRECTORY "${PROJECT_SOURCE_DIR}/resources/" DESTINATION "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}" COMPONENT main)
