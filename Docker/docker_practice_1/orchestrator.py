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

def run_container():
    scripts_dir = CONTAINER_DIR # run_epoll.sh location

    cmd = [
        "docker", "run", "--rm",
        "-v", f"{scripts_dir}:/scripts",
        IMAGE_NAME,
        "bash", "chmod +x /scripts/run_epoll.sh", "/scripts/run_epoll.sh"
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