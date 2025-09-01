FROM fedora:42

WORKDIR /app
COPY . .

RUN mkdir build

RUN dnf update -y && dnf install -y cmake gcc-g++ antlr4-cpp-runtime-devel java-21-openjdk-headless catch2-devel vim 

RUN cmake -S . -B build
RUN cmake --build build

ENTRYPOINT [ "/bin/bash" ]

