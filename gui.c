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
float tempLength = 0.0f;
float tempAngle = 0.0f;
bool tempValuesSet = false;
float cameraZoom = 1.0f;
static Texture2D selectedTexture = { 0 };
static bool showTexture = false;
//bool textureSelected = false;
bool textureJustClosed = false;



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
					tempLength = bone->l;
					tempAngle = bone->a;
					tempValuesSet = true;
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
		direction = newDirection;
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

int GetBoneTextureIndex(Vector2 clickPosition, Bone* bones[], int boneCount, float zoom)
{
	int textureCount = 0;

	for (int i = 0; i < boneCount; i++)
	{
		if (bones[i]->keyframe[0].partex == -1)
			continue;

		float destWidth = 100.0f * zoom;
		float destHeight = 100.0f * zoom;
		float centerX = bones[i]->x * zoom;
		float centerY = bones[i]->y * zoom;

		if (clickPosition.x >= (centerX - destWidth / 2) &&
				clickPosition.x <= (centerX + destWidth / 2) &&
				clickPosition.y >= (centerY - destHeight / 2) &&
				clickPosition.y <= (centerY + destHeight / 2))
		{
			int selectedBoneIndex = i;

			for (int j = 0; j <= selectedBoneIndex; j++)
			{
				if (bones[j]->keyframe[0].partex > -1)
					textureCount++;
			}
			currentBone = bones[selectedBoneIndex];
			return textureCount;
		}          
	}
	return -1;
}

void DrawOnTop(Bone* bone, int time)
{

	Vector2 mousePosition = GetMousePosition();

	if (textureJustClosed)
	{
		if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) // Esperar a que se suelte el clic
		{
			textureJustClosed = false; // Permitir clics en los huesos de nuevo
		}
		return; // Bloquear clics hasta que se libere el botón
	}

    if (showTexture && selectedTexture.id != 0)
    {
		int gridSize = contTxt; 
		float partWidth = (float)selectedTexture.width / gridSize;
		float partHeight = (float)selectedTexture.height / gridSize;

		Rectangle destRect = {
			(GetScreenWidth() - (selectedTexture.width * camera.zoom)) / 2.0f,
			(GetScreenHeight() - (selectedTexture.height * camera.zoom)) / 2.0f,
			(float)selectedTexture.width * camera.zoom,
			(float)selectedTexture.height * camera.zoom
		};

		Rectangle sourceRect = { 0, 0, (float)selectedTexture.width, (float)selectedTexture.height };
		Vector2 origin = { 0, 0 };
		DrawTexturePro(selectedTexture, sourceRect, destRect, origin, 0.0f, WHITE);

		if (CheckCollisionPointRec(mousePosition, destRect))
		{
			float relativeMouseX = (mousePosition.x - destRect.x) / camera.zoom;
			float relativeMouseY = (mousePosition.y - destRect.y) / camera.zoom;

			int gridX = (int)(relativeMouseX / partWidth);
			int gridY = (int)(relativeMouseY / partHeight);

			float highlightX = destRect.x + (gridX * partWidth * camera.zoom);
			float highlightY = destRect.y + (gridY * partHeight * camera.zoom);
			float highlightWidth = partWidth * camera.zoom;
			float highlightHeight = partHeight * camera.zoom;

			DrawRectangle(highlightX, highlightY, highlightWidth, highlightHeight, Fade(RED, 0.5f));

			if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
			{
				for (int i = 0; i < bone->keyframeCount; i++)
				{
					if (bone->keyframe[i].time == time)
					{
						bone->keyframe[i].partex = gridX + gridY * gridSize;
						showTexture = false;
						textureJustClosed = true;
					}
				}
			}
		}

		for (int y = 0; y < gridSize; y++)
		{
			for (int x = 0; x < gridSize; x++)
			{
				float rectX = destRect.x + (x * partWidth * camera.zoom);
				float rectY = destRect.y + (y * partHeight * camera.zoom);

				DrawRectangleLines(rectX, rectY, partWidth * camera.zoom, partHeight * camera.zoom, RED);
			}
		}
	}

	
    if (!textureJustClosed)
    {
	for (int i = 0; i < bone->keyframeCount; i++)
		if (bone->keyframe[i].time == time)
			if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && bone->keyframe[0].partex > -1)
			{
				int textureIndex = GetBoneTextureIndex(mousePosition, bones, boneCount, camera.zoom);
				if (textureIndex > 0)
					if (textureIndex >= 0 && textureIndex < 20)
					{
						selectedTexture = textures[textureIndex];
						showTexture = true;
					}
			}
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

	int visibleButtonIndex = 0;

	scaledContent.height = boneCount * 30 * z; // Altura dinámica basada en la cantidad de huesos

	BeginScissorMode(scaledPanel.x, scaledPanel.y, scaledPanel.width, scaledPanel.height);
	
	GuiScrollPanel(scaledPanel, NULL, scaledContent, &scrollOffset, NULL);

	for (int i = 0; i < boneCount; i++) {
		if (showTexture && bones[i]->keyframe[0].partex == -1) {
			continue; // Saltar este botón y pasar al siguiente
		}
		if (bones[i]->keyframe[0].partex != -1)
			GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, ColorToInt(BLUE));
		else
			GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, ColorToInt(GRAY));

		Rectangle buttonRect = (Rectangle){
			scaledPanel.x, 
				(scaledPanel.y + visibleButtonIndex * 30 * z + scrollOffset.y), 
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
			// Si se obtiene un índice válido, actualizar la textura seleccionada
			if (showTexture) {
				int textureCount = 0; 
				for (int j = 0; j <= selectedBone; j++)
					if (bones[j] != NULL && bones[j]->keyframe[0].partex > -1)
						textureCount++;
				selectedTexture = textures[textureCount];
				showTexture = true;  // Mostrar la textura
			}
			else
				showTexture = false; // No hay textura para mostrar
		}
		visibleButtonIndex++;
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
		frameNumFloat = (int)(frameNumFloat + 0.5f);
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
