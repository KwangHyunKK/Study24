## epoll-ebpf-demo

### High-Level Concept

- Host
  - execute container using python script
  - eBPF for observing epoll_wait
- Container
  - C++ based epoll TCP echo server
  - run_epoll.sh script will execute epoll server and test client.
- eBPF
  - bcc library based python script -> ``__x64_sys_epoll_wait`` kprobe

### Directory Layout

```
epoll-ebpf-demo/
├── orchestrator.py          # docker를 호출하는 Python 스크립트
├── ebpf/
│   └── trace_epoll.py       # eBPF(bcc) 트레이서
└── container/
    ├── Dockerfile
    ├── epoll_server.cpp     # C++ epoll 서버
    └── run_epoll.sh         # 컨테이너 안에서 실행할 bash 스크립트
```

### Python docker executor / Orchestrator

```python
    #!/usr/bin/env python3
    import subprocess
    import pathlib
    import sys

    ROOT = pathlib.Path(__file__).resolve().parent
    CONTAINER_DIR = ROOT / "container"
    IMAGE_NAME = "epoll_ebpf_demo:latest"

    def run_cmd(cmd, **kwargs):
        """간단한 helper: 커맨드 실행 + stdout/stderr 출력"""
        print(f"\n[RUN] {' '.join(cmd)}")
        result = subprocess.run(cmd, text=True, capture_output=True, **kwargs)
        print("---- STDOUT ----")
        print(result.stdout.strip())
        print("---- STDERR ----")
        print(result.stderr.strip())
        print(f"[EXIT CODE] {result.returncode}")
        return result

    def build_image():
        cmd = [
            "docker", "build",
            "-t", IMAGE_NAME,
            str(CONTAINER_DIR)
        ]
        return run_cmd(cmd)

    def run_container()
        scripts_dir = CONTAINER_DIR # run_epoll.sh location

        cmd = [
            "docker", "run", "--rm",
            "-v", f"{scripts_dir}:/scripts",
            IMAGE_NAME,
            "bash", "/scripts/run_epoll.sh"
        ]
        return run_cmd(cmd)

    def main():
        if len(sys.argv) < 2:
            print("Usage : python orchestrator.py [build|run|all]")
            sys.exit(1)

        mode = sys.argv[1]
        if mode in ("build", "all"):
            r = build_image()
            if r.returncode != 0:
                print("Docker build failed")
                sys.exit(r.returncode)

        if mode in ("run", "all"):
            r = run_container()
            if r.returncode != 0:
                print("Docker run failed")
                sys.exit(r.returncode)

    if __name__ == "__main__":
        main()
```

- ``build_image()`` runs ``docker build`` on ``./container``
- ``run_container()`` mounts ``./container`` into ``./scripts`` and executes ``./run_epoll.sh`` inside the container
- ``run_cmd()`` prints stdout/stderr and exit code so you can inspect docker output easily.

### Dockerfile - C++ epoll environment

```dockerfile
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
```

### run_epoll.sh

```bash
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
```