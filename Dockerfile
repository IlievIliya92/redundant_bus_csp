FROM ubuntu
ARG DEBIAN_FRONTEND=noninteractive
MAINTAINER "Iliya Iliev"

# Install required packages
RUN apt-get update && \
     apt-get install --no-install-recommends --no-install-suggests -y sudo build-essential git cmake build-essential can-utils libyaml-dev pkg-config libsocketcan-dev libzmq3-dev net-tools iproute2 can-utils kmod

RUN useradd -m docker_user && \
    echo 'docker_user:docker_user_pass' | chpasswd
RUN usermod -aG sudo docker_user
RUN echo 'docker_user ALL=(ALL) NOPASSWD:ALL' > /etc/sudoers.d/docker_user
WORKDIR /home/docker_user

# Build
RUN git config --global http.sslVerify false && \
     git clone https://github.com/IlievIliya92/redundant_bus_csp.git && \
     cd redundant_bus_csp && \
     git submodule init && \
     git submodule update && \
     cmake -B ./build -DCMAKE_BUILD_TYPE=Release && \
     cmake --build ./build --config Release

USER docker_user
SHELL ["/bin/bash", "-c"]
CMD ["bash"]
