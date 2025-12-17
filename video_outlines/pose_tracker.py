import sys
import os
import cv2
import mediapipe as mp
import numpy as np

# MediaPipeの初期化 (グローバル設定)
mp_pose = mp.solutions.pose
pose = mp_pose.Pose(min_detection_confidence=0.5,
                     min_tracking_confidence=0.5,
                     enable_segmentation=True)

# ----------------------------------------------------------------------
# --- ヘルパー関数 ---
# ----------------------------------------------------------------------

def get_fixed_points(points, target_count):

    N_actual = len(points)

    if N_actual == 0 or target_count == 0:
        return np.empty((0, 2), dtype=np.int32)

    ## # 安全性のため、target_countが2 * N_actualを超えないようにクリップ
    ## if target_count > 2 * N_actual:
    ##     target_count = 2 * N_actual

    target_count //= 3

    # サンプリング用インデックスの生成
    # 0からN_actual-1の範囲で、target_count個の浮動小数点インデックスを均一に生成
    float_indices = np.linspace(0, N_actual - 1, target_count)

    # 最も近い元の点に丸めてインデックスを確定 (均等な分布を実現)
    final_indices = np.round(float_indices).astype(int)

    # インデックスの範囲チェック (安全のため)
    final_indices = np.clip(final_indices, 0, N_actual - 1)

    return points[final_indices]

def scan_video_for_contour_stats(video_path, pose_model):
    """動画をスキャンし、輪郭点の最大数と最小数を返す (Pass 1)。"""
    cap = cv2.VideoCapture(video_path)
    if not cap.isOpened():
        raise FileNotFoundError(f"動画ファイル {video_path} を開けませんでした。")

    max_points = 0
    min_points = float('inf')

    print("--- 動画スキャン (Pass 1) を開始 ---", file=sys.stderr)

    while cap.isOpened():
        ret, frame = cap.read()
        if not ret:
            break

        image = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        results = pose_model.process(image)

        if results.segmentation_mask is not None:
            # セグメンテーションマスクの取得と二値化
            condition = np.stack((results.segmentation_mask,) * 3, axis=-1) > 0.1
            mask_image = np.where(condition, 255, 0).astype(np.uint8)[:, :, 0]

            # 輪郭の抽出
            contours, _ = cv2.findContours(mask_image, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

            if contours:
                largest_contour = max(contours, key=cv2.contourArea)
                current_points = len(largest_contour)

                max_points = max(max_points, current_points)
                min_points = min(min_points, current_points)

    cap.release()

    final_min = min_points if min_points != float('inf') else 0
    print(f"スキャン完了: 最大輪郭点数={max_points}, 最小輪郭点数={final_min}", file=sys.stderr)

    return max_points, final_min

# ----------------------------------------------------------------------
# --- メイン処理 ---
# ----------------------------------------------------------------------

if __name__ == "__main__":
    # 引数チェック
    if len(sys.argv) < 2:
        print("--- 使い方 ---", file=sys.stderr)
        print("動画ファイル名を引数として指定してください。", file=sys.stderr)
        sys.exit(1)

    input_source = sys.argv[1]
    video_path = input_source
    output_path = "output_" + os.path.basename(video_path)

    # ファイル存在チェック
    if not os.path.exists(video_path):
        print(f"エラー: 指定されたファイル '{video_path}' が見つかりません。", file=sys.stderr)
        sys.exit(1)

    # 1. 動画スキャン (Pass 1) を実行し、統計情報を取得
    max_points, min_points = scan_video_for_contour_stats(video_path, pose)

    if max_points == 0:
        print("エラー: 動画全体で人物輪郭を検出できませんでした。処理を終了します。", file=sys.stderr)
        sys.exit(1)

    # 2. 固定点数 (N_fixed) の決定ロジック
    # N_fixed = min(N_max, 2 * N_min)
    target_fixed_points = max_points
    print(f"決定された固定輪郭点数 (N_fixed): {target_fixed_points} 点", file=sys.stderr)

    # ------------------------------------------------------------------
    # 3. メイン処理 (Pass 2) を開始
    # ------------------------------------------------------------------

    cap = cv2.VideoCapture(video_path)

    # 動画書き出し設定 (メインループ前に行う)
    if not cap.isOpened():
        print(f"エラー: 動画ファイル {video_path} を開けませんでした。", file=sys.stderr)
        sys.exit(1)

    # 動画の情報を取得
    fps = cap.get(cv2.CAP_PROP_FPS)
    width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
    height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
    fourcc = cv2.VideoWriter_fourcc(*'mp4v')

    # VideoWriterを初期化
    out = cv2.VideoWriter(output_path, fourcc, fps, (width, height))
    print(f"処理結果を {output_path} に書き出します。", file=sys.stderr)


    # --- 姿勢推定のメインループ (Pass 2) ---
    frame_num = 0
    while cap.isOpened():
        ret, frame = cap.read()
        frame_num += 1

        if not ret:
            print(f"フレーム {frame_num} で読み込み失敗/終了。", file=sys.stderr)
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
                cv2.drawContours(frame, [largest_contour], -1, (0, 0, 255), 2)

                # 輪郭座標を (N, 2) の形状に変換する
                contour_points = largest_contour.reshape(-1, 2)

                # 固定点数のサンプリングを適用
                fixed_points_array = get_fixed_points(contour_points, target_fixed_points)

                # 1. フレーム番号を出力開始
                output_line = f"{frame_num}, "

                # 2. すべての固定座標をリストに追加 (x y 形式で結合)
                # coords = []
                # for x, y in fixed_points_array:
                #     coords.append(f"{int(x)} {int(y)+50}")

                rd_x = 1000
                rd_y = 0
                ld_x = 0
                ld_y = 0
                ru_x = 1000
                ru_y = 50
                lu_x = 0
                lu_y = 50

                coords = []
                for x,y in fixed_points_array:
                   coords.append(f"{int(x)} {int(-y)}")
                #coords.append(f"{int(rd_x)} {int(rd_y)}")
                #coords.append(f"{int(ld_x)} {int(ld_y)}")
                #coords.append(f"{int(ru_x)} {int(ru_y)}")
                #coords.append(f"{int(lu_x)} {int(lu_y)}")

                # 3. リストをスペース区切りで結合し、フレーム番号の後ろに出力
                output_line += " ".join(coords)
                for _ in range(10):
                    print(output_line)

                cnt = 0
                dodai = []
                for _ in range(target_fixed_points//4):
                    dodai.append(f"{int(rd_x)} {int(rd_y)}")
                    dodai.append(f"{int(ld_x)} {int(ld_y)}")
                    dodai.append(f"{int(ru_x)} {int(ru_y)}")
                    dodai.append(f"{int(lu_x)} {int(lu_y)}")
                for _ in range(target_fixed_points%4):
                    dodai.append(f"{int(rd_x)} {int(rd_y)}")

                dodai_line = f"{frame_num}, "
                dodai_line += " ".join(dodai)
                #for _ in range(10):
                    #print(dodai_line)



        # 処理後のフレームを出力ファイルに書き込む
        out.write(frame)

    # --- 処理後 ---
    cap.release()
    out.release() # VideoWriterを閉じる
    cv2.destroyAllWindows()
    print(f"動画処理が完了しました。ファイル: {output_path}", file=sys.stderr)
