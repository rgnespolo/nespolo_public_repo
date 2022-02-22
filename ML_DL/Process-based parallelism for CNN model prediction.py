
import tensorflow as tf
import tensorflow_hub as hub
import numpy as np
import pandas as pd
import cv2
import multiprocessing
import time
import sys

from keras.preprocessing.image import img_to_array
dataset_labels = np.array(['capsulorhexis', 'Hydrodissection', 'no_ultrasound','paracentesis','phacoemulsification'])
CATARACT_SAVED_MODEL = "saved_models/cataract"
cataract_model = tf.keras.models.load_model(CATARACT_SAVED_MODEL,custom_objects={'KerasLayer':hub.KerasLayer})
cataract_model.build([None, 224, 224, 3])
print(cataract_model.summary())

cap = cv2.VideoCapture(1)
width = cap.get(cv2.CAP_PROP_FRAME_WIDTH)
confidence_score = 0.90


def cnn_pred_daemon(frame):
    frame = frame.astype("float") / 255.0
    frame = img_to_array(frame)
    frame = np.expand_dims(frame, axis=0)
    single_prediction = cataract_model.predict(frame)
    #print(np.around(single_prediction, decimals=3))
    idx = single_prediction.argmax(axis=-1)
    if idx > confidence_score:
        print(dataset_labels[idx])

if __name__ == '__main__':
    d = multiprocessing.Process(name='daemon', target=cnn_pred_daemon)
    d.daemon = True
    d.start()
    time.sleep(1)

    while (cap.isOpened()):
        ret, frame = cap.read()
        frame = frame[:, :(int(width / 2))]
        frame = cv2.resize(frame, (224, 224))
        cv2.imshow('Frame', frame)
        cnn_pred_daemon(frame)

        key = cv2.waitKey(1) & 0xFF
        if key == ord('q'):
            break

        if key == ord('m'):
            actual_frame = cap.get(cv2.CAP_PROP_POS_FRAMES)
            cap.set(cv2.CAP_PROP_POS_FRAMES, actual_frame + 600)
            cv2.waitKey(-1)  # wait until any key is pressed
        if key == ord('n'):
            actual_frame = cap.get(cv2.CAP_PROP_POS_FRAMES)
            cap.set(cv2.CAP_PROP_POS_FRAMES, actual_frame - 300)
            cv2.waitKey(-1)  # wait until any key is pressed

    cap.release()
    cv2.destroyAllWindows()


