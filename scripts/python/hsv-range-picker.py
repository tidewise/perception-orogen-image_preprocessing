#! /usr/bin/env python3

import cv2 as cv
import numpy as np
import sys

class HSVPicker:
    def __init__(self, hsv_image):
        self.image = hsv_image
        self.min = np.array([np.inf, np.inf, np.inf])
        self.max = -np.array([np.inf, np.inf, np.inf])

    def mouse_callback(self, event, x, y, flags, param):
        if event == cv.EVENT_LBUTTONUP:
            self._add(self.image[y, x])

    def _add(self, pixel):
        self.min = np.minimum(pixel, self.min)
        self.max = np.maximum(pixel, self.max)

    def range(self):
        return (self.min, self.max)

def draw_picked(image, hsv_image, range_):
    if np.any(np.isinf(range_[0])) or np.any(np.isinf(range_[1])):
        return image

    mask = cv.inRange(hsv_image, range_[0], range_[1])
    selected = np.where(mask == 0xff)
    drawing = image.copy()
    drawing[selected] = [0, 0, 0xff]
    return drawing

def usage():
    print("./hsv-range-picker.py PATH_TO_IMAGE")
    print("")
    print("Tool for selecting pixels from an image with the mouse\n"
          "and get the HSV range that would filter all the selected\n"
          "pixels. The selected pixels are colored in RED.\n"
          "\n"
          "Press ESC to exit")

if __name__ == "__main__":
    if len(sys.argv) != 2 or sys.argv[1] == "-h" or sys.argv[1] == "--help":
        usage()
        exit(0)

    image_path = sys.argv[1]
    image = cv.imread(image_path)

    hsv_image = cv.cvtColor(image, cv.COLOR_BGR2HSV)
    picker = HSVPicker(hsv_image)

    cv.namedWindow("image")
    cv.setMouseCallback("image", picker.mouse_callback)
    while cv.waitKey(30) != 27: # ESC
        cv.imshow("image", draw_picked(image, hsv_image, picker.range()))

    cv.destroyAllWindows()
    print(picker.range())