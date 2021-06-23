FROM ubuntu:bionic

LABEL maintainer="Dong H. Ahn <ahn1@llnl.gov>"

ARG USER=perfflow
ARG UID=2000

# Update pkg caches, install latest pkg utils:
RUN apt-get update \
 && apt-get -qq install -y --no-install-recommends \
        apt-utils \
 && rm -rf /var/lib/apt/lists/*

# Utilities
RUN apt-get update \
 && apt-get -qq install -y --no-install-recommends \
        git \
        sudo \
        vim \
        bison \
        flex \
        jq \
 && rm -rf /var/lib/apt/lists/*

# Compilers, autotools
RUN apt-get update \
 && apt-get -qq install -y --no-install-recommends \
        build-essential \
        autotools-dev \
        libtool \
        autoconf \
        automake \
        make \
        cmake \
        clang-6.0 \
        clang-tidy \
        llvm-dev \
        gcc-8 \
        g++-8 \
 && rm -rf /var/lib/apt/lists/*

# Python
RUN apt-get update \
 && apt-get -qq install -y --no-install-recommends \
        python3-dev \
 && rm -rf /var/lib/apt/lists/*

# Other deps
RUN apt-get update \
 && apt-get -qq install -y --no-install-recommends \
	libssl-dev \
	libjansson-dev \
 && rm -rf /var/lib/apt/lists/*

# Add configured user to image with sudo access:
#
RUN sudo groupadd -g $UID $USER \
   && sudo useradd -g $USER -u $UID -d /home/$USER -m $USER \
   && sudo sh -c "printf \"$USER ALL= NOPASSWD: ALL\\n\" >> /etc/sudoers" \
   && sudo adduser $USER sudo ;

RUN sudo sh -c "printf \"set encoding=UTF-8\" >> /etc/vim/vimrc"

USER $USER
WORKDIR /home/$USER

