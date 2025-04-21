FROM debian:bookworm-slim AS crossbuild_arm64

# Setup arm64 compilation.
RUN dpkg --add-architecture arm64 &&\
    apt-get update &&\
    apt-get install -y \
    build-essential \
    ca-certificates \
    crossbuild-essential-arm64 \
    curl \
    dpkg-dev \
    file \
    git \
    gpg \
    pkg-config \
    qemu-user \
    qemu-user-static \
    uuid-dev:arm64 \
    wget \
    zip

# Get a newer cmake than the one in the repo.
RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null &&\
    echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ jammy main' | tee /etc/apt/sources.list.d/kitware.list >/dev/null &&\
    apt-get update &&\
    rm /usr/share/keyrings/kitware-archive-keyring.gpg &&\
    apt-get install -y kitware-archive-keyring cmake

# Setup vcpkg.
ENV VCPKG_DEFAULT_TRIPLET=arm64-linux
ENV VCPKG_ROOT=/root/.vcpkg
# VCPKG Caching (https://learn.microsoft.com/en-us/vcpkg/users/binarycaching)
ENV VCPKG_DEFAULT_BINARY_CACHE=/root/.vcpkg-cache
RUN mkdir ${VCPKG_DEFAULT_BINARY_CACHE}
RUN git clone https://github.com/microsoft/vcpkg.git ${VCPKG_ROOT} &&\
    cd ${VCPKG_ROOT} && \
    ./bootstrap-vcpkg.sh

# Create custom triplet to get vcpkg to work with crosscompiler.
ENV CROSS_TRIPLE=aarch64-linux-gnu
ENV CROSS_ROOT=/usr/${CROSS_TRIPLE}
ENV AS=/usr/bin/${CROSS_TRIPLE}-as \
    AR=/usr/bin/${CROSS_TRIPLE}-ar \
    CC=/usr/bin/${CROSS_TRIPLE}-gcc \
    CPP=/usr/bin/${CROSS_TRIPLE}-cpp \
    CXX=/usr/bin/${CROSS_TRIPLE}-g++ \
    LD=/usr/bin/${CROSS_TRIPLE}-ld \
    LD_LIBRARY_PATH=${CROSS_ROOT}/lib
ENV VCPKG_OVERLAY_TRIPLETS=/root/triplets
COPY raspi/arm64-linux-gnu-crossbuild.cmake /root/
RUN mkdir ${VCPKG_OVERLAY_TRIPLETS} &&\
    cp ${VCPKG_ROOT}/triplets/community/arm64-linux.cmake ${VCPKG_OVERLAY_TRIPLETS}/arm64-linux-crossbuild.cmake &&\
    echo "set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE /root/arm64-linux-gnu-crossbuild.cmake)" >> "${VCPKG_OVERLAY_TRIPLETS}/arm64-linux-crossbuild.cmake"

COPY . /work/
WORKDIR /work

CMD ["sh"]
