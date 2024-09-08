#ifndef _BONES_H
#define _BONES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "raylib.h"
#include "raymath.h"
#include "raygui.h"


#define MAX_CHCOUNT                99  /* Max children count */
#define MAX_BONECOUNT              99  /* Max bone count */
#define BONE_ABSOLUTE_ANGLE        0x01 /* Bone angle is absolute or relative to parent */
#define BONE_ABSOLUTE_POSITION     0x02 /*Bone pos is absolute in the world or relative to the parent */
#define BONE_ABSOLUTE              (BONE_ABSOLUTE_ANGLE | BONE_ABSOLUTE_POSITION)
#define MAX_KFCOUNT                4096 /* Max keyframe count */
#define MAX_VXCOUNT                4   /* Max vertex count per bone */
#define MAX_MESHVXCOUNT            (MAX_VXCOUNT * MAX_BONECOUNT) /* Max vertices in mesh */

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct {
    float x, y, r, g, b;
} Vertex;

typedef struct {
    uint32_t time;
    float angle, length;
    int partex, layer, coll;
} Keyframe;

typedef struct Bone {
    char name[99];               /* Name of the bone */
    float x, y;                  /* Starting point x & y */
    float a;                     /* Angle, in radians */
    float l;                     /* Length of the bone */
    float offA, offL;            /* Offset values for interpolation */
    uint8_t flags;               /* Bone flags */
    uint8_t childCount;          /* Number of children */
    struct Bone *child[MAX_CHCOUNT]; /* Pointers to children */
    struct Bone *parent;        /* Parent bone */
    uint32_t keyframeCount, frame, depth, effect;
    Keyframe keyframe[MAX_KFCOUNT];
    uint32_t vertexCount;
    Vertex vertex[MAX_VXCOUNT];
} Bone;

typedef struct _BoneVertex{
    Vertex v;                    /* Info on this vertex */
    int boneCount,t;               /* Number of bones this vertex is connected to */
    float weight[MAX_BONECOUNT]; /* Weight for each bone connected */
    Bone *bone[MAX_BONECOUNT];   /* Pointer to connected bones */
} BoneVertex;

typedef struct s_mesh{
    int vertexCount;             /* Number of vertices in this mesh */
    BoneVertex v[MAX_MESHVXCOUNT]; /* Vertices of the mesh */
} t_mesh;

typedef struct _BonesXY{
    float bonex, boney, bonea, bonel; /* Bone data */
} BonesXY;

// Variables globales
extern char *currentName;
extern BonesXY bonesdata[MAX_BONECOUNT];
extern uint32_t maxTime;
extern float cut_x;
extern float cut_y;
extern float cut_xb;
extern float cut_yb;
extern Texture2D textures[];
/* Function declarations */
Bone* boneFreeTree(Bone *root);
void boneDumpAnim(Bone *root, uint8_t level);
Bone* boneFindByName(Bone *root, char *name);
int boneInterAnimation(Bone *root, Bone *introot, int time, float intindex);
int boneAnimate(Bone *root, int time);
Bone* boneCleanAnimation(Bone *root, t_mesh *body, char *path);
int boneAnimateReverse(Bone *root, int time);
void boneListNames(Bone *root, char names[MAX_BONECOUNT][99]);
Bone* boneChangeAnimation(Bone *root, char *path);
Bone* boneLoadStructure(const char *path);
Bone* boneAddChild(Bone *root, float x, float y, float a, float l, uint8_t flags, char *name);
void DrawBones(Bone *root, bool drawBonesEnabled);
void meshLoadData(char *file, t_mesh *mesh, Bone *root);
void LoadTextures(void);
Matrix GetBoneMatrix(Bone *bone);
void getPartTexture(int tex,int contTxt);
float getBoneAngle(Bone* b);
Vector2 applyBoneMove(Bone *bone, Vector2 vertex);
int compareVerticesByLayer(const void *a, const void *b);
void meshDraw(t_mesh *mesh, Bone *root, int time);
void animationLoadKeyframes(const char *path, Bone *root);

#endif
