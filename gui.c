#include "config.h"
#include "bones.h"
#include "gui.h"
#include <string.h>

// Declaraciones externas
extern Bone* currentBone;   // Variable global externa para el hueso actual
extern Bone* bones[MAX_BONES]; // Lista de huesos
extern int boneCount;       // Contador de huesos
extern int selectedBone;    // Índice del hueso seleccionado
extern int frameNum;        // Número de fotograma actual
extern Bone* root;          // Hueso raíz

// Variables internas
static Rectangle scrollPanelBounds = {0};
static Rectangle contentBounds = {0};
static Vector2 scrollOffset = {0};
static bool isKeyframe[MAX_BONES] = {0};
bool forwAnim = 0;
bool revAnim = 0;
int keyframeStatus = 0;
float frameNumFloat;
bool drawBonesEnabled = true;
int direction = 0 ;
float newSliderValue;

void HandleDirectionChange(int newDirection, int maxTime) {
    int currentFrame = frameNum;

    // Si la dirección cambia, recorremos el resto de la animación actual antes de cambiar de dirección
    if (newDirection != direction) {
        if (direction == 1) {
            while (currentFrame <= maxTime) {
                boneAnimate(root, ++currentFrame);
                frameNum = currentFrame;
            }
        } else {
            while (currentFrame >= 0) {
                boneAnimateReverse(root, --currentFrame);
                frameNum = currentFrame;
            }
        }
        direction = newDirection; // Actualizamos la dirección
    }
}

void UpdateAnimationWithSlider(float sliderValue, int maxTime) {
    int newFrame = (int)sliderValue;
    int newDirection = (newFrame > frameNum) ? 1 : (newFrame < frameNum) ? -1 : direction;

    // Manejo del cambio de dirección
    HandleDirectionChange(newDirection, maxTime);

    // Ajuste fino hasta alcanzar el nuevo frame
    while (frameNum != newFrame) {
        if (direction == 1) {
            boneAnimate(root, ++frameNum);
        } else {
            boneAnimateReverse(root, --frameNum);
        }
    }
}

void UpdateGUI(void) {
    if (IsKeyPressed(KEY_P)) {
        HandleDirectionChange(1, maxTime);  // Si se presiona P, avanzamos en la animación
        frameNum++;
        if (frameNum > maxTime)
            frameNum = 0;
        boneAnimate(root, frameNum);
        frameNumFloat = frameNum;
    } else if (IsKeyPressed(KEY_O)) {
        HandleDirectionChange(-1, maxTime);  // Si se presiona O, retrocedemos en la animación
        frameNum--;
        if (frameNum < 0)
            frameNum = maxTime;
        boneAnimateReverse(root, frameNum);
        frameNumFloat = frameNum;
    }

    if (forwAnim) {  // Animación hacia adelante
        HandleDirectionChange(1, maxTime);
        frameNum++;
        if (frameNum >= maxTime)
            frameNum = 0;
        boneAnimate(root, frameNum);
        frameNumFloat = frameNum;
    }

    if (revAnim) {  // Animación en reversa
        HandleDirectionChange(-1, maxTime);
        frameNum--;
        if (frameNum < 0)
            frameNum = maxTime;
        boneAnimateReverse(root, frameNum);
        frameNumFloat = frameNum;
    }
}
/*
void UpdateAnimationWithSlider(float sliderValue, int maxTime)
{
    int newFrame = (int)sliderValue;
    int currentFrame = frameNum;

	int newDirection = (newFrame > currentFrame) ? 1 : (newFrame < currentFrame) ? -1 : direction;	
    if (newDirection != direction)
    {
        if (direction == 1)
			while (currentFrame <= maxTime)
            {
                boneAnimate(root, ++currentFrame);
                frameNum = currentFrame;
            }
        else
            while (currentFrame >= 0)
            {
                boneAnimateReverse(root, --currentFrame);
                frameNum = currentFrame;
            }
        direction = newDirection;
    }
    while (currentFrame != newFrame)
    {
        if (direction == 1)
            boneAnimate(root, ++currentFrame);
        else
            boneAnimateReverse(root, --currentFrame);
        frameNum = currentFrame;
    }
}*/

static void LoadBonesBoxRecursive(Bone* bone, Bone* bones[], int* index, int* count)
{
    if (*index >= MAX_BONES)
		return;
    bones[*index] = bone;
    (*index)++;
    (*count)++;
    for (int i = 0; i < bone->childCount; i++)
        LoadBonesBoxRecursive(bone->child[i], bones, index, count);
}

