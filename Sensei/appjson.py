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

# Función para extraer fotogramas de video
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
    if results.pose_landmarks:
        for idx, lm in enumerate(results.pose_landmarks.landmark):
            keypoints[f'landmark_{idx:02d}'] = {
                'x': lm.x,
                'y': lm.y,
                'z': lm.z,
                'visibility': lm.visibility
            }
    out_img_path = f"openpose_{frame_path}"
    processed_img.save(out_img_path)
    # Guardar JSON individual
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
        # Usar el nombre sin extensión como clave
        frame_key = os.path.splitext(fp)[0]
        all_keypoints[frame_key] = kp

    # Guardar JSON combinado
    combined_path = "all_keypoints.json"
    with open(combined_path, 'w') as cf:
        json.dump(all_keypoints, cf, indent=2)
    combined_path = "all_keypoints.json"
    with open(combined_path, 'w') as cf:
        json.dump(all_keypoints, cf, indent=2)

    final_vid = create_video(processed_imgs, fps, "openpose")
    # Archivos para descarga: video, frames, JSON individuales y el combinado
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
