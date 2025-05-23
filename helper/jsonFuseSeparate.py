import json
from copy import deepcopy

# Cargar datos originales
with open('data1.json') as f:  # Vista frontal (x,y)
    data_front = json.load(f)

with open('data2.json') as f:  # Vista lateral (x como z)
    data_side = json.load(f)

# Crear estructura combinada 3D
combined_3d = {}

for frame_key in data_front:
    if frame_key in data_side:
        frame_front = data_front[frame_key]
        frame_side = data_side[frame_key]
        
        combined_frame = {}
        
        # Iterar a través de todas las personas en el frame
        for person_key in frame_front:
            if person_key in frame_side:
                combined_person = {}
                
                # Combinar joints para cada persona
                front_joints = frame_front[person_key]
                side_joints = frame_side[person_key]
                
                for joint in front_joints:
                    if joint in side_joints:
                        combined_person[joint] = {
                            'x': front_joints[joint]['x'],
                            'y': front_joints[joint]['y'],
                            'z': side_joints[joint]['x']  # Usar X lateral como Z
                        }
                
                combined_frame[person_key] = combined_person
        
        combined_3d[frame_key] = combined_frame

# Guardar nuevo JSON 3D
with open('3d_combined_data.json', 'w') as f:
    json.dump(combined_3d, f, indent=2)

print("JSON 3D combinado guardado como: 3d_combined_data.json")