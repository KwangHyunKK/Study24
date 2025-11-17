FROM ubuntu::22.04

# 비대화형 설정
ENV DEBIAN_FRONTEND=nointeractive

# 필요한 패키지 설치 : g++ 등
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        g++ make netcat-openbsd iproute2 \
        && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# epoll C++ server code만 미리 복사해서 빌드
COPY epoll_server.cpp /app/epoll_server.cpp

RUN g++ -std=c++17 -O2 -Wall -Wextra -o /usr/local/bin/epoll_server /app/epoll_server.cpp

# run_epoll.sh runtime mount
CMD ["bash"]