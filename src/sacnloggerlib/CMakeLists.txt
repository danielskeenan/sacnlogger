add_library(sacnloggerlib STATIC
        AbbreviationMap.cpp
        Config.cpp
        CsvRow.cpp
        UniverseMonitor.cpp
)

find_package(fmt CONFIG REQUIRED)
include(sACN)
find_package(spdlog CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
target_link_libraries(sacnloggerlib PUBLIC
        fmt::fmt
        sACN
        spdlog::spdlog
        yaml-cpp::yaml-cpp
)
