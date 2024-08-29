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
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define MAX_BONES 100
#define MAX_KEYFRAMES 50

Bone* currentBone = NULL;
Bone* bones[MAX_BONES];
int boneCount = 0;
int selectedBone = -1;
//int maxKeyframes = MAX_KEYFRAMES;
int frameNum = 0;
Bone* root = NULL;

void LoadBonesBox(Bone* root, Bone* bones[], int* count)
{
    int index = 0;

    void LoadBonesBoxRecursive(Bone* bone)
    {
        if (index >= MAX_BONES) return;
        bones[index++] = bone;
        (*count)++;
        for (int i = 0; i < bone->childCount; i++)
        {
            LoadBonesBoxRecursive(bone->child[i]);
        }
    }

    LoadBonesBoxRecursive(root);
}

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
        return 0;
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
            break;
        }
    }

    return found;
}

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Bone Animation");
    SetTargetFPS(60);

    rlEnableDepthTest();
    rlEnableColorBlend();

    t_mesh body;
    root = boneLoadStructure("Bbs_Skel.txt");
    root->x = GetScreenWidth() / 2.0f;
    root->y = GetScreenHeight() / 1.0f;

    char names[MAX_BONES][99] = {0};
    boneListNames(root, names);
    LoadBonesBox(root, bones, &boneCount);

    meshLoadData("Bbs_Mesh.txt", &body, root);
    LoadTextures();

    animationLoadKeyframes("Bbs_SkelAnim.txt", root);

    frameNum = 1;
    bool animating = false;



    currentBone = root;
    int keyframeStatus = 0;

	// Define the scroll panel's rectangle and content height
    Rectangle scrollPanelBounds = (Rectangle){SCREEN_WIDTH - 190, 40, 180, SCREEN_HEIGHT - 140};
    Rectangle contentBounds = (Rectangle){0, 0, 180, boneCount * 30}; // Adjust content height based on bone count
    Vector2 scrollOffset = {0, 0}; // Initialize the scroll offset


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
			frameNum++;
			if (frameNum >= maxTime)
			{
				frameNum = 0;
			}
			boneAnimate(root, frameNum);
		}

        BeginDrawing();
        ClearBackground(GRAY);
        meshDraw(&body, root, frameNum);
        DrawBones(root);

        DrawText(TextFormat("Frame Number: %d", frameNum), 10, 10, 20, WHITE);
        DrawText(TextFormat("Bone: %s", currentBone ? currentBone->name : "None"), 10, 40, 20, WHITE);
        DrawText(TextFormat("Length: %.2f", currentBone ? currentBone->l : 0.0f), 10, 70, 20, WHITE);
        DrawText(TextFormat("Angle: %.2f", currentBone ? currentBone->a : 0.0f), 10, 100, 20, WHITE);
		if (keyframeStatus)
            DrawText("Keyframe encontrado", 10, 130, 20, GREEN);
		else
			DrawText("Keyframe no encontrado", 10, 130, 20, RED);

		//Draw Panel
		GuiScrollPanel(scrollPanelBounds, NULL, contentBounds, &scrollOffset, NULL);
		for (int i = 0; i < boneCount; i++)
		{
			Rectangle buttonRect = (Rectangle){scrollPanelBounds.x,
				scrollPanelBounds.y + i * 30 + scrollOffset.y, scrollPanelBounds.width - 20, 20};
			if (GuiButton(buttonRect, bones[i]->name))
			{
				selectedBone = i;
				currentBone = bones[selectedBone];
				maxTime = currentBone->keyframeCount;
			}
		}
		// Draw the slider
		float frameNumFloat = (float)frameNum;
		float *sliderValuePtr = &frameNumFloat;
		GuiSlider((Rectangle){20, SCREEN_HEIGHT - 90, SCREEN_WIDTH - 40, 20},
				"Keyframe", NULL, sliderValuePtr, 0.0f, (float)(maxTime - 1));

        EndDrawing();
    }

    return 0;
}

