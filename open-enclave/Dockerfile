FROM ubuntu:20.04 AS base

ARG SIM 

ENV DEBIAN_FRONTEND=noninteractive
RUN apt update && apt -y install dkms wget build-essential

# RUN wget https://download.01.org/intel-sgx/sgx-linux/2.17/distro/ubuntu20.04-server/sgx_linux_x64_driver_1.41.bin -O sgx_linux_x64_driver.bin && \
#     chmod +x sgx_linux_x64_driver.bin && \
#     ./sgx_linux_x64_driver.bin

RUN echo 'deb [arch=amd64] https://download.01.org/intel-sgx/sgx_repo/ubuntu focal main' | tee /etc/apt/sources.list.d/intel-sgx.list && \
    wget -qO - https://download.01.org/intel-sgx/sgx_repo/ubuntu/intel-sgx-deb.key | apt-key add - && \
    echo "deb http://apt.llvm.org/focal/ llvm-toolchain-focal-11 main" | tee /etc/apt/sources.list.d/llvm-toolchain-focal-11.list && \
    wget -qO - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -&& \
    echo "deb [arch=amd64] https://packages.microsoft.com/ubuntu/20.04/prod focal main" | tee /etc/apt/sources.list.d/msprod.list && \
    wget -qO - https://packages.microsoft.com/keys/microsoft.asc | apt-key add -&& \
    apt update

RUN apt -y install clang-11 libssl-dev gdb libsgx-enclave-common libsgx-quote-ex libprotobuf17 libsgx-dcap-ql libsgx-dcap-ql-dev az-dcap-client open-enclave

FROM base AS sim

RUN echo "source /opt/openenclave/share/openenclave/openenclaverc && export OE_SIMULATION=1" >> /etc/bash.bashrc