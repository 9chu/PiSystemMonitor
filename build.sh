#!/bin/bash

set -e

cd $(dirname $0)
mkdir -p build
cd build
cmake ../software -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build . --parallel

# install to /usr/local/bin if you want to run the program from anywhere
cp PiSystemMonitor ../PiSystemMonitor
cp ../startup.sh.default ../startup.sh
chmod +x ../startup.sh
