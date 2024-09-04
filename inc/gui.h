#ifndef GUI_H
#define GUI_H

#include "raylib.h" 
#include "bones.h"

extern int selectedBone;
extern bool forwAnim;
extern bool revAnim;
extern float frameNumFloat;
extern int keyframeStatus;
extern bool drawBonesEnabled;
extern float frameNumFloat;

// Declaración de funciones
void InitializeGUI(void);
void UpdateGUI(void);
void DrawGUI(void);

#endif // GUI_H

