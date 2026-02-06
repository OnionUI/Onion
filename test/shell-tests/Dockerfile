FROM alpine:3.18

# Install BATS and shell dependencies
RUN apk add --no-cache bash coreutils findutils grep sed unzip git jq 7zip \
    && apk add --no-cache --repository=https://dl-cdn.alpinelinux.org/alpine/edge/testing bats

# Install BATS helper libraries
RUN git clone --depth 1 https://github.com/bats-core/bats-support /opt/bats-support && \
    git clone --depth 1 https://github.com/bats-core/bats-assert /opt/bats-assert && \
    git clone --depth 1 https://github.com/bats-core/bats-file /opt/bats-file

# Create mock Miyoo Mini filesystem
RUN mkdir -p /mnt/SDCARD/.tmp_update/bin \
             /mnt/SDCARD/.tmp_update/config \
             /mnt/SDCARD/App/PackageManager/data \
             /mnt/SDCARD/Emu \
             /mnt/SDCARD/RApp \
             /mnt/SDCARD/Saves/CurrentProfile \
             /mnt/SDCARD/RetroArch/.retroarch

WORKDIR /root/workspace
ENTRYPOINT ["bats"]
