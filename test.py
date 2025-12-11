from collections import defaultdict
import cv2
import numpy as np
from ultralytics import YOLO

# モデルとビデオの読み込み
try:
    model = YOLO("yolo11n.pt")
    video_path = "sample1.mp4"
    cap = cv2.VideoCapture(video_path)
    
    if not cap.isOpened():
        raise ValueError(f"ビデオファイルが開けません: {video_path}")
except Exception as e:
    print(f"エラー: {e}")
    exit(1)

track_history = defaultdict(lambda: [])
frame_count = 0
max_track_length = 30  # 軌跡の最大長

width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))

cv2.namedWindow("YOLO11 Tracking", cv2.WINDOW_NORMAL)
cv2.resizeWindow("YOLO11 Tracking", width, height)

while cap.isOpened():
    success, frame = cap.read()
    if success:
        frame_count += 1
        results = model.track(frame, persist=True)
        
        if results[0].boxes.id is not None:
            boxes = results[0].boxes.xywh.cpu()
            track_ids = results[0].boxes.id.int().cpu().tolist()
            annotated_frame = results[0].plot()
            
            for box, track_id in zip(boxes, track_ids):
                x, y, w, h = box
                track = track_history[track_id]
                track.append((float(x), float(y)))
                # 軌跡の長さを制限
                if len(track) > max_track_length:
                    track.pop(0)
                
                if len(track) > 1:
                    points = np.hstack(track).astype(np.int32).reshape((-1, 1, 2))
                    cv2.polylines(annotated_frame, [points], isClosed=False, color=(230, 230, 230), thickness=10)
        else:
            annotated_frame = frame
        
        cv2.imshow("YOLO11 Tracking", annotated_frame)
        if cv2.waitKey(1) & 0xFF == ord("q"):
            break
    else:
        break

cap.release()
cv2.destroyAllWindows()

# 統計情報を出力
num_tracked_objects = len(track_history.keys())
print(f"フレーム数: {frame_count}")
print(f"追跡物体数: {num_tracked_objects}")
print("\n追跡データ:")
for tid in sorted(track_history.keys()):
    print(f"Track ID {tid}:")
    for x, y in track_history[tid]:
        print(f"  {x:.2f}, {y:.2f}")