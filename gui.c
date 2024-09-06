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
bool drawBones = true;
int direction = 0 ;
//float newSliderValue;
float tempLength = 0.0f;
float tempAngle = 0.0f;
bool tempValuesSet = false;
float cameraZoom = 1.0f;

void ResetBoneToOriginalState(Bone* bone)
{
	if (tempValuesSet)
	{	
		bone->l = tempLength;
		bone->a = tempAngle;
		tempValuesSet = false;
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
				if (!tempValuesSet)
				{
					tempLength = bone->l;       // Guardar longitud original
					tempAngle = bone->a;        // Guardar ángulo original
					tempValuesSet = true;       // Marcar como guardado
				}
				bone->l += 1.0f;
				bone->keyframe[i].length += 1.0f;
			}
			if (IsKeyDown(KEY_DOWN))
			{
				if (!tempValuesSet)
				{
					tempLength = bone->l;
					tempAngle = bone->a;
					tempValuesSet = true;
				}
				bone->l -= 1.0f;
				bone->keyframe[i].length -= 1.0f;
			}
			if (IsKeyDown(KEY_RIGHT))
			{
				if (!tempValuesSet)
				{
					tempLength = bone->l;
					tempAngle = bone->a;
					tempValuesSet = true;
				}
				bone->a += 0.05f;
				bone->keyframe[i].angle += 0.05f;
			}
			if (IsKeyDown(KEY_LEFT))
			{
				if (!tempValuesSet)
				{
					tempLength = bone->l;
					tempAngle = bone->a;
					tempValuesSet = true;
				}
				bone->a -= 0.05f;
				bone->keyframe[i].angle -= 0.05f;
			}
			break;
		}
	}
	return found;
}            


void HandleDirectionChange(int newDirection, int maxTime)
{
	int currentFrame = frameNum;

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
		direction = newDirection; // Actualizamos la dirección
	}
}

void UpdateAnimationWithSlider(float sliderValue, int maxTime)
{
	int newFrame = (int)sliderValue;
	int newDirection = (newFrame > frameNum) ? 1 : (newFrame < frameNum) ? -1 : direction;

	HandleDirectionChange(newDirection, maxTime);
	while (frameNum != newFrame)
	{
		if (direction == 1)
			boneAnimate(root, ++frameNum);
		else
			boneAnimateReverse(root, --frameNum);
	}
}

