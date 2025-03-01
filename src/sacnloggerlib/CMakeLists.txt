add_library(sacnloggerlib STATIC
        Config.cpp
)

find_package(fmt CONFIG REQUIRED)
include(sACN)
find_package(spdlog CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
target_link_libraries(sacnloggerlib PRIVATE
        fmt::fmt
        sACN
        spdlog::spdlog
        yaml-cpp::yaml-cpp
)
