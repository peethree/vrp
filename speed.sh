#!/bin/bash

configure() {
    cd build || exit 1
    /home/pee3/vcpkg/downloads/tools/cmake-3.30.1-linux/cmake-3.30.1-linux-x86_64/bin/cmake ..
}

build() {
    cd build || exit 1
    /home/pee3/vcpkg/downloads/tools/cmake-3.30.1-linux/cmake-3.30.1-linux-x86_64/bin/cmake --build .
}

run() {
    cd build || exit 1
    ./main
}

case "$1" in 
  configure|c)
    configure
    exit 0
    ;;
  build|b)
    build
    exit 0
    ;;
  run|r)
    run
    exit 0
    ;;
esac

echo "invalid arg: $1"