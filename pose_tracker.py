import sys
import os
import cv2
import mediapipe as mp 
import numpy as np
# (略)

# MediaPipeの初期化
mp_pose = mp.solutions.pose
pose = mp_pose.Pose(min_detection_confidence=0.5, 
                     min_tracking_confidence=0.5, 
                     enable_segmentation=True) 

# --- メイン処理 ---
if __name__ == "__main__":
    # (略：引数チェック、ファイル存在チェック)

    input_source = sys.argv[1]
    video_path = input_source
    output_path = "output_" + os.path.basename(video_path) # ★ 出力ファイル名を設定
    
    cap = cv2.VideoCapture(video_path)
    
    # 3. 動画書き出し設定 (メインループ前に行う)
    if not cap.isOpened():
        print(f"エラー: 動画ファイル {video_path} を開けませんでした。")
        sys.exit(1)

    # 動画の情報を取得
    fps = cap.get(cv2.CAP_PROP_FPS)
    width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
    height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
    fourcc = cv2.VideoWriter_fourcc(*'mp4v') # mp4v (MP4コーデック) を使用
    
    # VideoWriterを初期化
    out = cv2.VideoWriter(output_path, fourcc, fps, (width, height))
    print(f"処理結果を {output_path} に書き出します。", file=sys.stderr)
    
    
    # --- 姿勢推定のメインループ ---
    frame_num = 0
    while cap.isOpened():
        ret, frame = cap.read()
        frame_num += 1
        
        if not ret:
            print(f"フレーム {frame_num} で読み込み失敗/終了。")
            break
        
        image = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        results = pose.process(image)
        
        # セグメンテーションマスクと輪郭の処理
        if results.segmentation_mask is not None:
            # 1. セグメンテーションマスクの取得と二値化
            condition = np.stack((results.segmentation_mask,) * 3, axis=-1) > 0.1
            mask_image = np.where(condition, 255, 0).astype(np.uint8)[:, :, 0]
            
            # 2. 輪郭の抽出
            contours, _ = cv2.findContours(mask_image, 
                                            cv2.RETR_EXTERNAL, 
                                            cv2.CHAIN_APPROX_SIMPLE)

            # 3. 輪郭の描画と座標出力
            if contours:
                largest_contour = max(contours, key=cv2.contourArea)
                
                # 輪郭を元のフレームに描画 (色: 赤, 太さ: 2px)
                # ★ 可視化処理
                cv2.drawContours(frame, [largest_contour], -1, (0, 0, 255), 2)

                countour_points = largest_contour.reshape(-1,2)

                output_line = f"{frame_num}, "

                coords = []
                for x, y in countour_points: 
                    coords.append(f"{int(x)} {int(y)}")

                output_line += " ".join(coords)
                print(output_line)

        # ★ 処理後のフレームを出力ファイルに書き込む ★
        out.write(frame)

    # --- 処理後 ---
    cap.release()
    out.release() # VideoWriterを閉じる
    cv2.destroyAllWindows()
    print(f"動画処理が完了しました。ファイル: {output_path}")