void LoadBonesBox(Bone* root, Bone* bones[], int* count)
{
    int index = 0;
    *count = 0;
    LoadBonesBoxRecursive(root, bones, &index, count);
}

void InitializeGUI(void)
{
	forwAnim = 0;
	revAnim = 0;	
	frameNumFloat = (float)frameNum;
	LoadBonesBox(root, bones,&boneCount);
	currentBone = bones[selectedBone];
    scrollPanelBounds = (Rectangle){SCREEN_WIDTH - 190, 40, 180, SCREEN_HEIGHT - 140};
    contentBounds = (Rectangle){0, 0, 180, boneCount * 30};
    scrollOffset = (Vector2){0, 0};
    memset(isKeyframe, 0, sizeof(isKeyframe));
    if (currentBone != NULL)
        for (int j = 0; j < currentBone->keyframeCount; j++)
            if (currentBone->keyframe[j].time >= 0 && currentBone->keyframe[j].time <= maxTime)
                isKeyframe[currentBone->keyframe[j].time] = true;
}

void DrawGUI(void)
{
	// Toggles
	DrawRectangleRec((Rectangle){10, SCREEN_HEIGHT - 165, 130, 110}, WHITE);
	GuiCheckBox((Rectangle){20, SCREEN_HEIGHT - 160, 50, 30}, "Draw Bones", &drawBonesEnabled);
	if (GuiCheckBox((Rectangle){20, SCREEN_HEIGHT - 125, 50, 30}, "Animating", &forwAnim))
		revAnim = false;
	if (GuiCheckBox((Rectangle){20, SCREEN_HEIGHT - 90, 50, 30}, "Reverse", &revAnim))
		forwAnim = false;

	// Draw Panel
	BeginScissorMode(scrollPanelBounds.x, scrollPanelBounds.y, 
			scrollPanelBounds.width, scrollPanelBounds.height);
	GuiScrollPanel(scrollPanelBounds, NULL, contentBounds, &scrollOffset, NULL);
	for (int i = 0; i < boneCount; i++) {
		Rectangle buttonRect = (Rectangle){scrollPanelBounds.x, 
			scrollPanelBounds.y + i * 30 + scrollOffset.y, scrollPanelBounds.width - 20, 20};
		if (GuiButton(buttonRect, bones[i]->name))
		{
			selectedBone = i;
			currentBone = bones[selectedBone];
			memset(isKeyframe, 0, sizeof(isKeyframe));
            if (currentBone != NULL)
                for (int j = 0; j < currentBone->keyframeCount; j++)
                    if (currentBone->keyframe[j].time >= 0 && currentBone->keyframe[j].time <= maxTime)
                        isKeyframe[currentBone->keyframe[j].time] = true;
        }
    }
    EndScissorMode();
    // Draw the slider
	if (!forwAnim & !revAnim)
	{
		Rectangle sliderBounds = (Rectangle){20, SCREEN_HEIGHT - 30, SCREEN_WIDTH - 40, 20};
		newSliderValue = GuiSlider(sliderBounds, "", NULL, &frameNumFloat, 0.0f, (float)maxTime);

        if (newSliderValue != frameNumFloat) {
            forwAnim = false;
            revAnim = false;
            UpdateAnimationWithSlider(frameNumFloat, maxTime);
        }

		for (int i = 0; i <= maxTime; i++)
		{
			float posX = sliderBounds.x + (i * ((sliderBounds.width - 20) / (float)maxTime));
			Color textColor = isKeyframe[i] ? GREEN : WHITE;
			DrawText(TextFormat("%d", i), posX + 5, sliderBounds.y - 20, 15, textColor);
		}
	}
    // Info
    DrawText(TextFormat("Bone: %s", currentBone ? currentBone->name : "None"), 10, 40, 20, WHITE);
    DrawText(TextFormat("Length: %.2f", currentBone ? currentBone->l : 0.0f), 10, 70, 20, WHITE);
    DrawText(TextFormat("Angle: %.2f", currentBone ? currentBone->a : 0.0f), 10, 100, 20, WHITE);
    DrawText(TextFormat("Frame Number: %d", frameNum), 10, 10, 20, WHITE);
    if (keyframeStatus)
        DrawText("Keyframe encontrado", 10, 130, 20, GREEN);
    else
        DrawText("Keyframe no encontrado", 10, 130, 20, RED);
}
