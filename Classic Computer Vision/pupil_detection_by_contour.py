# -*- coding: utf-8 -*-
"""
Classic computer vision algorithm for pupil detection in real time, using canny edge detection
@author: Rogerio Nespolo
"""
import numpy as np
import cv2

camera = cv2.VideoCapture(1) #Video streaming from capture device
kernel = np.ones((2, 2), np.uint8)

while True:
    # Loading Camera
    ret, frame = camera.read()

    blurred = cv2.pyrMeanShiftFiltering(frame, 3, 3)
    hsv = cv2.cvtColor(blurred, cv2.COLOR_BGR2HSV)

    lower_range = np.array([150, 10, 10])
    upper_range = np.array([180, 255, 255])
    mask = cv2.inRange(hsv, lower_range, upper_range)

    dilation = cv2.dilate(mask, kernel, iterations=1)

    closing = cv2.morphologyEx(dilation, cv2.MORPH_GRADIENT, kernel)
    closing = cv2.morphologyEx(dilation, cv2.MORPH_CLOSE, kernel)

    # Getting the edge of morphology
    edge = cv2.Canny(closing, 175, 175)
    contours, hierarchy = cv2.findContours(edge, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)

    # Find the index of the largest contour
    areas = [cv2.contourArea(c) for c in contours]
    max_index = np.argmax(areas)
    cnt = contours[max_index]

    x, y, w, h = cv2.boundingRect(cnt)
    cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)

    cv2.imshow('threshold', frame)
    cv2.imshow('edge', edge)

    key = cv2.waitKey(1) & 0xFF
    if key == ord("q"):
        # if 'q' key-pressed break out
        break
    if key == ord('p'):
        cv2.waitKey(-1)  # wait until any key is pressed

    #For pre-recorded videos, gives the option to forward or backward the video
    if key == ord('m'):
        actual_frame = camera.get(cv2.CAP_PROP_POS_FRAMES)
        camera.set(cv2.CAP_PROP_POS_FRAMES, actual_frame + 300)
        cv2.waitKey(-1)  # wait until any key is pressed
    if key == ord('n'):
        actual_frame = camera.get(cv2.CAP_PROP_POS_FRAMES)
        camera.set(cv2.CAP_PROP_POS_FRAMES, actual_frame - 300)
        cv2.waitKey(-1)  # wait until any key is pressed

camera.release()
cv2.destroyAllWindows()