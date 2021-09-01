# syntax=docker/dockerfile:1
FROM snek_base AS ubuntu-builder
COPY . /snek
WORKDIR /snek
RUN mkdir build
WORKDIR /snek/build
RUN cmake ..
RUN cmake --build .

FROM ubuntu-builder AS ubuntu-runner
CMD service ssh start ; /snek/build/snekMMO -p 21337 > snekLog.txt
