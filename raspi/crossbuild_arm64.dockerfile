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
    sudo \
    wget \
    zip \
    uuid-dev:arm64

# Get a newer cmake than the one in the repo.
RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null &&\
    echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ jammy main' | tee /etc/apt/sources.list.d/kitware.list >/dev/null &&\
    apt-get update &&\
    rm /usr/share/keyrings/kitware-archive-keyring.gpg &&\
    apt-get install -y kitware-archive-keyring cmake

# User setup.
ENV USER=crossbuild
RUN /bin/bash -c 'echo "${USER} ALL=(ALL:ALL) NOPASSWD: ALL" > /etc/sudoers.d/nopasswd'
RUN useradd -u 1000 -ms /bin/bash "$USER" && echo "${USER}:${USER}" | chpasswd && adduser ${USER} sudo # only add to sudo if build scripts require it
USER ${USER}
WORKDIR /home/${USER}

# Setup vcpkg.
ENV VCPKG_DEFAULT_TRIPLET=arm64-linux
ENV VCPKG_ROOT=/home/${USER}/.vcpkg
# VCPKG Caching (https://learn.microsoft.com/en-us/vcpkg/users/binarycaching)
ENV VCPKG_DEFAULT_BINARY_CACHE=/home/${USER}/.vcpkg-cache
RUN mkdir -p $VCPKG_DEFAULT_BINARY_CACHE
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
ENV VCPKG_OVERLAY_TRIPLETS=/home/${USER}/triplets
COPY raspi/arm64-linux-gnu-crossbuild.cmake /home/${USER}/
RUN mkdir ${VCPKG_OVERLAY_TRIPLETS} &&\
    cp ${VCPKG_ROOT}/triplets/community/arm64-linux.cmake ${VCPKG_OVERLAY_TRIPLETS}/arm64-linux-crossbuild.cmake &&\
    echo "set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE /home/${USER}/arm64-linux-gnu-crossbuild.cmake)" >> "${VCPKG_OVERLAY_TRIPLETS}/arm64-linux-crossbuild.cmake"

RUN git config --global --add safe.directory /home/${USER}/work
COPY . /home/${USER}/work/
WORKDIR /home/${USER}/work

CMD ["sh"]
