//cc main.c -o main -L. -lraylib -lm -ldl -lpthread -lGL -lX11
//cc main.c -o main libraylib.a -lm 
//cc main.c bones.c -o main -L. -lraylib -lm -ldl -lpthread -lGL -lX11

#include <stddef.h>
#include <string.h>
#include "bones.h"
#include "raylib.h"
#include "rlgl.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Bone Animation Example");
    SetTargetFPS(60);
    rlEnableDepthTest();
    rlEnableColorBlend();
    t_mesh body;
    Bone *root = boneLoadStructure("Bbs_Skel.txt");
    Bone *introot = boneLoadStructure("Bbs_Skel.txt");
    //introot = boneCleanAnimation(introot, &body, "Bbs_Skel3.txt");
    //introot = boneChangeAnimation(introot, "Bbs_Skel2.txt");
    char names[MAX_BONECOUNT][99] = {0};
    boneListNames(root, names);
    const char *currentName = names[0];
    meshLoadData("Bbs_Mesh.txt", &body, root);
    LoadTextures();
    int frameNum = 0;
    bool animating = true;
    float intindex = 0.0f;
    float alocintp = 0.0f;
    while (!WindowShouldClose())
    {
        if (animating)
        {
            if (!boneAnimate(root, introot, frameNum++, intindex))
            {
                frameNum = 0;
                intindex = alocintp;
            }
			frameNum++;
        }
        BeginDrawing();
        ClearBackground(BLACK);
        rlEnableDepthTest();
        rlEnableColorBlend();
        meshDraw(&body, root, frameNum);
        //DrawGLBone(root, frameNum);
        DrawText("Sample Text", 10, 10, 20, DARKGRAY);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
