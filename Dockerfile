FROM fedora:42

ARG ANTLR4_VERSION=13.2

WORKDIR /app
COPY . .

RUN mkdir build

ADD https://www.antlr.org/download/antlr-4.$ANTLR4_VERSION-complete.jar build/antlr-4.$ANTLR4_VERSION-complete.jar

RUN dnf update -y && dnf install -y cmake gcc-g++ antlr4-cpp-runtime-devel java-21-openjdk-headless

RUN cmake -S . -B build
RUN cmake --build build

ENTRYPOINT [ "/app/build/cscript" ]

