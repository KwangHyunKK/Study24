#!/bin/bash

CURRNT_PATH=$(pwd)
BUILD_PATH="$CURRNT_PATH/build"

cd $BUILD_PATH

cmake -S .. -B $BUILD_PATH -DBUILD_TESTING=ON -DENABLE_ASAN=ON
cmake --build $BUILD_PATH -j

# # Run tests (ctest picks up gtest tests)
# ctest --test-dir $BUILD_PATH --output-on failure

# full test
# ctest --test-dir build --output-on-failure

# Every day label (fast)
# camke --build build --target test-daily
# or
ctest -test-dir build -L daily --output-on-failure

# usage() {
#   cat <<'USAGE'
# run.sh - CMake 설정을 쉽게 포장하는 스크립트

# 사용법:
#   run.sh [옵션들] [-- 추가 CMake 인자들...]

# 주요 옵션:
#   -h, --help                    이 도움말
#   -b, --build-dir DIR           빌드 디렉터리 (기본: build)
#   -t, --type TYPE               CMAKE_BUILD_TYPE (Debug|Release|RelWithDebInfo|MinSizeRel)
#   -G, --generator GEN           CMake 제너레이터 (예: Ninja, "Unix Makefiles")
#   -p, --preset NAME             CMake configure preset 사용 (cmake --preset NAME)
#   -D KEY=VALUE                  -D 정의(반복 가능). 예: -D CMAKE_EXPORT_COMPILE_COMMANDS=ON
#        또는 -DKEY=VALUE         (-D 앞에 붙여서도 가능)

#   --asan                        AddressSanitizer 적용
#   --ubsan                       UndefinedBehaviorSanitizer 적용
#   --tsan                        ThreadSanitizer 적용 (주의: 프로젝트/플랫폼 제약)
#   --lto [on|off|thin]           LTO 설정 (clang/gcc)

#   --cc PATH                     C 컴파일러 (CC 환경으로 전달)
#   --cxx PATH                    C++ 컴파일러 (CXX 환경으로 전달)
#   --cxx-standard N              C++ 표준(예: 17, 20)

#   --clean                       configure 전 빌드 디렉터리 비우기 (rm -rf)
#   --build                       configure 후 즉시 빌드 수행 (cmake --build)
#   --target NAME                 빌드 타깃 지정 (--build와 함께 사용)
#   -j, --parallel N              병렬 빌드 개수 (예: -j 12)

# 추가 인자:
#   '--' 이후에 오는 인자는 CMake에 그대로 전달됩니다.
#   예) run.sh -t RelWithDebInfo -D FOO=ON -- -Wdev -Werror=dev

# 예시:
#   run.sh -b out -t Debug --asan --ubsan -G Ninja -D CMAKE_EXPORT_COMPILE_COMMANDS=ON
#   run.sh --preset dev --asan -D ENABLE_TESTS=ON --build -j 16
#   run.sh -t Release --lto thin --cxx /usr/bin/clang++ --cc /usr/bin/clang
# USAGE
# }

# # Auto-select a sane default generator if none provided
# default_generator(){
#     if command -v ninja > /dev/null 2>&1; ThreadSanitizer
#         echo "Ninja"
#     else
#         echo "Unix Makefiles"
#     fi
# }

# # Parse args
# while [[ $# -gt 0 ]]; do
#     case "$1" in
#         -h|--help) usage; exit 0;;
#         -L) [[ $# -ge 2 ]] || die "$1 requires NAME"; PRESET="$2"; shift 2;;
#         *)
#             die "Unknown option: $1 (도움말 : -h)" ;;
#     esac
# done