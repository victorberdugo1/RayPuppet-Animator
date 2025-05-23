import gradio as gr
from controlnet_aux import OpenposeDetector
import os
import cv2
import numpy as np
from PIL import Image
from moviepy.editor import VideoFileClip, ImageSequenceClip
import json
import mediapipe as mp
from mediapipe.tasks import python
from mediapipe.tasks.python import vision

# Configuración de MediaPipe
mp_holistic = mp.solutions.holistic
mp_drawing = mp.solutions.drawing_utils

# Especificaciones de dibujo personalizadas
body_drawing_spec = mp_drawing.DrawingSpec(color=(255, 0, 0), thickness=2, circle_radius=2)
hand_drawing_spec = mp_drawing.DrawingSpec(color=(0, 255, 0), thickness=2, circle_radius=2)
face_drawing_spec = mp_drawing.DrawingSpec(color=(0, 0, 255), thickness=1, circle_radius=1)

# Inicializar OpenPose
openpose = OpenposeDetector.from_pretrained('lllyasviel/ControlNet')

def initialize_holistic():
    return mp_holistic.Holistic(
        static_image_mode=False,
        model_complexity=2,
        refine_face_landmarks=True,
        min_detection_confidence=0.7,
        min_tracking_confidence=0.7
    )

holistic = initialize_holistic()

def get_frames(video_in):
    frames = []
    clip = VideoFileClip(video_in)
    target_fps = min(clip.fps, 30)
    clip_resized = clip.resize(height=512)
    clip_resized.write_videofile("video_resized.mp4", fps=target_fps, verbose=False, logger=None)

    cap = cv2.VideoCapture("video_resized.mp4")
    fps = cap.get(cv2.CAP_PROP_FPS)
    idx = 0
    while True:
        ret, frame = cap.read()
        if not ret:
            break
        path = f'frame_{idx:04d}.jpg'
        cv2.imwrite(path, frame)
        frames.append(path)
        idx += 1
    cap.release()
    return frames, fps

def draw_custom_landmarks(image, results):
    # Copiar la imagen para no modificar la original
    annotated_image = image.copy()
    
    # Dibujar cuerpo
    if results.pose_landmarks:
        mp_drawing.draw_landmarks(
            image=annotated_image,
            landmark_list=results.pose_landmarks,
            connections=mp_holistic.POSE_CONNECTIONS,
            landmark_drawing_spec=body_drawing_spec,
            connection_drawing_spec=body_drawing_spec)
    
    # Dibujar manos
    if results.left_hand_landmarks:
        mp_drawing.draw_landmarks(
            image=annotated_image,
            landmark_list=results.left_hand_landmarks,
            connections=mp_holistic.HAND_CONNECTIONS,
            landmark_drawing_spec=hand_drawing_spec,
            connection_drawing_spec=hand_drawing_spec)
    
    if results.right_hand_landmarks:
        mp_drawing.draw_landmarks(
            image=annotated_image,
            landmark_list=results.right_hand_landmarks,
            connections=mp_holistic.HAND_CONNECTIONS,
            landmark_drawing_spec=hand_drawing_spec,
            connection_drawing_spec=hand_drawing_spec)
    
    # Dibujar cara
    if results.face_landmarks:
        mp_drawing.draw_landmarks(
            image=annotated_image,
            landmark_list=results.face_landmarks,
            connections=mp_holistic.FACEMESH_TESSELATION,
            landmark_drawing_spec=face_drawing_spec,
            connection_drawing_spec=face_drawing_spec)
    
    return annotated_image

def process_landmarks(results, image_width, image_height):
    landmarks = {
        "body": [],
        "left_hand": [],
        "right_hand": [],
        "face": []
    }
    
    # Procesar cuerpo (BODY_25)
    if results.pose_landmarks:
        for lm in results.pose_landmarks.landmark:
            landmarks["body"].append({
                "x": lm.x * image_width,
                "y": lm.y * image_height,
                "z": lm.z,
                "visibility": lm.visibility
            })
    
    # Procesar manos
    if results.left_hand_landmarks:
        for lm in results.left_hand_landmarks.landmark:
            landmarks["left_hand"].append({
                "x": lm.x * image_width,
                "y": lm.y * image_height,
                "z": lm.z
            })
    
    if results.right_hand_landmarks:
        for lm in results.right_hand_landmarks.landmark:
            landmarks["right_hand"].append({
                "x": lm.x * image_width,
                "y": lm.y * image_height,
                "z": lm.z
            })
    
    # Procesar cara
    if results.face_landmarks:
        for lm in results.face_landmarks.landmark:
            landmarks["face"].append({
                "x": lm.x * image_width,
                "y": lm.y * image_height,
                "z": lm.z
            })
    
    return landmarks

