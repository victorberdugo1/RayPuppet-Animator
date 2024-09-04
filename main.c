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

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Bone Animation");
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
        keyframeStatus = UpdateBoneProperties(currentBone, frameNum);

		UpdateGUI();
		
		BeginDrawing();
        ClearBackground(GRAY);		
		DrawGUI();

		meshDraw(&body, root, frameNum);
        DrawBones(root, drawBonesEnabled);

		EndDrawing();
	}
	//boneDumpAnim(root, 0);
	CloseWindow();
	return 0;
}
