#!/bin/bash

docker run\
 --rm \
 -v "$(realpath ..)":/home/devel/lsdnet\
 --runtime nvidia\
 --workdir /home/devel/lsdnet/line_extracter/\
 -t lsdnet\
 bash build_project.sh
