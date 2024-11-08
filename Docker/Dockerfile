# 베이스 이미지로 Ubuntu 최신 버전 사용
FROM ubuntu:latest

# 기본 설정
ENV DEBIAN_FRONTEND=noninteractive

# 패키지 업데이트 및 기본 도구 설치
RUN apt update && \
    apt upgrade -y && \
    apt install -y \
    build-essential \
    net-tools \
    htop \
    git \
    zip \
    unzip \
    vim \
    nano \
    wget \
    curl \
    python3 \
    python3-pip \
    ufw && \
    apt clean && rm -rf /var/lib/apt/lists/*
    apt-get install -y iputils-ping

# 기본 작업 디렉토리 설정
WORKDIR /workspace

# 컨테이너 시작 시 사용할 기본 쉘 설정
CMD ["/bin/bash"]
