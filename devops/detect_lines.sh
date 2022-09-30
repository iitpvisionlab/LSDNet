#!/bin/bash

# set these variables
input_path="../visualization/test_images/"
out_path="../predictions"

model_path="models_onnx/model.onnx"
docker run\
 --rm \
 -v "$(realpath ..)":/home/devel/lsdnet\
 --workdir /home/devel/lsdnet/lsd/\
 -t lsdnet\
 python detect_lines.py $input_path $out_path $model_path
