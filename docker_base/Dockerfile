# syntax=docker/dockerfile:1
FROM ubuntu:20.04 AS ubuntu-cmake
RUN apt update
RUN apt install -y make cmake g++ openssh-server
RUN useradd snek -s /snek/client.sh -p '*'
RUN mkdir /home/snek
RUN passwd -d snek
RUN echo "Match User snek" >> /etc/ssh/sshd_config
RUN echo "    PasswordAuthentication yes" >> /etc/ssh/sshd_config
RUN echo "    PermitEmptyPasswords yes" >> /etc/ssh/sshd_config
