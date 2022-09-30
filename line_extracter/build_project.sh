#!/bin/bash

cmake -Bcmake-build-release -H. -DCMAKE_BUILD_TYPE=Release
cd cmake-build-release
make