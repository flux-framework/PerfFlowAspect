FROM ubuntu:20.04

LABEL maintainer="Vanessasaurus <@vsoch>"

# docker build -t perf-flow-aspect .

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && \
    apt-get install -y clang llvm-dev libjansson-dev libssl-dev wget bison flex make cmake python3 python3-pip llvm-12 && \
    apt-get install -y mpich sudo pciutils

RUN wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2004/x86_64/cuda-ubuntu2004.pin && \
    mv cuda-ubuntu2004.pin /etc/apt/preferences.d/cuda-repository-pin-600 && \
    wget https://developer.download.nvidia.com/compute/cuda/12.1.0/local_installers/cuda-repo-ubuntu2004-12-1-local_12.1.0-530.30.02-1_amd64.deb && \
    dpkg -i cuda-repo-ubuntu2004-12-1-local_12.1.0-530.30.02-1_amd64.deb && \cp /var/cuda-repo-ubuntu2004-12-1-local/cuda-*-keyring.gpg /usr/share/keyrings/ && \
    apt-get update && \
    apt-get -y install cuda && \
    ln -s -T /usr/bin/make /usr/bin/gmake

ENV PATH=/usr/local/cuda-12.1/bin:$PATH

# Python helpers
RUN python3 -m pip install --upgrade pip pytest setuptools flake8 rstfmt black

# Install PerfFlowAspect to /usr (so root sees it too)
WORKDIR /code
COPY . /code
RUN cd src/c && \
    mkdir -p build install && \
    cd build && \
    cmake -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_INSTALL_PREFIX=/usr -DLLVM_DIR=/usr/lib/llvm-10/cmake .. && \
    cmake ${CMAKE_OPTS} .. && \
    make VERBOSE=1 && \
    make install
    