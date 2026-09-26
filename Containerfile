FROM registry.access.redhat.com/ubi9/ubi:latest

RUN dnf install -y \
        gcc \
        make \
        libarchive-devel \
        zstd \
        gdb \
    && dnf clean all

WORKDIR /src

CMD ["/bin/bash"]