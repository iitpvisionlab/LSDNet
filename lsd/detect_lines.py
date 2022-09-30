import argparse
import onnxruntime
import os
import cv2
import subprocess
import numpy as np
from PIL import Image
import shutil
from pathlib import Path

import os

os.environ["CUDA_VISIBLE_DEVICES"] = "1"


PROJECTROOT = Path("/home/devel/lsdnet/")


class SegmentDector:
    """
    detects segments in one image
    """

    def __init__(self, model_path: str) -> None:
        # create onnx runner to run model

        self.session = onnxruntime.InferenceSession(model_path, None)
        self.input_name = self.session.get_inputs()[0].name
        self.output_name = self.session.get_outputs()[0].name
        self.image_size = self.session.get_inputs()[0].shape[1]

        # create intermediate dir
        self.path_to_intermediate_predictions = Path("intermediate_predictions")
        self.path_to_intermediate_predictions.mkdir(exist_ok=True)

        self.post_process_path = (
            PROJECTROOT / "line_extracter/cmake-build-release/Line_extracter"
        )

    def __call__(self, image: "np.ndarray") -> "np.ndarray":
        """
        returns segments detected on image
        """
        img_original_height, img_original_width, _ = image.shape

        img = image.astype("float32") / 255
        img = np.expand_dims(
            cv2.resize(img, (self.image_size, self.image_size)), axis=0
        )
        result = self.session.run([self.output_name], {self.input_name: img})
        cls_map = result[0][0][:, :, 0]
        angle_map = result[0][0][:, :, 1]

        a1 = Image.fromarray(cls_map)
        a2 = Image.fromarray(angle_map)

        cls_map_path = self.path_to_intermediate_predictions / "0_cls_map.tiff"
        angle_map_path = self.path_to_intermediate_predictions / "0_angle_map.tiff"
        a1.save(cls_map_path)
        a2.save(angle_map_path)

        # here we invoke c++ subprogramm
        subprocess.run(
            [
                self.post_process_path,
                cls_map_path,
                angle_map_path,
                str(self.path_to_intermediate_predictions) + "/",
                str(img_original_height),
                str(img_original_width),
            ]
        )

        segments = np.array(
            np.genfromtxt(
                self.path_to_intermediate_predictions / "0.csv", delimiter=" "
            ),
            dtype="float",
        )
        if len(segments.shape) == 1:
            segments = np.expand_dims(segments, 0)
        return segments


def detect_and_save(input_path: str, out_path: str, model_path: str):
    """
    detects lines on image or images specified in input_path and saves predictions to out_path

    Keyword arguments:
    input_path -- path to image or to foulder with images
    out_path -- output foulder
    model_path --  onnx neural network name to use in detection
    """

    sd = SegmentDector(model_path)

    if os.path.isdir(input_path):
        directory_content_paths = [
            os.path.join(input_path, ent) for ent in os.listdir(input_path)
        ]
    else:
        directory_content_paths = [input_path]

    # detect lines and save
    for entity_path in directory_content_paths:
        entity_name_split = entity_path.split(".")
        if entity_name_split[-1] != "jpg" and entity_name_split[-1] != "png":
            continue

        image = np.flip(cv2.imread(str(entity_path)), axis=2)
        segments = sd(image)
        dst_path = Path(out_path) / (Path(entity_path).name + ".txt")
        dst_path.parent.mkdir(exist_ok=True,parents=True)
        np.savetxt(
            dst_path, segments, fmt="%.6f"
        )

    shutil.rmtree("intermediate_predictions", ignore_errors=True)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("input_path", help="path to image or to foulder with images")
    parser.add_argument("out_path", help="output foulder")
    parser.add_argument("model_path", help="Neural network name to use in detection")

    args = parser.parse_args()

    detect_and_save(args.input_path, args.out_path, args.model_path)
