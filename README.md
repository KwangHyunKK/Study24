# Embedded_Test

### Docker Ubuntu 22:04 설치

```
$ docker pull ubuntu:20.04

$ docker run -it --name my-ubuntu ubuntu:20.04 # docker run 실행

$ docker run -it --name <container name> -v <Host path>:<Container path> <image name> /bin/bash

# 예시
$ docker run -it --name dev -v C:\document\devlopment:/dev ubuntu:22.04 /bin/bash

$ docker ps # 실행 중인 컨테이너 확인

$ docker restart
```


### Ubuntu setting

```
# Update system
sudo apt-get update && sudo apt-get upgrade -y

# Install build tools
sudo apt-get install -y build-essential cmake make gcc g++ 

# Install debugging tools
sudo apt-get install -y gdb valgrind cppcheck clang-tidy clang-format

# Install network tools
sudo apt-get install -y net-tools curl wget

# Install Git
sudo apt-get install -y git

# Install C++ libraries
sudo apt-get install -y libboost-all-dev

# Install .NET SDK and Mono
sudo apt-get install -y dotnet-sdk-7.0 mono-complete

# Install Python and pip
sudo apt-get install -y python3 python3-pip python3-venv
pip3 install numpy pandas matplotlib

# Install Node.js and npm
sudo apt-get install -y nodejs npm

# Install Java
sudo apt-get install -y openjdk-17-jdk

# Install Rust
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
```
