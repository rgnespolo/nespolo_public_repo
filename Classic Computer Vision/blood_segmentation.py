# -*- coding: utf-8 -*-
"""
Created on  Dez 5 2021
Read video (or video streaming from capture card) and segment blood during hemorrage. Default HSV values set 
according to surgeons' preference from previous usability survey
@author: Rogerio Nespolo
"""
from __future__ import print_function
import cv2
import PySimpleGUI as GUI
import numpy as np
class Blood_segmentation:
    def __init__(self, video_src):
        #set variables that handle with video pre-processing and user interface (including trackbars to set HSV color space values)
        self.cap = cv2.VideoCapture(video_src)
        self.resize_width = int(1920)
        self.resize_height = int(1080)
        #window properties
        self.max_value = 255
        self.max_value_H = 360 // 2
        self.low_H = 0
        self.low_S = 0
        self.low_V = 0
        self.high_H = self.max_value_H
        self.high_S = self.max_value
        self.high_V = self.max_value
        self.window_capture_name = 'Video Capture'
        self.window_detection_name = 'Object Detection'
        self.low_H_name = 'Low H'
        self.low_S_name = 'Low S'
        self.low_V_name = 'Low V'
        self.high_H_name = 'High H'
        self.high_S_name = 'High S'
        self.high_V_name = 'High V'

        cv2.namedWindow(self.window_capture_name)
        cv2.namedWindow(self.window_detection_name)
        cv2.createTrackbar(self.low_H_name, self.window_detection_name, self.low_H, self.max_value_H, self.on_low_H_thresh_trackbar)
        cv2.createTrackbar(self.high_H_name, self.window_detection_name, self.high_H, self.max_value_H, self.on_high_H_thresh_trackbar)
        cv2.createTrackbar(self.low_S_name, self.window_detection_name, self.low_S, self.max_value, self.on_low_S_thresh_trackbar)
        cv2.createTrackbar(self.high_S_name, self.window_detection_name, self.high_S, self.max_value, self.on_high_S_thresh_trackbar)
        cv2.createTrackbar(self.low_V_name, self.window_detection_name, self.low_V, self.max_value, self.on_low_V_thresh_trackbar)
        cv2.createTrackbar(self.high_V_name, self.window_detection_name, self.high_V, self.max_value, self.on_high_V_thresh_trackbar)

    def on_low_H_thresh_trackbar(self, val):
        self.low_H = min(self.high_H - 1, val)
        cv2.setTrackbarPos(self.low_H_name, self.window_detection_name, self.low_H)

    def on_high_H_thresh_trackbar(self,val):
        self.high_H = max(val, self.low_H + 1)
        cv2.setTrackbarPos(self.high_H_name, self.window_detection_name, self.high_H)

    def on_low_S_thresh_trackbar(self,val):
        self.low_S = min(self.high_S - 1, val)
        cv2.setTrackbarPos(self.low_S_name, self.window_detection_name, self.low_S)

    def on_high_S_thresh_trackbar(self,val):
        self.high_S = max(val, self.low_S + 1)
        cv2.setTrackbarPos(self.high_S_name, self.window_detection_name, self.high_S)

    def on_low_V_thresh_trackbar(self,val):
        self.low_V = min(self.high_V - 1, val)
        cv2.setTrackbarPos(self.low_V_name, self.window_detection_name, self.low_V)

    def on_high_V_thresh_trackbar(self,val):
        self.high_V = max(val, self.low_V + 1)
        cv2.setTrackbarPos(self.high_V_name, self.window_detection_name, self.high_V)

    def run(self):
        # Main function - read the video and segment blood from hemorrage during vitrectomy
        # check video input
        if not self.cap.isOpened():
            print('Error loading the video stream')
            return

        # video read loop
        while self.cap.isOpened():
            ret, frame = self.cap.read()
            frame = frame[:, :int(frame.shape[1] / 2)]
            frame = cv2.resize(
                frame, (self.resize_width, self.resize_height), interpolation=cv2.INTER_AREA)
            frame_HSV = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

            frame_threshold = cv2.inRange(frame_HSV, (self.low_H, self.low_S, self.low_V), (self.high_H, self.high_S, self.high_V))
            #cv2.imshow(self.window_detection_name, frame_threshold)

            # HSV values optimized for intraoperative hemorrage in vitreoretinal procedures
            mask1 = cv2.inRange(frame_HSV, (0, self.low_S, self.low_V), (30, self.high_S, self.high_V))
            mask2 = cv2.inRange(frame_HSV, (150, self.low_S, self.low_V), (180, self.high_S, self.high_V))
            mask = cv2.bitwise_or(mask1, mask2)
            cv2.imshow(self.window_detection_name, mask)

            frame_threshold = cv2.cvtColor(frame_threshold, cv2.COLOR_GRAY2BGR)
            frame = np.concatenate((frame, frame_threshold), axis=1)
            cv2.imshow(self.window_capture_name, frame)


            key = cv2.waitKeyEx(1) & 0xFF
            if key == ord("q"):  # quit
                break
            if key == ord('p'):  # pause
                cv2.waitKey(-1)  # wait until any key is pressed
            if key == ord('m'):  # fast-foward
                actual_frame = self.cap.get(cv2.CAP_PROP_POS_FRAMES)
                self.cap.set(cv2.CAP_PROP_POS_FRAMES, actual_frame + 300)
                cv2.waitKey(-1)  # wait until any key is pressed
            if key == ord('n'):  # back foward
                actual_frame = self.cap.get(cv2.CAP_PROP_POS_FRAMES)
                self.cap.set(cv2.CAP_PROP_POS_FRAMES, actual_frame - 300)
                cv2.waitKey(-1)  # wait until any key is pressed


def main():
    GUI.ChangeLookAndFeel('Black')
    #Call pop-up to select video file - 1 for capture card (or capture card device id - 0, 1, 2...)
    video_src = GUI.PopupGetFile('Please enter a file name')

    while not (video_src is None):
        Blood_segmentation(video_src).run()
        video_src = GUI.PopupGetFile('Please enter a file name')
    print('Done')


if __name__ == '__main__':
    main()
    cv2.destroyAllWindows()
