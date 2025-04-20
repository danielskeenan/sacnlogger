include(FetchContent)
FetchContent_Declare(yaml-cpp
        GIT_REPOSITORY "https://github.com/jbeder/yaml-cpp.git"
        GIT_TAG "28f93bdec6387d42332220afa9558060c8016795"
        EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(yaml-cpp)
