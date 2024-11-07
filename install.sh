#!/bin/bash

# Dependencies: git cmake gcc g++ libxinerama-dev libxrandr-dev libxcursor-dev libxi-dev libxext-dev

command -v git >/dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "Git is not installed. Please install git and try again."
    exit 1
fi

command -v cmake >/dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "CMake is not installed. Please install cmake and try again."
    exit 1
fi

command -v python3 >/dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "Python3 is not installed. Please install python3 and try again."
    exit 1
fi

set -e

# create .venv
if [ ! -d .venv ]; then
    python3 -m venv .venv
fi
source .venv/bin/activate
pip install -r requirements.txt

# build DearPyGui
mkdir -p build
cd build
git clone https://github.com/hoffstadt/DearPyGui --depth=1 --branch=v2.0.0
cd DearPyGui
git submodule update --init --recursive
export CMAKE_BUILD_PARALLEL_LEVEL=1
python setup.py install
