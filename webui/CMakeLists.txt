find_program(npm_PROG npm DOC "Path to NPM (https://www.npmjs.com)" REQUIRED)
message(STATUS "Installing Web UI dependencies...")
execute_process(
        COMMAND "${npm_PROG}" install
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMAND_ERROR_IS_FATAL ANY
)

set(WEBUI_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/dist")
# The config assumes a relative path and will explode directories in the source dir if it is not given one.
cmake_path(RELATIVE_PATH WEBUI_BUILD_DIR BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" OUTPUT_VARIABLE WEBUI_BUILD_DIR_REL)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/vite.config.ts.in" "${CMAKE_CURRENT_SOURCE_DIR}/vite.config.ts" @ONLY)

file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/public")

# Install Javascript deps
file(GLOB_RECURSE WEBUI_SOURCES LIST_DIRECTORIES false CONFIGURE_DEPENDS
        public/*
        src/*
        index.html
        package-lock.json
        tsconfig.json
        tsconfig.app.json
        tsconfig.node.json
        vite.config.ts.in
)

# Build site
add_custom_target(sacnlogger_webui
        COMMENT "Building Web UI..."
        COMMAND "${npm_PROG}" run build
        SOURCES ${WEBUI_SOURCES}
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        VERBATIM
)
# Ensure make clean does what it should.
set_property(TARGET sacnlogger_webui APPEND PROPERTY ADDITIONAL_CLEAN_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/node_modules"
        "${WEBUI_BUILD_DIR}"
)

install(DIRECTORY "${WEBUI_BUILD_DIR}/" DESTINATION "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/web")
