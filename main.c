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
bool		openFile = false;
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
	/*/rlEnableDepthTest();
	//rlEnableColorBlend();
	root = boneLoadStructure("Skel/Skel.txt");
	root->x = GetScreenWidth() / 2.0f;
	root->y = GetScreenHeight() / 1.1f;

	meshLoadData("Skel/SkelMesh.txt", &mesh, root);
	LoadTextures();
	animationLoadKeyframes("Skel/SkelAnim.txt", root);
	*/
	static char	fileName[128];
	char		command[256];

	snprintf(command, sizeof(command), "zenity --file-selection --title=\"Select File\"");
	FILE *fp = popen(command, "r");
	if (fp == NULL)
		return -1;
	fgets(fileName, sizeof(fileName), fp);
	fileName[strcspn(fileName, "\n")] = 0;
	pclose(fp);
	printf("%s", fileName);
	char baseName[128];
	strcpy(baseName, fileName);
	char *dot = strrchr(baseName, '.');
	if (dot) *dot = '\0'; 
	char skeletonPath[128], meshPath[128], animPath[128];
	sprintf(skeletonPath, "%s", fileName);
	sprintf(meshPath, "%sMesh.txt", baseName);
	sprintf(animPath, "%sAnim.txt", baseName);
	root = boneLoadStructure(skeletonPath);
	meshLoadData(meshPath, &mesh, root);
	LoadTextures(mesh.vertexCount);
	animationLoadKeyframes(animPath, root);
	root->x = GetScreenWidth() / 2.0f;
	root->y = GetScreenHeight() / 1.1f;

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
		if(!openFile)
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
