# Build happens in a Docker container.
find_program(DOCKER_PROG docker REQUIRED)

# Setup output directories.
set(RASPI5_IMG_DIR "${CMAKE_CURRENT_BINARY_DIR}/deploy")
file(MAKE_DIRECTORY "${RASPI5_IMG_DIR}")
file(CHMOD "${RASPI5_IMG_DIR}" DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_WRITE WORLD_EXECUTE)
set(RASPI5_IMG "${RASPI5_IMG_DIR}/${PROJECT_NAME}-${PROJECT_VERSION}-raspi5.img")

# Determine image details.
configure_file(build.options "${CMAKE_CURRENT_BINARY_DIR}/build.options" @ONLY)

# Build process depends on custom overlay, so mark it out of date if the overlay changes.
file(GLOB_RECURSE OVERLAY_FILES LIST_DIRECTORIES false CONFIGURE_DEPENDS
        custom/*
        docker-compose.yaml
        Dockerfile
        entrypoint.sh
)

# Build image.
add_custom_command(OUTPUT "${RASPI5_IMG}"
        COMMAND ${CMAKE_COMMAND} -E env BUILDKIT_PROGRESS=plain "${DOCKER_PROG}" compose run
        --build
        --volume "${CMAKE_CURRENT_BINARY_DIR}/build.options:/home/imagegen/build.options"
        --volume "${RASPI5_IMG_DIR}:/home/imagegen/work/deploy"
        rpi_imagegen -o /home/imagegen/build.options
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        USES_TERMINAL
        DEPENDS ${OVERLAY_FILES}
)
add_custom_target(${PROJECT_NAME}_raspi5 DEPENDS "${RASPI5_IMG}")
