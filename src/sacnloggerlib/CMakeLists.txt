add_library(sacnloggerlib STATIC
        AbbreviationMap.cpp
        Config.cpp
        CsvRow.cpp
        UniverseMonitor.cpp
)

find_package(fmt CONFIG REQUIRED)
include(sACN)
find_package(spdlog CONFIG REQUIRED)
include(yaml-cpp)
target_link_libraries(sacnloggerlib PUBLIC
        fmt::fmt
        sACN
        spdlog::spdlog
        yaml-cpp
)
