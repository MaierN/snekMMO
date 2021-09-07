# syntax=docker/dockerfile:1

FROM ubuntu:20.04

RUN apt update
RUN apt install -y make cmake g++ openssh-server

RUN useradd snek -s /snek/client.sh -p '*'
RUN mkdir /home/snek
RUN passwd -d snek
RUN echo "Match User snek" >> /etc/ssh/sshd_config
RUN echo "    PasswordAuthentication yes" >> /etc/ssh/sshd_config
RUN echo "    PermitEmptyPasswords yes" >> /etc/ssh/sshd_config

COPY . /snek
RUN echo "#!/bin/bash" > /snek/client.sh
RUN echo "/snek/build/snekMMO -i localhost -p 21337" >> /snek/client.sh
RUN chmod +x /snek/client.sh
WORKDIR /snek
RUN mkdir build
WORKDIR /snek/build
RUN cmake ..
RUN cmake --build .

CMD service ssh start ; /snek/build/snekMMO -p 21337 > snekLog.txt 2>snekErr.txt
