#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "bones.h"
#include "raylib.h"
#include "rlgl.h"
#include "config.h"
#include "gui.h"

Bone* currentBone = NULL;
Bone* bones[MAX_BONES];
int boneCount = 0;
int selectedBone = 0;
int frameNum = 0;
Bone* root = NULL;
Camera2D camera = { 0 };



int main(void)
{
	SetTraceLogLevel(LOG_NONE);
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Bone Animation");
	
	SetWindowState(FLAG_WINDOW_RESIZABLE);

	camera.target = (Vector2){ 0.0f, 0.0f };
	camera.rotation = 0.0f;
	camera.zoom = 1.0f;

	SetTargetFPS(60);

	rlEnableDepthTest();
	rlEnableColorBlend();

    t_mesh body;
    root = boneLoadStructure("Bbs_Skel.txt");
    root->x = GetScreenWidth() / 2.0f;
    root->y = GetScreenHeight() / 1.1f;

    char names[MAX_BONES][99] = {0};
    boneListNames(root, names);

    meshLoadData("Bbs_Mesh.txt", &body, root);
    LoadTextures();

    animationLoadKeyframes("Bbs_SkelAnim.txt", root);

    frameNum = 0;
	
	InitializeGUI();

    while (!WindowShouldClose())
    {
		int windowWidth = GetScreenWidth();
        int windowHeight = GetScreenHeight();

        // Ajustar la cámara si el tamaño de la ventana cambia
        camera.offset = (Vector2){ 0.0f, 0.0f };
		float zoomX = (float)windowWidth / (float)SCREEN_WIDTH;
        float zoomY = (float)windowHeight / (float)SCREEN_HEIGHT;
        camera.zoom = (zoomX < zoomY) ? zoomX : zoomY; // Mantener la relación de aspecto

        keyframeStatus = UpdateBoneProperties(currentBone, frameNum);
		
		UpdateGUI();
		
		BeginDrawing();

        ClearBackground(GRAY);		
	
		DrawGUI();

		BeginMode2D(camera);

		meshDraw(&body, root, frameNum);
        DrawBones(root, drawBones);

		EndMode2D(); 
		EndDrawing();
	}
	boneDumpAnim(root, 0);
	CloseWindow();
	return 0;
}
