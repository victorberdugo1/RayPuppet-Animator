import json

# Cargar el archivo que contiene ambas personas
with open('all_keypoints.json') as f:
    data = json.load(f)

combined_3d = {}

for frame_key in data:
    frame_data = data[frame_key]
    
    # Verificar que existen ambas personas en el frame
    if "person_0" in frame_data and "person_1" in frame_data:
        combined_person = {}
        
        # Vista lateral (person_0) provee la coordenada Z (su X)
        side_person = frame_data["person_0"]
        # Vista frontal (person_1) provee las coordenadas X e Y
        front_person = frame_data["person_1"]
        
        # Combinar joints comunes
        for joint_name in front_person:
            if joint_name in side_person:
                combined_person[joint_name] = {
                    "x": front_person[joint_name]["x"],  # X desde frontal
                    "y": front_person[joint_name]["y"],  # Y desde frontal
                    "z": side_person[joint_name]["x"],   # Z desde lateral (su X)
                    #"visibility": front_person[joint_name]["visibility"]  # Usar visibilidad de la frontal
                }
        
        # Asignar la persona combinada como person_0 en el resultado
        combined_3d[frame_key] = {"person_0": combined_person}

# Guardar el resultado
with open('3d_combined_data.json', 'w') as f:
    json.dump(combined_3d, f, indent=2)

print("JSON 3D combinado guardado como: 3d_combined_data.json")