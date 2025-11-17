#!/usr/bin/env bash
set -euo pipefail

echo "[run_epoll.sh] Starting epoll_server and test client..."

# epoll server -> background
epoll_server & 
SERVER_PID=$!

# wait for initializtion
sleep 0.5

echo "[run_epoll.sh] Sending test message via netcat..."
echo "hello from client" | nc 127.0.0.1 9000

# 추가로 여러 줄 보내도 됨
echo "second line" | nc 127.0.0.1 9000

echo "[run_epoll.sh] Waiting for server to exit..."
wait "${SERVER_PID}" || true

echo "[run_epoll.sh] Done."