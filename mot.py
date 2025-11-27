#source yolovenv/Scripts/activate
from collections import defaultdict

import cv2
import numpy as np

from ultralytics import YOLO

model = YOLO("yolo11n.pt")
video_path = "sample2.mp4"
cap = cv2.VideoCapture(video_path)
track_history = defaultdict(lambda: [])

width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))

# ウィンドウ作成
cv2.namedWindow("YOLO11 Tracking", cv2.WINDOW_NORMAL)
cv2.resizeWindow("YOLO11 Tracking", width, height)

while cap.isOpened():
    success, frame = cap.read()
    if success:
        results = model.track(frame, persist=True)
        boxes = results[0].boxes.xywh.cpu()
        track_ids = results[0].boxes.id.int().cpu().tolist()
        annotated_frame = results[0].plot()
        for box, track_id in zip(boxes, track_ids):
            x, y, w, h = box
            track = track_history[track_id]
            track.append((float(x), float(y)))
            if len(track) > 30:
                track.pop(0)
            points = np.hstack(track).astype(np.int32).reshape((-1, 1, 2))
            cv2.polylines(annotated_frame, [points], isClosed=False, color=(230, 230, 230), thickness=10)
        cv2.imshow("YOLO11 Tracking", annotated_frame)
        if cv2.waitKey(1) & 0xFF == ord("q"):
            break
    else:
        break
cap.release()
cv2.destroyAllWindows()