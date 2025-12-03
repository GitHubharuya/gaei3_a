#source yolovenv/Scripts/activate
from collections import defaultdict

import cv2
import numpy as np

from ultralytics import YOLO

model = YOLO("yolo11n.pt")
video_path = "sample3.mp4"
cap = cv2.VideoCapture(video_path)
track_history = defaultdict(lambda: [])
m = 0 #ステップ数をカウント
frame_count = 0 #フレーム数

width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH)) 
height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT)) 
# ウィンドウ作成
cv2.namedWindow("YOLO11 Tracking", cv2.WINDOW_NORMAL) 
cv2.resizeWindow("YOLO11 Tracking", width, height) 

while cap.isOpened(): 
    success, frame = cap.read() 
    if success: 
        frame_count += 1
        results = model.track(frame, persist=True, verbose=False) 
        boxes = results[0].boxes.xywh.cpu() 
        track_ids = results[0].boxes.id.int().cpu().tolist()  
        annotated_frame = results[0].plot() 
        for box, track_id in zip(boxes, track_ids): #各検出物体について
            if frame_count % 5 == 0: #5フレームに1回軌跡を更新
                m += 1 #ステップ数をカウント
                x, y, w, h = box #バウンディングボックスの中心座標と幅・高さ
                track = track_history[track_id] #物体IDごとの軌跡リスト
                track.append((float(x), float(y))) 
                #if len(track) > 1: 
                    # track.pop(0)
                points = np.hstack(track).astype(np.int32).reshape((-1, 1, 2)) 
            #cv2.polylines(annotated_frame, [points], isClosed=False, color=(230, 230, 230), thickness=10) #軌跡を描画
        cv2.imshow("YOLO11 Tracking", annotated_frame) #フレームを表示
        if cv2.waitKey(1) & 0xFF == ord("q"): # 'q'キーで終了
            break
    else:
        break
cap.release() 
cv2.destroyAllWindows() 

n=len(track_history.keys()) #検出された物体の数

print(m,n) 

for tid in sorted(track_history.keys()):
    for x, y in track_history[tid]:
        print(x, y)
    print("")  #各物体の軌跡を区切るための改行