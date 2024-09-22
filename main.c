#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "bones.h"
#include "raylib.h"
#include "rlgl.h"
#include "config.h"
#include "gui.h"

Bone*		currentBone = NULL;
Bone*		bones[MAX_BONES];
int			boneCount = 0;
int			selectedBone = 0;
int			frameNum = 0;
Bone*		root = NULL;
Camera2D	camera = { 0 };
bool		animMode = false;
t_mesh		mesh;


int main(void)
{
	SetTraceLogLevel(LOG_NONE);
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "RayPuppet");
	SetWindowState(FLAG_WINDOW_RESIZABLE);
	MaximizeWindow();
	camera.target = (Vector2){ 0.0f, 0.0f };
	camera.rotation = 0.0f;
	camera.zoom = 1.0f;
	SetTargetFPS(60);
	//rlEnableDepthTest();
	//rlEnableColorBlend();
    root = boneLoadStructure("Skel/Skel.txt");
    root->x = GetScreenWidth() / 2.0f;
    root->y = GetScreenHeight() / 1.1f;
    char names[MAX_BONES][99] = {0};
    boneListNames(root, names);
    meshLoadData("Skel/SkelMesh.txt", &mesh, root);
    LoadTextures();
    animationLoadKeyframes("Skel/SkelAnim.txt", root);

    frameNum = 1;
	InitializeGUI();

    while (!WindowShouldClose())
    {
		int windowWidth = GetScreenWidth();
        int windowHeight = GetScreenHeight();

        camera.offset = (Vector2){ 0.0f, 0.0f };
		float zoomX = (float)windowWidth / (float)SCREEN_WIDTH;
        float zoomY = (float)windowHeight / (float)SCREEN_HEIGHT;
        camera.zoom = (zoomX < zoomY) ? zoomX : zoomY;

		BeginDrawing();
		ClearBackground(GRAY);		
		
		DrawGUI();
		keyframeStatus = UpdateBoneProperties(currentBone, frameNum);
		UpdateGUI();

		BeginMode2D(camera);
		meshDraw(&mesh, root, frameNum);
        DrawBones(root, drawBones);
		EndMode2D();
		
		if(animMode)
			mouseAnimate(currentBone, frameNum);
		else
			DrawOnTop(currentBone, frameNum);

		EndDrawing();
	}
	CloseWindow();
	return 0;
}
