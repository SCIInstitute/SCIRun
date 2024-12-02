#syntax=docker/dockerfile:1-labs
FROM ubuntu:24.04

ARG SCIRUN_VERSION="v5.0-beta.2023"

RUN apt-get -y update \
    && DEBIAN_FRONTEND=noninteractive TZ=Etc/UTC apt-get -y install qtbase5-dev libqt5svg5-dev \
    cmake git openssl tzdata build-essential g++ zlib1g-dev libssl-dev libncurses-dev libsqlite3-dev \
    libreadline-dev libtk-img-dev libgdm-dev libdb-dev libpcap-dev \
    && rm -rf /var/cache/apt/archives /var/lib/apt/lists/*

WORKDIR /scirun-src
COPY . /scirun-src/
# RUN git clone --depth 1 --branch ${SCIRUN_VERSION} https://github.com/georgiastuart/SCIRun.git .

WORKDIR /opt/scirun
# TODO: Separate external building from SCIRun
RUN cmake -DQt_PATH=/usr/lib/x86_64-linux-gnu/cmake \
    -DCMAKE_BUILD_TYPE=Release /scirun-src/Superbuild && make

# Make the internal python usable externally
ENV LD_LIBRARY_PATH=/opt/scirun/Externals/Install/Python_external/lib:$LD_LIBRARY_PATH
ENV PATH=/opt/scirun/SCIRun:/opt/scirun/Externals/Install/Python_external/bin:$PATH 
RUN python3 -m ensurepip && python3 -m pip install --upgrade pip && python3 -m venv /opt/venv

WORKDIR /opt/scripts 
COPY <<EOF /opt/scripts/entrypoint.sh
set -e 
. /opt/venv/bin/activate
exec "\$@"
EOF

RUN chmod a+x /opt/scripts/entrypoint.sh

WORKDIR /data
ENTRYPOINT [ "/opt/scripts/entrypoint.sh" ]
CMD ["/opt/scirun/SCIRun/SCIRun"]