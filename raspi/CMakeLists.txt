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
set(RASPI5_DEB "${CMAKE_CURRENT_BINARY_DIR}/pkgs/${PROJECT_NAME}_${PROJECT_VERSION}_arm64.deb")
add_custom_command(OUTPUT "${RASPI5_DEB}"
        COMMAND ${CMAKE_COMMAND} -E env BUILDKIT_PROGRESS=plain "${DOCKER_PROG}" build
        --file "${PROJECT_SOURCE_DIR}/raspi/crossbuild_arm64.dockerfile"
        --build-arg build_type=$<LOWER_CASE:$<CONFIG>>
        --tag sacnlogger_crossbuild_arm64:$<LOWER_CASE:$<CONFIG>>
        .
        COMMAND ${CMAKE_COMMAND} -E env BUILDKIT_PROGRESS=plain "${DOCKER_PROG}" run
        --workdir /work/cmake-build-$<LOWER_CASE:$<CONFIG>>
        --name crossbuild_arm64_$<LOWER_CASE:$<CONFIG>>
        sacnlogger_crossbuild_arm64:$<LOWER_CASE:$<CONFIG>>
        cpack -B "./packages" -G DEB
        COMMAND ${CMAKE_COMMAND} -E env BUILDKIT_PROGRESS=plain "${DOCKER_PROG}" cp
        "crossbuild_arm64_$<LOWER_CASE:$<CONFIG>>:/work/cmake-build-$<LOWER_CASE:$<CONFIG>>/packages/${PROJECT_NAME}_${PROJECT_VERSION}_arm64.deb"
        "${RASPI5_DEB}"
        COMMAND ${CMAKE_COMMAND} -E env BUILDKIT_PROGRESS=plain "${DOCKER_PROG}" container rm "crossbuild_arm64_$<LOWER_CASE:$<CONFIG>>"
        VERBATIM
        DEPENDS ${PROJECT_NAME}
        USES_TERMINAL
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)
add_custom_command(OUTPUT "${RASPI5_IMG}"
        COMMAND ${CMAKE_COMMAND} -E env BUILDKIT_PROGRESS=plain "${DOCKER_PROG}" build
        --file "${PROJECT_SOURCE_DIR}/raspi/rpi_imagegen.dockerfile"
        --tag rpi_imagegen:latest
        .
        COMMAND ${CMAKE_COMMAND} -E env BUILDKIT_PROGRESS=plain "${DOCKER_PROG}" run
        --workdir /work/cmake-build-$<LOWER_CASE:$<CONFIG>>
        --volume "${CMAKE_CURRENT_BINARY_DIR}/build.options:/home/imagegen/build.options"
        --volume "${RASPI5_DEB}:/home/imagegen/custom/pkgs/${PROJECT_NAME}_${PROJECT_VERSION}_arm64.deb"
        --volume "${RASPI5_IMG_DIR}:/home/imagegen/work/deploy"
        --privileged
        --name rpi_imagegen_$<LOWER_CASE:$<CONFIG>>
        rpi_imagegen:latest
        rpi_imagegen -o /home/imagegen/build.options
        COMMAND ${CMAKE_COMMAND} -E env BUILDKIT_PROGRESS=plain "${DOCKER_PROG}" container rm "rpi_imagegen_$<LOWER_CASE:$<CONFIG>>"
        VERBATIM
        DEPENDS "${RASPI5_DEB}" ${OVERLAY_FILES}
        USES_TERMINAL
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)
add_custom_target(${PROJECT_NAME}_raspi5 DEPENDS "${RASPI5_IMG}")
