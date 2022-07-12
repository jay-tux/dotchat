#!/usr/bin/zsh

# usage:    ./build.sh          build both
#           ./build.sh s        build only server
#           ./build.sh c        build only client

function build_server() {
    cd server/cmake-build-debug/
    cmake --build .
    cd ../../
}

function build_client() {
    cd client/cmake-build-debug/
    cmake --build .
    cd ../../
}

if [ "$#" = 0 ]; then
    build_server
    build_client
elif [ "$1" = "s" ]; then
    build_server
elif [ "$1" = "c" ]; then
    build_client
else
    echo 'ERROR: either no arguments (build all), `s` (build server) or `c` (build client) required.' >&1
fi
