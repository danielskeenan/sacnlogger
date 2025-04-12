# Based on https://github.com/jonnymacs/rpi-image-gen-example

FROM debian:bookworm AS base

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

ENTRYPOINT ["/usr/bin/rpi_imagegen"]
