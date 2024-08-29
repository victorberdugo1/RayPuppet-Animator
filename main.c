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


void AdvanceBoneSelection(Bone* root)
{
    if (currentBone == NULL)
	{
        currentBone = root;
        return;
    }
    if (currentBone->childCount > 0)
        currentBone = currentBone->child[0];
    else
	{
        while (currentBone->parent != NULL)
		{
            Bone* parent = currentBone->parent;
            for (int i = 0; i < parent->childCount; i++)
			{
                if (parent->child[i] == currentBone && i + 1 < parent->childCount)
				{
                    currentBone = parent->child[i + 1];
                    return;
                }
            }
            currentBone = parent;
        }
    }
}

int UpdateBoneProperties(Bone* bone, int time)
{
    if (bone == NULL)
		return (0); 
    int found = 0;
    for (int i = 0; i < bone->keyframeCount; i++)
    {
        if (bone->keyframe[i].time == time)
        {
            found = 1;

            if (IsKeyDown(KEY_UP))
            {
                bone->l += 1.0f;
                bone->keyframe[i].length += 1.0f;
            }
            if (IsKeyDown(KEY_DOWN))
            {
                bone->l -= 1.0f;
                bone->keyframe[i].length -= 1.0f;
            }
            if (IsKeyDown(KEY_RIGHT))
            {
                bone->a += 0.05f;
                bone->keyframe[i].angle += 0.05f;
            }
            if (IsKeyDown(KEY_LEFT))
            {
                bone->a -= 0.05f;
                bone->keyframe[i].angle -= 0.05f;
            }
            break ;
        }
    }

    return (found);
}


int main(int argc, char *argv[])
{
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
	int keyframeStatus = 0;

	// Main game loop
	while (!WindowShouldClose())
	{
		if (IsKeyPressed(KEY_L))
		{
			AdvanceBoneSelection(root);
		}
		keyframeStatus = UpdateBoneProperties(currentBone, frameNum);

		if (IsKeyPressed(KEY_P))
		{
			frameNum++;
			if (frameNum >= maxTime)
			{
				frameNum = 0;
			}
			boneAnimate(root, frameNum);
		}
		else if (IsKeyPressed(KEY_O))
		{
			frameNum--;
			if (frameNum < 0) 
			{
				frameNum = maxTime;
			}
			boneAnimateReverse(root, frameNum);
		}


        if (animating)
        {
			static float frameTimer = 0.0f;
			frameTimer += GetFrameTime();

			// Cambiar el frame cada 0.1 segundos (ajusta este valor según sea necesario)
			if (frameTimer >= 0.02f)
			{
				frameTimer = 0.0f;
				if (frameNum < 0)	//			if (frameNum >= maxTime)
				{
					frameNum = maxTime;
					if (argc > 1)
					{
						frameNum = maxTime;
					}
				}
				boneAnimateReverse(root, frameNum);
				frameNum--;

			}
        }	
		// drawing and rendering logic
		BeginDrawing();
		ClearBackground(GRAY);
		meshDraw(&body, root, frameNum);
		DrawBones(root);

		// Muestra info 
        DrawText(TextFormat("Frame Number: %d", frameNum), 10, 10, 20, WHITE);
        DrawText(TextFormat("Selected Bone: %s", currentBone->name), 10, 40, 20, WHITE);
        DrawText(TextFormat("Length: %.2f", currentBone->l), 10, 70, 20, WHITE);
        DrawText(TextFormat("Angle: %.2f", currentBone->a), 10, 100, 20, WHITE);
		if (keyframeStatus)
            DrawText("Keyframe encontrado", 10, 130, 20, GREEN);
        else
            DrawText("Keyframe no encontrado", 10, 130, 20, RED);
		EndDrawing();
	}
	
	boneDumpAnim(root, 0);
	// Clean up and close window
	CloseWindow();
	return 0;
}
