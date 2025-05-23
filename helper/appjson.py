import gradio as gr
from controlnet_aux import OpenposeDetector
import os
import cv2
import numpy as np
from PIL import Image
from moviepy.editor import *
import json
import mediapipe as mp

# Carga del detector OpenPose para imagen anotada
openpose = OpenposeDetector.from_pretrained('lllyasviel/ControlNet')

# Configuración de MediaPipe Pose para extraer keypoints
mp_pose = mp.solutions.pose
pose = mp_pose.Pose(static_image_mode=True, min_detection_confidence=0.5)

# Mapeo de MediaPipe a OpenPose (según indices)
OPENPOSE_KEYPOINT_NAMES = [
    "Nose", "Neck", "RShoulder", "RElbow", "RWrist", "LShoulder", "LElbow", "LWrist",
    "RHip", "RKnee", "RAnkle", "LHip", "LKnee", "LAnkle", "REye", "LEye", "REar", "LEar"
]

MEDIAPIPE_TO_OPENPOSE = {
    0: 0,   # Nose
    11: 2,  # RShoulder
    13: 3,  # RElbow
    15: 4,  # RWrist
    12: 5,  # LShoulder
    14: 6,  # LElbow
    16: 7,  # LWrist
    23: 8,  # RHip
    25: 9,  # RKnee
    27: 10, # RAnkle
    24: 11, # LHip
    26: 12, # LKnee
    28: 13, # LAnkle
    2: 14,  # REye
    5: 15,  # LEye
    3: 16,  # REar
    6: 17,  # LEar
    # Neck no está en MediaPipe; lo estimamos como punto medio entre hombros
}

# Extraer fotogramas del video
def get_frames(video_in):
    frames = []
    clip = VideoFileClip(video_in)
    target_fps = min(clip.fps, 30)
    clip_resized = clip.resize(height=512)
    clip_resized.write_videofile("video_resized.mp4", fps=target_fps)

    cap = cv2.VideoCapture("video_resized.mp4")
    fps = cap.get(cv2.CAP_PROP_FPS)
    i = 0
    while cap.isOpened():
        ret, frame = cap.read()
        if not ret:
            break
        frame_path = f'frame_{i:04d}.jpg'
        cv2.imwrite(frame_path, frame)
        frames.append(frame_path)
        i += 1
    cap.release()
    cv2.destroyAllWindows()
    return frames, fps

# Procesar un fotograma: imagen anotada y keypoints
def process_frame(frame_path):
    image = Image.open(frame_path).convert('RGB')
    processed_img = openpose(image)
    img_np = np.array(image)
    results = pose.process(cv2.cvtColor(img_np, cv2.COLOR_RGB2BGR))

    keypoints = {}
    width, height = image.size

    if results.pose_landmarks:
        landmarks = results.pose_landmarks.landmark
        for mp_idx, op_idx in MEDIAPIPE_TO_OPENPOSE.items():
            lm = landmarks[mp_idx]
            keypoints[OPENPOSE_KEYPOINT_NAMES[op_idx]] = {
                'x': lm.x,
                'y': lm.y,
                'z': lm.z,
                'visibility': lm.visibility
            }

        # Calcular "Neck" como promedio entre hombros
        if 11 in MEDIAPIPE_TO_OPENPOSE and 12 in MEDIAPIPE_TO_OPENPOSE:
            r_shoulder = landmarks[11]
            l_shoulder = landmarks[12]
            neck = {
                'x': (r_shoulder.x + l_shoulder.x) / 2,
                'y': (r_shoulder.y + l_shoulder.y) / 2,
                'z': (r_shoulder.z + l_shoulder.z) / 2,
                'visibility': (r_shoulder.visibility + l_shoulder.visibility) / 2
            }
            keypoints["Neck"] = neck

    out_img_path = f"openpose_{frame_path}"
    processed_img.save(out_img_path)

    json_ind_path = f"openpose_{os.path.splitext(frame_path)[0]}.json"
    with open(json_ind_path, 'w') as jf:
        json.dump(keypoints, jf, indent=2)

    return out_img_path, json_ind_path, keypoints

# Reconstruir video desde imágenes
def create_video(frames, fps, prefix):
    clip = ImageSequenceClip(frames, fps=fps)
    out_vid = f"{prefix}_result.mp4"
    clip.write_videofile(out_vid, fps=fps)
    return out_vid

# Convertir GIF a video
def convertG2V(imported_gif):
    clip = VideoFileClip(imported_gif.name)
    clip.write_videofile("my_gif_video.mp4")
    return "my_gif_video.mp4"

# Función principal de inferencia
def infer(video_in):
    frames_list, fps = get_frames(video_in)
    processed_imgs = []
    json_files = []
    all_keypoints = {}

    for fp in frames_list:
        img_path, json_ind, kp = process_frame(fp)
        processed_imgs.append(img_path)
        json_files.append(json_ind)
        frame_key = os.path.splitext(fp)[0]
        all_keypoints[frame_key] = kp

    combined_path = "all_keypoints.json"
    with open(combined_path, 'w') as cf:
        json.dump(all_keypoints, cf, indent=2)

    final_vid = create_video(processed_imgs, fps, "openpose")
    files = [final_vid] + processed_imgs + json_files + [combined_path]
    return final_vid, files

# Interfaz Gradio
title = """
<div style="text-align: center; max-width: 500px; margin: 0 auto;">
    <h1 style="font-weight: 600;">Video to OpenPose</h1>
</div>
"""
with gr.Blocks() as demo:
    gr.HTML(title)
    with gr.Row():
        with gr.Column():
            video_input = gr.Video(sources=["upload"])
            gif_input = gr.File(label="Import a GIF instead", file_types=['.gif'])
            gif_input.change(fn=convertG2V, inputs=gif_input, outputs=video_input)
            submit_btn = gr.Button("Submit")
        with gr.Column():
            video_output = gr.Video(label="Video con OpenPose")
            download_files = gr.Files(label="Descargar Videos, Imágenes y JSON")
    submit_btn.click(fn=infer, inputs=[video_input], outputs=[video_output, download_files])

demo.launch()
