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
