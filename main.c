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


void animationLoadKeyframes(const char *path, Bone *root) {
    FILE *file;
    float angle, length;
    int layer, partex, coll;
    uint32_t time;
    char name[100], buffer[4096], *ptr, *token, *rest;
    Bone *bone;

    if (!(file = fopen(path, "r"))) {
        fprintf(stderr, "Can't open file %s for reading\n", path);
        return;
    }

    while (fgets(buffer, sizeof(buffer), file)) {
        if (strlen(buffer) < 3)
            continue;

        sscanf(buffer, "%s", name);
        bone = boneFindByName(root, name);
        if (!bone) {
            fprintf(stderr, "Bone %s not found\n", name);
            continue;
        }

        ptr = buffer + strlen(name) + 1;
        while ((token = strtok_r(ptr, " ", &rest))) {
            ptr = NULL;
            sscanf(token, "%d", &time);
            if ((token = strtok_r(NULL, " ", &rest)) == NULL) break;
            sscanf(token, "%d", &partex);
            if ((token = strtok_r(NULL, " ", &rest)) == NULL) break;
            sscanf(token, "%d", &layer);
            if ((token = strtok_r(NULL, " ", &rest)) == NULL) break;
            sscanf(token, "%d", &coll);
            if ((token = strtok_r(NULL, " ", &rest)) == NULL) break;
            sscanf(token, "%f", &angle);
            if ((token = strtok_r(NULL, " ", &rest)) == NULL) break;
            sscanf(token, "%f", &length);

            if (bone->keyframeCount >= MAX_KFCOUNT) {
                fprintf(stderr, "Warning: Keyframe count exceeded for bone %s\n", name);
                continue;
            }

            Keyframe *k = &(bone->keyframe[bone->keyframeCount]);
            k->time = time;
            k->partex = partex;
            k->layer = layer;
            k->coll = coll;
            k->angle = angle;
            k->length = length;
            bone->keyframeCount++;

            printf("Added keyframe to bone %s: Time=%u, Partex=%d, Layer=%d, Coll=%d, Angle=%.2f, Length=%.2f\n",
                name, time, partex, layer, coll, angle, length);
        }
    }

    fclose(file);
}        


int main(void)
{
    // Initialization of window and rendering settings
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Bone Animation Example");
    SetTargetFPS(60);
    rlEnableDepthTest();
    rlEnableColorBlend();

    t_mesh body;
    Bone *root = boneLoadStructure("Bbs_Skel.txt");
    root->x = GetScreenWidth() / 2.0f;
    root->y = GetScreenHeight() / 1.0f;
    Bone *introot = boneLoadStructure("Bbs_Skel.txt");

    // Load mesh data
    char names[MAX_BONECOUNT][99] = {0};
    boneListNames(root, names);
    const char *currentName = names[0];
    meshLoadData("Bbs_Mesh.txt", &body, root);
    LoadTextures();

	// Load animations
    animationLoadKeyframes("Bbs_SkelAnim.txt", root);
	//animationLoadKeyframes("Bbs_SkelAnim.txt", introot);
    // Print skeleton information to the console
    //PrintSkeletonInfo(root, &body);

    int frameNum = 0;
    bool animating = true;
    float intindex = 0.0f;
    float alocintp = 0.0f;

    // Main game loop
    while (!WindowShouldClose())
    {
if (animating)
        {
            printf("Before animation: frameNum = %d\n", frameNum); // Debug print

            if (!boneAnimate(root, introot, frameNum, intindex))
            {
                frameNum = 0;
                intindex = alocintp;
            }
            else
            {
                frameNum++;
            }

            printf("After animation: frameNum = %d\n", frameNum); // Debug print
        }
        // Drawing and rendering logic
        BeginDrawing();
        ClearBackground(GRAY);
        meshDraw(&body, root, frameNum);
        DrawBones(root);
        EndDrawing();
    }

    // Clean up and close window
    CloseWindow();
    return 0;
}
