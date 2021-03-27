FROM crystallang/crystal:1.0.0
ARG llvm_ver=11
RUN apt-get update && apt-get install -y \
  wget \
  lsb-release \
  software-properties-common


WORKDIR /tmp

RUN wget https://apt.llvm.org/llvm.sh && chmod +x llvm.sh && ./llvm.sh ${llvm_ver}
RUN ln -s /usr/bin/llvm-config-${llvm_ver} /usr/bin/llvm-config

RUN apt-get update && apt-get install -y \
  libclang-${llvm_ver}-dev \
  libclang-cpp${llvm_ver}-dev \
  cmake \
  libgl1-mesa-dev \
  libglu1-mesa-dev

ENV CLANG_BINARY=/usr/lib/llvm-${llvm_ver}/bin/clang++