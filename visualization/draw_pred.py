import numpy as np
import cv2


def draw_lines_on_image(image, lines, thresh, thickness=2, color=(255, 0, 0)):
    try:
        for line in lines:
            x1, y1, x2, y2, conf = line
            if conf < thresh:
                continue
            cv2.line(
                image,
                (int(x1), int(y1)),
                (int(x2), int(y2)),
                color=color,
                thickness=thickness,
            )
    except:
        print(lines.shape)
        raise
    return image


def draw_prediction(
    image: "numpy.ndarray", lines: "numpy.ndarray", thresh
) -> "numpy.ndarray":
    if np.squeeze(lines).shape == (0,):
        return image
    if len(lines.shape) == 1:
        lines = np.expand_dims(lines, 0)
    lines_our = lines.astype("float64")
    line_image = draw_lines_on_image(np.copy(image), lines_our, thresh)
    return line_image
