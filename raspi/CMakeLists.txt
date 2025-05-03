# Build happens in a Docker container.
find_program(DOCKER_PROG docker REQUIRED)

# Build process depends on custom overlay, so mark it out of date if the overlay changes.
file(GLOB_RECURSE OVERLAY_FILES LIST_DIRECTORIES false CONFIGURE_DEPENDS
        custom/*
        *.dockerfile
        arm64-linux-gnu-crossbuild.cmake
        build.options
        entrypoint.sh
)

# Build package.
set(RASPI5_PKGS_DIR "${CMAKE_CURRENT_BINARY_DIR}/cross-cmake-build/packages")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/cross-cmake-build")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/cross-vcpkg-cache")
set(RASPI5_DEB "${RASPI5_PKGS_DIR}/${PROJECT_NAME}_${PROJECT_VERSION}_arm64.deb")
set(RASPI5_PKGS
        "${RASPI5_DEB}"
)
add_custom_command(OUTPUT ${RASPI5_PKGS}
        COMMAND ${CMAKE_COMMAND} -E env BUILDKIT_PROGRESS=plain "${DOCKER_PROG}" build
        --file "${PROJECT_SOURCE_DIR}/raspi/crossbuild_arm64.dockerfile"
        --tag sacnlogger_crossbuild_arm64:latest
        .
        COMMAND ${CMAKE_COMMAND} -E env BUILDKIT_PROGRESS=plain "${DOCKER_PROG}" run
        --volume "${CMAKE_CURRENT_BINARY_DIR}/cross-cmake-build:/home/crossbuild/work/cmake-build"
        --volume "${CMAKE_CURRENT_BINARY_DIR}/cross-vcpkg-cache:/home/crossbuild/.vcpkg-cache"
        --rm
        sacnlogger_crossbuild_arm64:latest
        cmake -B "cmake-build" -S .
        -DCMAKE_TOOLCHAIN_FILE=/home/crossbuild/.vcpkg/scripts/buildsystems/vcpkg.cmake
        -DVCPKG_TARGET_TRIPLET=arm64-linux-crossbuild
        -DCMAKE_BUILD_TYPE=$<CONFIG>
        -DBUILD_DOC=Off
        -DBUILD_TESTING=Off
        -DBUILD_PACKAGE=On
        COMMAND ${CMAKE_COMMAND} -E env BUILDKIT_PROGRESS=plain "${DOCKER_PROG}" run
        --volume "${CMAKE_CURRENT_BINARY_DIR}/cross-cmake-build:/home/crossbuild/work/cmake-build"
        --volume "${CMAKE_CURRENT_BINARY_DIR}/cross-vcpkg-cache:/home/crossbuild/.vcpkg-cache"
        --rm
        sacnlogger_crossbuild_arm64:latest
        cmake --build "cmake-build" --config $<CONFIG>
        COMMAND ${CMAKE_COMMAND} -E env BUILDKIT_PROGRESS=plain "${DOCKER_PROG}" run
        --volume "${CMAKE_CURRENT_BINARY_DIR}/cross-cmake-build:/home/crossbuild/work/cmake-build"
        --volume "${CMAKE_CURRENT_BINARY_DIR}/cross-vcpkg-cache:/home/crossbuild/.vcpkg-cache"
        --rm
        --workdir /home/crossbuild/work/cmake-build
        sacnlogger_crossbuild_arm64:latest
        cpack -B "./packages" -G DEB
        VERBATIM
        DEPENDS ${PROJECT_NAME}
        USES_TERMINAL
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)
add_custom_target(${PROJECT_NAME}_raspi5_deb DEPENDS ${RASPI5_PKGS})

# Build image.
foreach (RASPI5_PKG IN LISTS RASPI5_PKGS)
    cmake_path(GET RASPI5_PKG FILENAME RASPI5_PKG_FILENAME)
    list(APPEND RASPI5_PKGS_VOLUMES "--volume" "${RASPI5_PKG}:/home/imagegen/custom/pkgs/${RASPI5_PKG_FILENAME}")
endforeach ()
set(RASPI5_IMG_DIR "${CMAKE_CURRENT_BINARY_DIR}/deploy")
file(MAKE_DIRECTORY "${RASPI5_IMG_DIR}")
set(RASPI5_IMG "${RASPI5_IMG_DIR}/${PROJECT_NAME}-${PROJECT_VERSION}-raspi5.img")
add_custom_command(OUTPUT "${RASPI5_IMG}"
        COMMAND ${CMAKE_COMMAND} -E env BUILDKIT_PROGRESS=plain "${DOCKER_PROG}" build
        --file "${PROJECT_SOURCE_DIR}/raspi/rpi_imagegen.dockerfile"
        --tag rpi_imagegen:latest
        .
        COMMAND ${CMAKE_COMMAND} -E env BUILDKIT_PROGRESS=plain "${DOCKER_PROG}" run
        --volume "${CMAKE_CURRENT_BINARY_DIR}/build_$<LOWER_CASE:$<CONFIG>>.options:/home/imagegen/build.options"
        ${RASPI5_PKGS_VOLUMES}
        --volume "${RASPI5_IMG_DIR}:/home/imagegen/work/deploy"
        --rm
        --privileged
        rpi_imagegen:latest
        -c sacnlogger$<IF:$<CONFIG:Debug>,_debug,_release> -o /home/imagegen/build.options
        VERBATIM
        COMMAND_EXPAND_LISTS
        DEPENDS ${RASPI5_PKGS} ${OVERLAY_FILES}
        USES_TERMINAL
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)
add_custom_target(${PROJECT_NAME}_raspi5 DEPENDS "${RASPI5_IMG}")
# Determine image details.
# Need to do this in two steps as configure_file doesn't know about generator expressions and file(GENERATE) doesn't
# know about variable substitution. (https://discourse.cmake.org/t/configuring-a-file-that-has-both-substitution-and-generator-expressions/3064)
configure_file(build.options "${CMAKE_CURRENT_BINARY_DIR}/build.options.in" @ONLY NEWLINE_STYLE UNIX)
file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/build_$<LOWER_CASE:$<CONFIG>>.options"
        INPUT "${CMAKE_CURRENT_BINARY_DIR}/build.options.in"
        TARGET ${PROJECT_NAME}_raspi5
        NEWLINE_STYLE UNIX
)
