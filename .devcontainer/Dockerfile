FROM ubuntu:20.04

# export PATH=/usr/local/cuda-12.1/bin/:$PATH
# cd src/c
# mkdir build
# cd build
# cmake -DCMAKE_CXX_COMPILER=clang++ -DLLVM_DIR=/usr/lib/llvm-10/cmake ..

LABEL maintainer="Vanessasaurus <@vsoch>"

# Match the default user id for a single system so we aren't root
ARG USERNAME=vscode
ARG USER_UID=1000
ARG USER_GID=1000
ENV USERNAME=${USERNAME}
ENV USER_UID=${USER_UID}
ENV USER_GID=${USER_GID}
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt install -y clang llvm-dev libjansson-dev libssl-dev \
    wget bison flex make cmake mpich python3 python3-pip sudo llvm-12 \
    sudo pciutils

# install cuda 12.1 (note uncomment this if you can support on your development machine!)
RUN wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2004/x86_64/cuda-ubuntu2004.pin && \
    mv cuda-ubuntu2004.pin /etc/apt/preferences.d/cuda-repository-pin-600 && \
    wget https://developer.download.nvidia.com/compute/cuda/12.1.0/local_installers/cuda-repo-ubuntu2004-12-1-local_12.1.0-530.30.02-1_amd64.deb && \
    dpkg -i cuda-repo-ubuntu2004-12-1-local_12.1.0-530.30.02-1_amd64.deb && \
    cp /var/cuda-repo-ubuntu2004-12-1-local/cuda-*-keyring.gpg /usr/share/keyrings/ && \
    apt-get update && \
    apt-get -y install cuda && \
    ln -s -T /usr/bin/make /usr/bin/gmake

ENV PATH=/usr/local/cuda-12.1/bin:$PATH

# Python helpers
RUN python3 -m pip install --upgrade pip pytest setuptools flake8 rstfmt black
#    sudo update-alternatives --install /usr/bin/llvm-config llvm-config /usr/bin/llvm-config-12 200

RUN ldconfig

# Add the group and user that match our ids
RUN groupadd -g ${USER_GID} ${USERNAME} && \
    adduser --disabled-password --uid ${USER_UID} --gid ${USER_GID} --gecos "" ${USERNAME} && \
    echo "${USERNAME} ALL=(ALL) NOPASSWD: ALL" > /etc/sudoers
USER $USERNAME
