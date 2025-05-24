if (NOT DEFINED IMG_IN)
    message(FATAL_ERROR "Set IMG_IN to the input image file path.")
endif ()
if (NOT IS_READABLE "${IMG_IN}")
    message(FATAL_ERROR "Not readable: ${IMG_IN}")
endif ()

if (NOT DEFINED IMG_OUT)
    message(FATAL_ERROR "Set IMG_OUT to the output compressed file path.")
endif ()

# Ensure parent directory exists.
cmake_path(GET IMG_OUT PARENT_PATH PARENT_DIR)
file(MAKE_DIRECTORY "${PARENT_DIR}")

# Compress file.
file(ARCHIVE_CREATE OUTPUT "${IMG_OUT}" PATHS "${IMG_IN}" FORMAT raw COMPRESSION Zstd VERBOSE)
