//cc main.c -o main -L. -lraylib -lm -ldl -lpthread -lGL -lX11
//cc main.c -o main libraylib.a -lm 
//cc main.c bones.c -o main -L. -lraylib -lm -ldl -lpthread -lGL -lX11

#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "bones.h"
#include "raylib.h"
#include "rlgl.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

void PrintBoneInfo(Bone *bone, int depth) {
    if (bone == NULL) {
        return;
    }
    // Print current bone information with indentation based on depth
    for (int i = 0; i < depth; i++) {
        printf("  ");
    }
    printf("Bone Name: %s\n", bone->name);
    printf("Position: (%f, %f)\n", bone->x, bone->y);
    printf("Angle: %f radians\n", bone->a);
    printf("Length: %f\n", bone->l);
    printf("Number of children: %d\n", bone->childCount);
    printf("\n");
    // Recursively print information about children
    for (int i = 0; i < bone->childCount; i++) {
        if (bone->child[i] != NULL) {
            PrintBoneInfo(bone->child[i], depth + 1);
        }
    }
}

void PrintSkeletonInfo(Bone *root, t_mesh *mesh) {
    if (root == NULL || mesh == NULL) {
        printf("No skeleton or mesh data loaded.\n");
        return;
    }

    printf("Skeleton Information:\n");
    printf("----------------------\n");
    // Print root bone information and recursively print all children
    PrintBoneInfo(root, 0);

    printf("Vertex Count: %d\n", mesh->vertexCount);
    for (int i = 0; i < mesh->vertexCount; i++) {
        printf("Vertex %d: (%f, %f)\n", i, mesh->v[i].v.x, mesh->v[i].v.y);
        printf("  Texture Index: %d\n", mesh->v[i].t);

        printf("  Bones and Weights:\n");
        for (int j = 0; j < mesh->v[i].boneCount; j++) {
            if (mesh->v[i].bone[j]) {
                printf("    Bone %d: %s, Weight: %f\n", j, mesh->v[i].bone[j]->name, mesh->v[i].weight[j]);
            } else {
                printf("    Bone %d: NULL\n", j);
            }
        }
        printf("\n");
    }
}


int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Bone Animation Example");
    SetTargetFPS(60);
    rlEnableDepthTest();
    rlEnableColorBlend();
    t_mesh body;
    Bone *root = boneLoadStructure("Bbs_Skel.txt");
	root->x = GetScreenWidth() / 2.0f;
    root->y = GetScreenHeight() / 1.0f;
    Bone *introot = boneLoadStructure("Bbs_Skel.txt");
    //introot = boneCleanAnimation(introot, &body, "Bbs_Skel3.txt");
    //introot = boneChangeAnimation(introot, "Bbs_Skel2.txt");
    char names[MAX_BONECOUNT][99] = {0};
    boneListNames(root, names);
    const char *currentName = names[0];
    meshLoadData("Bbs_Mesh.txt", &body, root);
    LoadTextures();

    // Print skeleton information to the console
    //PrintSkeletonInfo(root, &body);

    int frameNum = 0;
    bool animating = false;
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
        ClearBackground(GRAY);
        //rlEnableDepthTest();
        //rlEnableColorBlend();
        meshDraw(&body, root, frameNum);
        DrawBones(root);

		if (!frameNum) {
        	PrintSkeletonInfo(root, &body);
            frameNum = 1; // Marca la acción como realizada
        }
		//DrawText("Sample Text", 10, 10, 20, DARKGRAY);
        EndDrawing();
    }
	CloseWindow();
    return 0;
}