def process_frame(frame_path):
    original_image = Image.open(frame_path)
    width, height = original_image.size
    
    # Detección con MediaPipe Holistic
    image = cv2.imread(frame_path)
    results = holistic.process(cv2.cvtColor(image, cv2.COLOR_BGR2RGB))
    
    # Generar imagen base de OpenPose
    op_image = openpose(original_image, hand=True, face=True)
    op_cv_image = cv2.cvtColor(np.array(op_image), cv2.COLOR_RGB2BGR)
    
    # Dibujar landmarks adicionales
    final_image = draw_custom_landmarks(op_cv_image, results)
    
    # Convertir y redimensionar
    final_image_pil = Image.fromarray(cv2.cvtColor(final_image, cv2.COLOR_BGR2RGB))
    final_image_pil = final_image_pil.resize((width, height))
    
    # Guardar imagen
    out_img = f"openpose_{frame_path}"
    final_image_pil.save(out_img)
    
    # Procesar y guardar keypoints
    frame_data = {"people": []}
    if results.pose_landmarks:
        landmarks = process_landmarks(results, width, height)
        
        person = {
            "pose_keypoints_2d": [],
            "face_keypoints_2d": [],
            "hand_left_keypoints_2d": [],
            "hand_right_keypoints_2d": []
        }
        
        # Cuerpo (25 puntos)
        for lm in landmarks["body"][:25]:
            person["pose_keypoints_2d"].extend([lm["x"], lm["y"], lm["visibility"]])
        
        # Manos (21 puntos cada una)
        if landmarks["left_hand"]:
            for lm in landmarks["left_hand"][:21]:
                person["hand_left_keypoints_2d"].extend([lm["x"], lm["y"], 1.0])
        
        if landmarks["right_hand"]:
            for lm in landmarks["right_hand"][:21]:
                person["hand_right_keypoints_2d"].extend([lm["x"], lm["y"], 1.0])
        
        # Cara (70 puntos)
        if landmarks["face"]:
            for lm in landmarks["face"][:70]:
                person["face_keypoints_2d"].extend([lm["x"], lm["y"], 1.0])
        
        frame_data["people"].append(person)
    
    json_path = f"openpose_{os.path.splitext(frame_path)[0]}.json"
    with open(json_path, 'w') as f:
        json.dump(frame_data, f, indent=2)
    
    return out_img, json_path

def create_video(imgs, fps, prefix):
    sizes = [Image.open(img).size for img in imgs]
    if len(set(sizes)) != 1:
        raise ValueError(f"Tamaños inconsistentes: {set(sizes)}")
    
    clip = ImageSequenceClip(imgs, fps=fps)
    out = f"{prefix}_result.mp4"
    clip.write_videofile(out, fps=fps, verbose=False, logger=None)
    return out

def infer(video_in):
    # Limpiar archivos anteriores
    [os.remove(f) for f in os.listdir() if f.startswith(('frame_', 'openpose_')) and os.path.isfile(f)]
    
    # Procesar video
    frames, fps = get_frames(video_in)
    imgs = []
    json_files = []
    
    for fp in frames:
        img, json_path = process_frame(fp)
        imgs.append(img)
        json_files.append(json_path)
    
    # Crear video
    vid = create_video(imgs, fps, "openpose")
    [os.remove(f) for f in frames]
    
    return vid, imgs + json_files

title = """
<div style="text-align:center; max-width:500px; margin:0 auto;">
  <h1>Video to OpenPose Full Body</h1>
  <p>BODY_25 + Manos + Cara</p>
</div>
"""

with gr.Blocks() as demo:
    gr.HTML(title)
    with gr.Row():
        with gr.Column():
            video_in = gr.Video(sources=["upload"])
            btn = gr.Button("Procesar")
        with gr.Column():
            video_out = gr.Video(label="Resultado")
            files = gr.Files(label="Archivos Generados")
    btn.click(fn=infer, inputs=[video_in], outputs=[video_out, files])

demo.launch()