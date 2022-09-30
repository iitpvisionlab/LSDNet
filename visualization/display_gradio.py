import gradio as gr
from context import lsd
from draw_pred import draw_prediction
import cv2
import os
import numpy as np
import shutil

image_name = "demo"
out_path = "./predictions/"
model_path = "../lsd/models_onnx/model.onnx"


def display_prediction(image, score_thr):
    # firstly we save image to disk, to match "detect_and_save"s interface
    os.makedirs("intermediate_images", exist_ok=True)
    os.makedirs("predictions", exist_ok=True)
    path_to_image = os.path.join("./intermediate_images", image_name + ".jpg")
    cv2.imwrite(path_to_image, image)

    lsd.detect_and_save(path_to_image, out_path, model_path)
    segments = np.array(
        np.genfromtxt("./predictions/" + image_name + ".jpg.txt", delimiter=" "),
        dtype="float",
    )
    if segments.shape == (0,):
        return image
    line_image = draw_prediction(image, segments, score_thr)

    shutil.rmtree("intermediate_images")
    shutil.rmtree("predictions")
    return line_image


sample_images = [
    ["test_images/00031810.jpg", 0.7],
    ["test_images/00037254.jpg", 0.7],
    ["test_images/00055631.jpg", 0.7],
]


iface = gr.Interface(
    display_prediction,
    inputs=[
        "image",
        gr.inputs.Number(default=0.7, label="score threshold (0.0 ~ 1.0)"),
    ],
    outputs="image",
    server_name="0.0.0.0",
    server_port=7860,
    title="LSDNet",
    description='Official implementation of "LSDNet: Trainable Modification of LSD Algorithm for Real-Time Line Segment Detection." by Teplyakov, Lev, Leonid Erlygin, and Evgeny Shvets. IEEE Access 10 (2022): 45256-45265.',
    allow_flagging=False,
    examples=sample_images,
    allow_screenshot=False,
)
iface.launch()
