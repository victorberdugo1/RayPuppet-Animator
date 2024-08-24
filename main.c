//cc main.c -o main -L. -lraylib -lm -ldl -lpthread -lGL -lX11
//cc main.c -o main libraylib.a -lm 
//cc main.c bones.c -o main -L. -lraylib -lm -ldl -lpthread -lGL -lX11

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "bones.h"
#include "raylib.h"
#include "rlgl.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

Bone* currentBone = NULL;


void AdvanceBoneSelection(Bone* root) {
    if (currentBone == NULL) {
        currentBone = root;
        return;
    }

    // Si el hueso tiene hijos, selecciona el primero de ellos
    if (currentBone->childCount > 0) {
        currentBone = currentBone->child[0];
    }
    // Si no tiene hijos, retrocede hacia el padre y busca el siguiente hermano
    else {
        while (currentBone->parent != NULL) {
            Bone* parent = currentBone->parent;
            for (int i = 0; i < parent->childCount; i++) {
                if (parent->child[i] == currentBone && i + 1 < parent->childCount) {
                    currentBone = parent->child[i + 1];
                    return;
                }
            }
            currentBone = parent;
        }
    }
}
void UpdateBoneProperties(Bone* bone) {
    if (bone == NULL) return;

    if (IsKeyDown(KEY_UP)) {
        bone->l += 1.0f; // Incrementar longitud
    }
    if (IsKeyDown(KEY_DOWN)) {
        bone->l -= 1.0f; // Decrementar longitud
    }
    if (IsKeyDown(KEY_RIGHT)) {
        bone->a += 0.05f; // Incrementar ángulo
    }
    if (IsKeyDown(KEY_LEFT)) {
        bone->a -= 0.05f; // Decrementar ángulo
    }
}

int main(int argc, char *argv[])
{
	// Initialization of window and rendering settings
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Bone Animation");
	SetTargetFPS(60);
	rlEnableDepthTest();
	rlEnableColorBlend();

	t_mesh body;
	Bone *root = boneLoadStructure("Bbs_Skel.txt");
	root->x = GetScreenWidth() / 2.0f;
	root->y = GetScreenHeight() / 1.0f;

	// Load mesh data
	char names[MAX_BONECOUNT][99] = {0};
	boneListNames(root, names);
	const char *currentName = names[0];
	meshLoadData("Bbs_Mesh.txt", &body, root);
	LoadTextures();

	// Load animations
	animationLoadKeyframes("Bbs_SkelAnim.txt", root);

	//root = boneCleanAnimation(root, &body, "Bbs_SkelAnim.txt");
	//root = boneChangeAnimation(root, "Bbs_SkelAnim1.txt");	

	int frameNum = 1;
	bool animating = false;
	float intindex = 0.0f;
	float alocintp = 0.0f;

	if (argc > 1)
    {
        maxTime = atoi(argv[1]);
    }

	currentBone = root;  // Empezamos con el root

	// Main game loop
	while (!WindowShouldClose())
	{
		if (IsKeyPressed(KEY_L))
        {
			AdvanceBoneSelection(root);
		}

    UpdateBoneProperties(currentBone);

		if (IsKeyPressed(KEY_P))   // Avanzar un keyframe cuando se presiona 'P'
		{
			frameNum++;
			if (frameNum >= maxTime)  // Evitar que se pase del último keyframe
			{
				frameNum = 0;
			}
			boneAnimate(root, frameNum); // Actualizar la animación al siguiente keyframe
		}
		else if (IsKeyPressed(KEY_O)) // Retroceder un keyframe cuando se presiona 'O'
		{

		}

        if (animating)
        {
            // Puedes usar un temporizador para avanzar el frame
			static float frameTimer = 0.0f;
			frameTimer += GetFrameTime();

			// Cambiar el frame cada 0.1 segundos (ajusta este valor según sea necesario)
			if (frameTimer >= 0.02f)
			{
				frameTimer = 0.0f;
				if (frameNum >= maxTime) //eNum >= maxTime) Reinicia el frameNum cuando alcanza el máximo
				{
					frameNum = 0;
					if (argc > 1)
					{
						frameNum = maxTime;
					}
				}
				boneAnimate(root, frameNum); // Actualiza la animación
				frameNum++;

			}
        }	
		// drawing and rendering logic
		BeginDrawing();
		ClearBackground(GRAY);
		meshDraw(&body, root, frameNum);
		DrawBones(root);

		// Muestra el número de frame actual
        DrawText(TextFormat("Frame Number: %d", frameNum), 10, 10, 20, WHITE);
// Mostrar el hueso seleccionado en pantalla
        DrawText(TextFormat("Selected Bone: %s", currentBone->name), 10, 40, 20, WHITE);

        // Mostrar el largo y el ángulo del hueso seleccionado
        DrawText(TextFormat("Length: %.2f", currentBone->l), 10, 70, 20, WHITE);
        DrawText(TextFormat("Angle: %.2f", currentBone->a), 10, 100, 20, WHITE);

		EndDrawing();
	}
	
	boneDumpAnim(root, 0);
	// Clean up and close window
	CloseWindow();
	return 0;
}
