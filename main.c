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
	// Main game loop
	while (!WindowShouldClose())
	{
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

		EndDrawing();
	}
	
	boneDumpAnim(root, 0);
	// Clean up and close window
	CloseWindow();
	return 0;
}
