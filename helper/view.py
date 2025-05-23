import json
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from matplotlib.widgets import Button

# Cargar datos desde un archivo con nombre simple
with open("3d_combined_data.json") as f:
    data = json.load(f)

# Extraer frames
frames = [data[k] for k in sorted(data) if k.startswith("frame_")]

# Calcular límites globales
all_x, all_y, all_z = [], [], []
for frame in frames:
    for person in frame.values():
        for p in person.values():
            all_x.append(p["x"])
            all_y.append(p["y"])
            all_z.append(p["z"])
xlim = (min(all_x), max(all_x))
ylim = (min(all_y), max(all_y))
zlim = (min(all_z), max(all_z))

# Conexiones esqueléticas
connections = [
    ("Neck", "RShoulder"), ("Neck", "LShoulder"),
    ("RShoulder", "RElbow"), ("RElbow", "RWrist"),
    ("LShoulder", "LElbow"), ("LElbow", "LWrist"),
    ("Neck", "RHip"), ("Neck", "LHip"),
    ("RHip", "RKnee"), ("RKnee", "RAnkle"),
    ("LHip", "LKnee"), ("LKnee", "LAnkle"),
    ("Neck", "Nose"), ("Nose", "REye"), ("Nose", "LEye"),
    ("REye", "REar"), ("LEye", "LEar")
]

# Crear figura
fig = plt.figure(figsize=(10, 8))
ax = fig.add_subplot(111, projection="3d")
plt.subplots_adjust(bottom=0.2)
ax.set_xlabel("X")
ax.set_ylabel("Y")
ax.set_zlabel("Z")
ax.view_init(elev=90, azim=90)

def init():
    ax.clear()
    ax.set_box_aspect([1, 1, 1])
    ax.set_xlim(*xlim)
    ax.set_ylim(*ylim)
    ax.set_zlim(*zlim)
    ax.set_xlabel("X")
    ax.set_ylabel("Y")
    ax.set_zlabel("Z")
    return []

def update(frame_num):
    ax.clear()
    ax.set_box_aspect([0.5, 1, 1])
    ax.set_xlim(*xlim)
    ax.set_ylim(*ylim)
    ax.set_zlim(*zlim)
    ax.set_xlabel("X")
    ax.set_ylabel("Y")
    ax.set_zlabel("Z")

    frame = frames[frame_num]
    for person_id, keypoints in frame.items():
        p4 = keypoints["Nose"]
        ax.text(p4["x"], p4["y"] - 0.25, p4["z"], person_id, fontsize=10, color='blue', weight='bold', ha='center')

        for key, p in keypoints.items():
            ax.scatter(p["x"], p["y"], p["z"], s=30)
            ax.text(p["x"], p["y"], p["z"], f"{key}", size=7)

        for start, end in connections:
            if start in keypoints and end in keypoints:
                p1, p2 = keypoints[start], keypoints[end]
                ax.plot([p1["x"], p2["x"]], [p1["y"], p2["y"]], [p1["z"], p2["z"]], 'r-')

    ax.set_title(f"Frame: {frame_num+1}/{len(frames)}")
    return []

ani = FuncAnimation(fig, update, frames=len(frames), init_func=init,
                    interval=100, blit=False)

class PlayPause:
    def __init__(self, anim):
        self.anim = anim
        self.paused = False
    def __call__(self, event):
        if self.paused:
            self.anim.event_source.start()
            play_button.label.set_text("⏸ Pausar")
        else:
            self.anim.event_source.stop()
            play_button.label.set_text("▶ Play")
        self.paused = not self.paused

ax_play = plt.axes([0.3, 0.05, 0.1, 0.075])
ax_reset = plt.axes([0.5, 0.05, 0.1, 0.075])
play_button = Button(ax_play, "⏸ Pausar")
reset_button = Button(ax_reset, "⏮ Reiniciar")
play_pause = PlayPause(ani)
play_button.on_clicked(play_pause)

def reset(event):
    ani.event_source.stop()
    ani.frame_seq = ani.new_frame_seq()
    update(0)
    play_button.label.set_text("⏸ Pausar")
    play_pause.paused = False

reset_button.on_clicked(reset)

plt.show()
