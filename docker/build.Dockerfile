# Halcyon Chromium builder image.
#
# Based on the ungoogled-chromium-portablelinux builder (which has been
# proven on GitHub Actions free runners across 70+ successful builds).
# The image is intentionally a SINGLE, REUSABLE build environment used by
# every job in the chunked build pipeline: prep, build_part_01..10, and
# package.
#
# What's inside:
#   - Debian Trixie base (matches the Chromium build requirements for
#     glibc, g++, etc.)
#   - All Chromium build dependencies listed in chromium's
#     build/install-build-deps.sh, pruned to the Linux desktop chrome
#     target.
#   - Node.js + TypeScript + esbuild (needed by Chromium's WebUI
#     build pipeline)
#   - depot_tools (for `gn` and `gclient` — though we primarily use the
#     source tarball, we still need gn)
#   - A non-root `builder` user (uid 1000) that matches the GitHub Actions
#     runner uid, so file permissions on the bind-mounted workspace work
#     correctly.
#
# Build:
#   docker build -t halcyon-builder:trixie -f docker/build.Dockerfile .
#
# Or pull from GHCR (the workflow does this):
#   docker pull ghcr.io/magicdippyegg/chromium-z.ai/halcyon-builder:latest

FROM debian:trixie-slim

ARG NODE_VERSION=24

# Set Go paths under /tmp to avoid ownership/permission issues in CI.
ENV GOPATH=/tmp/go
ENV GOCACHE=/tmp/go-build-cache

# Non-interactive apt.
RUN echo 'debconf debconf/frontend select Noninteractive' | debconf-set-selections && \
    export DEBIAN_FRONTEND=noninteractive

# Base apt install + system upgrade.
RUN apt-get -y update && apt-get -y upgrade && \
    apt-get -y install -y --no-install-recommends \
      apt-transport-https ca-certificates curl gnupg xz-utils zstd \
      sudo less file

# Node.js LTS (for Chromium's WebUI / TypeScript build pipeline).
RUN apt-get -y update && apt-get -y install -y apt-transport-https ca-certificates curl gnupg && \
    curl -fsSL "https://deb.nodesource.com/setup_${NODE_VERSION}.x" | bash - && \
    apt-get -y update && apt-get -y install -y nodejs && \
    npm update -g npm && \
    npm -g install typescript esbuild

# Chromium build dependencies (Linux desktop chrome target).
# This list is the union of:
#   - chromium/build/install-build-deps.sh --no-suggest --no-arm
#   - the package list from ungoogled-chromium-portablelinux/docker/build.Dockerfile
# Pruned to what the Halcyon build actually needs (no Android SDK, no
# NaCl, no ChromeOS deps).
RUN apt-get -y install -y --no-install-recommends \
      bison \
      clang \
      clang-format \
      cmake \
      flex \
      git \
      golang \
      gperf \
      libasound2-dev \
      libatk1.0-dev \
      libatk-bridge2.0-dev \
      libcairo2-dev \
      libcap-dev \
      libcups2-dev \
      libdbus-1-dev \
      libdrm-dev \
      libegl1-mesa-dev \
      libelf-dev \
      libevent-dev \
      libexif-dev \
      libffi-dev \
      libflac-dev \
      libfontconfig1-dev \
      libfreetype6-dev \
      libgbm-dev \
      libgcrypt20-dev \
      libglib2.0-dev \
      libgl1-mesa-dev \
      libgles2-mesa-dev \
      libglew-dev \
      libglu1-mesa-dev \
      libgtk-3-dev \
      libhunspell-dev \
      libjpeg-dev \
      libjsoncpp-dev \
      libkrb5-dev \
      liblcms2-dev \
      libminizip-dev \
      libmodpbase64-dev \
      libnspr4-dev \
      libnss3-dev \
      libopenjp2-7-dev \
      libopus-dev \
      libpam0g-dev \
      libpango1.0-dev \
      libpci-dev \
      libpipewire-0.3-dev \
      libpng-dev \
      libpulse-dev \
      libre2-dev \
      libsnappy-dev \
      libspeechd-dev \
      libssl-dev \
      libudev-dev \
      libusb-1.0-0-dev \
      libva-dev \
      libvpx-dev \
      libwebp-dev \
      libx11-xcb-dev \
      libxcb-dri3-dev \
      libxcomposite-dev \
      libxdamage-dev \
      libxext-dev \
      libxfixes-dev \
      libxkbcommon-dev \
      libxrandr-dev \
      libxrender-dev \
      libxshmfence-dev \
      libxslt1-dev \
      libxss-dev \
      libxt-dev \
      libxtst-dev \
      mesa-common-dev \
      ninja-build \
      pkg-config \
      python3 \
      python3-httplib2 \
      python3-jinja2 \
      python3-pyparsing \
      python3-setuptools \
      python3-six \
      python3-venv \
      python-is-python3 \
      rsync \
      uuid-dev \
      valgrind \
      x11-apps \
      xfonts-base \
      xvfb \
      yasm

# depot_tools — for `gn` and (rarely) `gclient`. We clone it but DON'T
# run gclient sync — the Halcyon build uses the source tarball.
RUN git clone --depth 1 https://chromium.googlesource.com/chromium/tools/depot_tools.git /opt/depot_tools && \
    echo 'DEPOT_TOOLS_UPDATE=0' > /opt/depot_tools/env && \
    echo 'DEPOT_TOOLS_BOOTSTRAP_PYTHON3=1' >> /opt/depot_tools/env
ENV PATH="/opt/depot_tools:${PATH}"

# Build a `builder` user (uid 1000) that matches the GitHub Actions runner
# uid. This makes the bind-mounted workspace files readable/writable
# inside the container without permission errors.
RUN groupadd -g 1000 builder && \
    useradd -d /home/builder -g 1000 -u 1000 -m builder && \
    # Grant builder passwordless sudo (needed for apt installs in CI).
    echo 'builder ALL=(ALL) NOPASSWD: ALL' > /etc/sudoers.d/builder

USER builder
WORKDIR /repo

# Default command: do nothing. The workflow invokes
# `docker run halcyon-builder bash /repo/scripts/build.sh` with the
# workspace bind-mounted at /repo.
CMD ["bash", "-c", "echo 'halcyon-builder ready. mount /repo and run scripts/build.sh' && sleep infinity"]
