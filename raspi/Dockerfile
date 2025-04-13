FROM debian:trixie-slim AS crossbuild_arm64

# Setup arm64 compilation.
RUN dpkg --add-architecture arm64 &&\
    apt-get update &&\
    apt-get install -y \
    build-essential \
    ca-certificates \
    cmake \
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

# Setup vcpkg.
ENV VCPKG_DEFAULT_TRIPLET=arm64-linux
ENV VCPKG_ROOT=/root/.vcpkg
# VCPKG Caching (https://learn.microsoft.com/en-us/vcpkg/users/binarycaching)
ENV VCPKG_DEFAULT_BINARY_CACHE=/root/.vcpkg-cache
RUN mkdir ${VCPKG_DEFAULT_BINARY_CACHE}
VOLUME ${VCPKG_DEFAULT_BINARY_CACHE}
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

ARG build_type=release
RUN cmake \
    -B "cmake-build-${build_type}" \
    -DCMAKE_TOOLCHAIN_FILE="${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" \
    -DVCPKG_TARGET_TRIPLET=arm64-linux-crossbuild \
    -DCMAKE_BUILD_TYPE=${build_type} \
    -S . \
    -DBUILD_DOC=Off \
    -DBUILD_TESTING=Off \
    -DBUILD_PACKAGE=On

RUN cmake --build "cmake-build-${build_type}" --config ${build_type}

RUN cd "cmake-build-${build_type}" &&\
    cpack -B "./packages" -G DEB

FROM debian:bookworm AS rpi_imagegen
# Based on https://github.com/jonnymacs/rpi-image-gen-example

RUN apt-get update && apt-get install --no-install-recommends -y \
    arch-test \
    bc \
    bdebstrap \
    binfmt-support \
    btrfs-progs \
    build-essential \
    ca-certificates \
    crudini \
    curl \
    dbus-user-session \
    dctrl-tools \
    debootstrap \
    dirmngr \
    dosfstools \
    fdisk \
    file \
    genimage \
    git \
    gpg \
    gpg-agent \
    jq \
    kmod \
    libarchive-tools \
    libcap2-bin \
    mmdebstrap \
    mtools \
    parted \
    pigz \
    podman \
    pv \
    python-is-python3 \
    qemu-user-static \
    quilt \
    rsync \
    slirp4netns \
    squashfs-tools \
    sudo \
    uidmap \
    uuid-runtime \
    xxd \
    zerofree \
    zip \
    zstd \
  && rm -rf /var/lib/apt/lists/*

RUN curl -fsSL https://archive.raspberrypi.com/debian/raspberrypi.gpg.key \
  | gpg --dearmor > /usr/share/keyrings/raspberrypi-archive-keyring.gpg

COPY raspi/entrypoint.sh "/usr/bin/rpi_imagegen"
RUN chmod +x /usr/bin/rpi_imagegen

ENV USER imagegen
RUN /bin/bash -c 'echo "${USER} ALL=(ALL:ALL) NOPASSWD: ALL" > /etc/sudoers.d/nopasswd'
RUN useradd -u 4000 -ms /bin/bash "$USER" && echo "${USER}:${USER}" | chpasswd && adduser ${USER} sudo # only add to sudo if build scripts require it
USER ${USER}
WORKDIR /home/${USER}

VOLUME /home/${USER}/work
RUN mkdir -p /home/${USER}/work/deploy

ARG RPIIG_GIT_SHA=ba410bccd3f690a49cb8ec7a724cb59d08a4257e
RUN git clone --no-checkout https://github.com/raspberrypi/rpi-image-gen.git && cd rpi-image-gen && git checkout ${RPIIG_GIT_SHA}

COPY raspi/custom /home/imagegen/custom
COPY --from=crossbuild_arm64 /work/cmake-build-*/packages/sacnlogger_*_arm64.deb /home/imagegen/custom/pkgs/

ENTRYPOINT ["/usr/bin/rpi_imagegen"]