void UpdateGUI(void)
{
	if (IsKeyPressed(KEY_P))
	{
		if (tempValuesSet)
			ResetBoneToOriginalState(currentBone);
		HandleDirectionChange(1, maxTime);
		frameNumFloat++;
		if (frameNumFloat > maxTime)
			frameNumFloat = 0;
		UpdateAnimationWithSlider(frameNumFloat, maxTime);

	}
	else if (IsKeyPressed(KEY_O))
	{
		HandleDirectionChange(-1, maxTime);
		frameNumFloat--;
		if (frameNumFloat < 0)
			frameNumFloat = maxTime;
		UpdateAnimationWithSlider(frameNumFloat, maxTime);
		if (tempValuesSet)
			ResetBoneToOriginalState(currentBone);
	}

	if (forwAnim)
	{  
		HandleDirectionChange(1, maxTime);
		frameNum++;
		if (frameNum >= maxTime)
			frameNum = 0;
		boneAnimate(root, frameNum);
		frameNumFloat = frameNum;
		if (tempValuesSet)
			ResetBoneToOriginalState(currentBone);
	}

	if (revAnim)
	{  
		HandleDirectionChange(-1, maxTime);
		frameNum--;
		if (frameNum < 0)
			frameNum = maxTime;
		boneAnimateReverse(root, frameNum);
		frameNumFloat = frameNum;
		if (tempValuesSet)
			ResetBoneToOriginalState(currentBone);
	}
}

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
	float z = camera.zoom;

	int originalTextSize = GuiGetStyle(DEFAULT, TEXT_SIZE);
	GuiSetStyle(DEFAULT, TEXT_SIZE, (int)(originalTextSize * z * 1.2));
	DrawRectangleRec((Rectangle){10 * z, (SCREEN_HEIGHT - 165) * z, 140 * z, 100 * z}, WHITE);
	GuiCheckBox((Rectangle){20 * z, (SCREEN_HEIGHT - 160) * z, 50 * z, 30 * z}, "Draw Bones", &drawBones);

	if (GuiCheckBox((Rectangle){20 * z,(SCREEN_HEIGHT - 130) * z,50 * z,30 * z},"Animating", &forwAnim))
		revAnim = false;

	if (GuiCheckBox((Rectangle){20 * z, (SCREEN_HEIGHT - 100) * z, 50 * z, 30 * z}, "Reverse", &revAnim))
		forwAnim = false;

	// Draw Panel
	Rectangle scaledPanel = (Rectangle){scrollPanelBounds.x * z, scrollPanelBounds.y * z, 
		scrollPanelBounds.width * z, scrollPanelBounds.height * z};
	Rectangle scaledContent = (Rectangle){contentBounds.x * z, contentBounds.y * z, 
		contentBounds.width * z, contentBounds.height * z};

	scaledContent.height = boneCount * 30 * z; // Altura dinámica basada en la cantidad de huesos

	BeginScissorMode(scaledPanel.x, scaledPanel.y, scaledPanel.width, scaledPanel.height);
	GuiScrollPanel(scaledPanel, NULL, scaledContent, &scrollOffset, NULL);

	for (int i = 0; i < boneCount; i++) {
		if (bones[i]->keyframe[0].partex != -1)
			GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, ColorToInt(BLUE));
		else
			GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, ColorToInt(GRAY));

		Rectangle buttonRect = (Rectangle){
			scaledPanel.x, 
				(scaledPanel.y + i * 30 * z + scrollOffset.y), 
				scaledPanel.width - 20 * z, 20 * z
		};

		if (GuiButton(buttonRect, bones[i]->name)) {
			if (tempValuesSet)
				ResetBoneToOriginalState(currentBone);
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
	if (!forwAnim & !revAnim) {
		Rectangle sliderBounds = (Rectangle){20 * z, (SCREEN_HEIGHT - 30) * z, 
			(SCREEN_WIDTH - 40) * z, 20 * z};
		GuiSlider(sliderBounds, "", NULL, &frameNumFloat, 0.0f, (float)maxTime);

		if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) &&
				CheckCollisionPointRec(GetMousePosition(), sliderBounds)) {
			if (tempValuesSet) 
				ResetBoneToOriginalState(currentBone);    
		}
		UpdateAnimationWithSlider(frameNumFloat, maxTime);

		for (int i = 0; i <= maxTime; i++) {
			float posX = sliderBounds.x + (i * ((sliderBounds.width - 20 * z) / (float)maxTime));
			Color textColor = isKeyframe[i] ? GREEN : WHITE;

			DrawText(TextFormat("%d", i), posX, sliderBounds.y - 25 * z, 15 * z, textColor);
		}
	}

	// Info
	DrawText(TextFormat("Bone: %s", currentBone ? currentBone->name : "None"),10 * z,40 * z,20 * z, WHITE);
	DrawText(TextFormat("Length: %.2f", currentBone ? currentBone->l : 0.0f),10 * z,70 * z, 20 * z, WHITE);
	DrawText(TextFormat("Angle: %.2f", currentBone ? currentBone->a : 0.0f),10 * z,100 * z, 20 * z, WHITE);
	DrawText(TextFormat("Frame Number: %d", frameNum), 10 * z, 10 * z, 20 * z, WHITE);

	if (keyframeStatus)
		DrawText("Keyframe encontrado", 10 * z, 130 * z, 20 * z, GREEN);
	else
		DrawText("Keyframe no encontrado", 10 * z, 130 * z, 20 * z, RED);

	GuiSetStyle(DEFAULT, TEXT_SIZE, originalTextSize);
}
