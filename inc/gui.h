#ifndef GUI_H
#define GUI_H

#include "raylib.h" 
#include "bones.h"

extern int		selectedBone;
extern bool		forwAnim;
extern bool		revAnim;
extern float	frameNumFloat;
extern int		keyframeStatus;
extern bool		drawBones;
extern bool		animMode;
extern bool		openFile;
extern float	frameNumFloat;
extern Camera2D	camera;
// Declaración de funciones
void	InitializeGUI(void);
void	UpdateGUI(void);
void	DrawGUI(void);
void	DrawOnTop(Bone* bone, int time);
void	mouseAnimate(Bone* bone, int time);
int		UpdateBoneProperties(Bone* bone, int time);
Bone* CleanAndLoadModel(Bone *root, t_mesh* mesh);

#endif // GUI_H
