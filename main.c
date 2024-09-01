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
int selectedBone = 0;
int frameNum = 0;
Bone* root = NULL;

/*
guardar una variable que identifique si la animacion esta llendo 
adelante o atras
guardar el ultimo moviemitn si fue ++ o --
y si cambia 
se debe ir si es ++ hasta el final 
y si es -- hasta 0
y luego ya animar hasta el frame con la nueva direccion
}*/

void UpdateAnimationWithSlider(float sliderValue)
{
    int newFrame = (int)sliderValue;
    int currentFrame = frameNum;

    if (currentFrame != newFrame)
    {
        if (newFrame > currentFrame)
        {
            while (currentFrame < newFrame)
            {
                boneAnimate(root, ++currentFrame);
            }
        }
        else
        {
            while (currentFrame > newFrame)
            {
                boneAnimateReverse(root, --currentFrame);
            }
        }
        frameNum = newFrame;
    }
}
void LoadBonesBox(Bone* root, Bone* bones[], int* count)
{
    int index = 0;

    void LoadBonesBoxRecursive(Bone* bone)
    {
        if (index >= MAX_BONES)
			return;
        bones[index++] = bone;
        (*count)++;
        for (int i = 0; i < bone->childCount; i++)
            LoadBonesBoxRecursive(bone->child[i]);
    }
    LoadBonesBoxRecursive(root);
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
    root->y = GetScreenHeight() / 1.1f;

    char names[MAX_BONES][99] = {0};
    boneListNames(root, names);

    meshLoadData("Bbs_Mesh.txt", &body, root);
    LoadTextures();

    animationLoadKeyframes("Bbs_SkelAnim.txt", root);

    frameNum = 0;
    bool animating = false;
	
	/* GUI */
    LoadBonesBox(root, bones, &boneCount);
	int keyframeStatus = 0;
	currentBone = bones[selectedBone];
	Rectangle scrollPanelBounds = (Rectangle){SCREEN_WIDTH - 190, 40, 180, SCREEN_HEIGHT - 140};
    Rectangle contentBounds = (Rectangle){0, 0, 180, boneCount * 30};
    Vector2 scrollOffset = {0, 0};
	bool isKeyframe[maxTime + 1];
	bool drawBonesEnabled = true;
	float frameNumFloat = (float)frameNum;
	memset(isKeyframe, 0, sizeof(isKeyframe));
	for (int j = 0; j < currentBone->keyframeCount; j++)
		if (currentBone->keyframe[j].time >= 0 && currentBone->keyframe[j].time <= maxTime)
			isKeyframe[currentBone->keyframe[j].time] = true;

    while (!WindowShouldClose())
    {
        keyframeStatus = UpdateBoneProperties(currentBone, frameNum);

        if (IsKeyPressed(KEY_P))
		{
			frameNum++;
			if (frameNum > maxTime)
				frameNum = 0;
			boneAnimate(root, frameNum);
			frameNumFloat = frameNum;

		}
		else if (IsKeyPressed(KEY_O))
		{
			frameNum--;
			if (frameNum < 0)
				frameNum = maxTime;
			boneAnimateReverse(root, frameNum);
			frameNumFloat = frameNum;

		}
		if (animating)
		{
			/*frameNum--;
			if (frameNum < 0)
				frameNum = maxTime;
			boneAnimateReverse(root, frameNum);*/

			frameNum++;
			if (frameNum >= maxTime)
				frameNum = 0;
			boneAnimate(root, frameNum);
			frameNumFloat = frameNum;
		}

        BeginDrawing();
        ClearBackground(GRAY);		
		//Toggles
		DrawRectangleRec((Rectangle){10, SCREEN_HEIGHT - 160, 130, 85},  WHITE);
		GuiCheckBox((Rectangle){20, SCREEN_HEIGHT - 150, 50, 30}, "Draw Bones", &drawBonesEnabled);
		GuiCheckBox((Rectangle){20, SCREEN_HEIGHT - 115, 50, 30}, "Animating", &animating);
		//Draw Panel
		BeginScissorMode(scrollPanelBounds.x, scrollPanelBounds.y, 
				scrollPanelBounds.width, scrollPanelBounds.height);
		GuiScrollPanel(scrollPanelBounds, NULL, contentBounds, &scrollOffset, NULL);
		for (int i = 0; i < boneCount; i++)
		{
			Rectangle buttonRect = (Rectangle){scrollPanelBounds.x,
				scrollPanelBounds.y + i * 30 + scrollOffset.y, scrollPanelBounds.width - 20, 20};
			if (GuiButton(buttonRect, bones[i]->name))
			{
				selectedBone = i;
				currentBone = bones[selectedBone];
				memset(isKeyframe, 0, sizeof(isKeyframe));
				for (int j = 0; j < currentBone->keyframeCount; j++)
					if (currentBone->keyframe[j].time >= 0 && currentBone->keyframe[j].time <= maxTime)
						isKeyframe[currentBone->keyframe[j].time] = true;
			}
		}
		EndScissorMode();
		// Draw the slider 
		if (!animating)
		{

			Rectangle sliderBounds = (Rectangle){20, SCREEN_HEIGHT - 30, SCREEN_WIDTH - 40, 20};
			float newSliderValue = GuiSlider(sliderBounds, "", NULL, &frameNumFloat, 0.0f, (float)maxTime);
			if (newSliderValue != frameNumFloat)
				animating = false;
			UpdateAnimationWithSlider(frameNumFloat);
			for (int i = 0; i <= maxTime; i++)
			{
				float posX = sliderBounds.x + (i * ((sliderBounds.width - 20) / (float)maxTime));
				Color textColor = isKeyframe[i] ? GREEN : WHITE;
				DrawText(TextFormat("%d", i), posX + 5, sliderBounds.y - 20, 15, textColor);
			}
		}

		//Info
        DrawText(TextFormat("Bone: %s", currentBone ? currentBone->name : "None"), 10, 40, 20, WHITE);
        DrawText(TextFormat("Length: %.2f", currentBone ? currentBone->l : 0.0f), 10, 70, 20, WHITE);
        DrawText(TextFormat("Angle: %.2f", currentBone ? currentBone->a : 0.0f), 10, 100, 20, WHITE);
        DrawText(TextFormat("Frame Number: %d", frameNum), 10, 10, 20, WHITE);
		if (keyframeStatus)
            DrawText("Keyframe encontrado", 10, 130, 20, GREEN);
		else
			DrawText("Keyframe no encontrado", 10, 130, 20, RED);

		//boneAnimate(root, frameNum);

		meshDraw(&body, root, frameNum);
        DrawBones(root, drawBonesEnabled);

		EndDrawing();
	}
	//boneDumpAnim(root, 0);
	CloseWindow();
	return 0;
}
