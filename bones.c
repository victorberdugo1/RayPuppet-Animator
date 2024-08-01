#include <stddef.h> 
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include "raylib.h"
#include "bones.h"

char *currentName = NULL;
BonesXY bonesdata[MAX_BONECOUNT];
Texture2D textures[19];
int coordenada,contTxt;
float cut_x,cut_y,cut_xb,cut_yb;
char nameFTx[99];

Bone* boneFreeTree(Bone *root) {
    if (!root) return NULL;
    for (int i = 0; i < root->childCount; i++)
        boneFreeTree(root->child[i]);
    free(root);
    return NULL;
}

void boneDumpTree(Bone *root, uint8_t level) {
    if (!root) return;
    for (int i = 0; i < level; i++) printf("#");
    printf(" %3.1f %3.1f %3.1f %3.1f %d %s ", root->x, root->y, root->a, root->l, root->childCount, root->name);
    for (int f = 0; f < root->keyframeCount; f++)
        printf("%i %d %i %i %3.1f %3.1f ", root->keyframe[f].time, root->keyframe[f].partex, root->keyframe[f].layer, root->keyframe[f].coll, root->keyframe[f].angle, root->keyframe[f].length);
    printf("\n");
    for (int i = 0; i < root->childCount; i++)
        boneDumpTree(root->child[i], level + 1);
}

void boneDumpAnim(Bone *root, uint8_t level) {
    if (!root) return;
    printf("%3.1f %3.1f %s ", root->a, root->l, root->name);
    for (int f = 0; f < root->keyframeCount; f++)
        printf("%i %d %i %i %3.1f %3.1f ", root->keyframe[f].time, root->keyframe[f].partex, root->keyframe[f].layer, root->keyframe[f].coll, root->keyframe[f].angle, root->keyframe[f].length);
    printf("\n");
    for (int i = 0; i < root->childCount; i++)
        boneDumpAnim(root->child[i], level + 1);
}

Bone *boneFindByName(Bone *root, char *name)
{
	int i;
	Bone *p;
	if (!root)
		return NULL;
	if (!strcmp(root->name, name))
		return root;
	for (i = 0; i < root->childCount; i++)
	{
		p = boneFindByName(root->child[i], name);
		if (p)
			return p;
	}
	return NULL;
}

/*
int boneAnimate(Bone *root, Bone *introot, int time, float intindex) {
    if (!root) return 0;
    for (int i = 0; i < root->keyframeCount; i++) {
        if (root->keyframe[i].time == time) {
            if (i < root->keyframeCount - 1) {
                float tim = root->keyframe[i + 1].time - root->keyframe[i].time;
                root->depth = root->keyframe[i].layer;
                root->collition = root->keyframe[i].coll;
                root->frame = root->keyframe[i].partex;
                root->offA = ((root->keyframe[i + 1].angle - root->keyframe[i].angle) +
                    ((introot->keyframe[i + 1].angle - introot->keyframe[i].angle) -
                    (root->keyframe[i + 1].angle - root->keyframe[i].angle)) * intindex) / tim;
                root->offL = ((root->keyframe[i + 1].length - root->keyframe[i].length) +
                    ((introot->keyframe[i + 1].length - introot->keyframe[i].length) -
                    (root->keyframe[i + 1].length - root->keyframe[i].length)) * intindex) / tim;
            } else {
                root->offA = root->offL = 0;
            }
        } else if (root->keyframe[i].time > time) return 1;
    }
    root->a += root->offA;
    root->l += root->offL;
    int others = 0;
    for (int i = 0; i < root->childCount; i++) {
        if (boneAnimate(root->child[i], introot->child[i], time, intindex))
            others = 1;
    }
    return others;
}*/

int boneAnimate(Bone *root, Bone *introot, int time, float intindex) {
    if (!root) return 0;
    int keyframeUpdated = 0;
    for (int i = 0; i < root->keyframeCount; i++) {
        if (root->keyframe[i].time == time) {
            if (i < root->keyframeCount - 1) {
                float tim = root->keyframe[i + 1].time - root->keyframe[i].time;
                root->depth = root->keyframe[i].layer;
                root->collition = root->keyframe[i].coll;
                root->frame = root->keyframe[i].partex;
                root->offA = ((root->keyframe[i + 1].angle - root->keyframe[i].angle) +
                    ((introot->keyframe[i + 1].angle - introot->keyframe[i].angle) -
                    (root->keyframe[i + 1].angle - root->keyframe[i].angle)) * intindex) / tim;
                root->offL = ((root->keyframe[i + 1].length - root->keyframe[i].length) +
                    ((introot->keyframe[i + 1].length - introot->keyframe[i].length) -
                    (root->keyframe[i + 1].length - root->keyframe[i].length)) * intindex) / tim;
            } else {
                root->offA = root->offL = 0;
            }
            keyframeUpdated = 1;
            break;
        } else if (root->keyframe[i].time > time) {
            break;
        }
    }

    if (keyframeUpdated) {
        root->a += root->offA;
        root->l += root->offL;
        printf("Updated bone %s: angle=%.2f, length=%.2f, offA=%.2f, offL=%.2f\n", root->name, root->a, root->l, root->offA, root->offL);
    }

    int others = 0;
    for (int i = 0; i < root->childCount; i++) {
        if (boneAnimate(root->child[i], introot->child[i], time, intindex)) {
            others = 1;
        }
    }
    return keyframeUpdated || others;
}


int bonePlusAnimate(Bone *root, Bone *introot, int time, float intindex) {
    if (!root) return 0;

    int i, others = 0;
    float ang, len, tim;

    for (i = 0; i < root->keyframeCount; i++) {
        if (root->keyframe[i].time == time) {
            if (i != root->keyframeCount - 1) {
                root->depth = root->keyframe[i].layer;
                root->collition = root->keyframe[i].coll;
                root->frame = root->keyframe[i].partex;

                tim = root->keyframe[i + 1].time - root->keyframe[i].time;
                ang = root->keyframe[i + 1].angle - root->keyframe[i].angle;
                len = root->keyframe[i + 1].length - root->keyframe[i].length;

                root->offA = ang / tim;
                root->offL = len / tim;
            } else {
                root->offA = 0;
                root->offL = 0;
            }
        } else if (root->keyframe[i].time > time) {
            others = 1;
        }
    }

    root->a += root->offA;
    root->l += root->offL;

    for (i = 0; i < root->childCount; i++) {
        if (bonePlusAnimate(root->child[i], introot->child[i], time, intindex)) {
            others = 1;
        }
    }

    return others;
}

int boneLessAnimate(Bone *root, int time) {
    if (!root) return 0;
    for (int i = root->keyframeCount - 1; i > 0; i--) {
        if (root->keyframe[i].time == time) {
            if (i > 0) {
                float tim = root->keyframe[i].time - root->keyframe[i - 1].time;
                root->depth = root->keyframe[i - 1].layer;
                root->collition = root->keyframe[i].coll;
                root->frame = root->keyframe[i - 1].partex;
                root->offA = (root->keyframe[i].angle - root->keyframe[i - 1].angle) / tim;
                root->offL = (root->keyframe[i].length - root->keyframe[i - 1].length) / tim;
            } else {
                root->offA = root->offL = 0;
            }
        } else if (root->keyframe[i].time < time) return 1;
    }
    root->a += root->offA;
    root->l += root->offL;
    int others = 0;
    for (int i = 0; i < root->childCount; i++) {
        if (boneLessAnimate(root->child[i], time))
            others = 1;
    }
    return others;
}

Bone *boneCleanAnimation(Bone *root, t_mesh *body, char *path) {
    Bone *temp;
	FILE *file;
	float x, y, angle, length;
	int depth, actualLevel, flags;
	char name[99], depthStr[99], buffer[2048], animBuf[2048], *ptr, *token;
	if (!(file = fopen(path, "r"))) {
		fprintf(stderr, "Can't open file %s for reading\n", path);
		return NULL;
	}
	root = NULL;
	temp = NULL;
	actualLevel = 0;
	while (!feof(file)) {
		memset(animBuf, 0, 2048);
		fgets(buffer, 2048, file);
		sscanf(buffer, "%s %f %f %f %f %d %s %[^\n]", depthStr, &x, &y, &angle, &length, &flags, name, animBuf);
		if (strlen(buffer) < 3)
			continue;
		depth = strlen(depthStr) - 1;
		if (depth < 0 || depth > MAX_CHCOUNT) {
			fprintf(stderr, "Wrong bone depth (%s)\n", depthStr);
			return NULL;
		}
		for (; actualLevel > depth; actualLevel--)
			temp = temp->parent;
		if (!root && !depth) {
			root = boneAddChild(NULL, x, y, angle, length, flags, name);
			temp = root;
		} else
			temp = boneAddChild(temp, x, y, angle, length, flags, name);
		if (strlen(animBuf) > 3) {
			ptr = animBuf;
			while ((token = strtok(ptr, " "))) {
				ptr = NULL;
			}
			temp->keyframeCount = 0;
		}
		actualLevel++;
	}
    meshLoadData("Bbs_Mesh.txt", body, root);
	return root;
}

void boneListNames(Bone *root, char names[MAX_BONECOUNT][99]) {
    int i, present;
    if (!root)
        return;
    present = 0;
    for (i = 0; (i < MAX_BONECOUNT) && (names[i][0] != '\0'); i++)
        if (!strcmp(names[i], root->name)) {
            present = 1;
            break;
        }
    if (!present && (i < MAX_BONECOUNT)) {
        strcpy(names[i], root->name);
        if (i + 1 < MAX_BONECOUNT)
            names[i + 1][0] = '\0';
    }
    for (i = 0; i < root->childCount; i++)
        boneListNames(root->child[i], names);
}

Bone *boneChangeAnimation(Bone *root, char *path) {
    Bone *temp;
    FILE *file;
    float angle, length;
    int partex, layer, coll;
    uint32_t time;
    char name[99], buffer[4096], animBuf[4096], *ptr, *token;
    Keyframe *k;
    if (!(file = fopen(path, "r"))) {
        return NULL;
    }
    while (!feof(file)) {
        memset(animBuf, 0, 4096);
        fgets(buffer, 4096, file);
        sscanf(buffer, "%f %f %s %[^\n]", &angle, &length, name, animBuf);
        if (strlen(animBuf) > 2) {
            strcpy(currentName, name); // Utiliza strcpy en lugar de asignación directa
            temp = boneFindByName(root, currentName);
            temp->a = angle;
            temp->l = length;
            ptr = animBuf;
            while ((token = strtok(ptr, " "))) {
                ptr = NULL;
                sscanf(token, "%d", &time);
                token = strtok(ptr, " ");
                sscanf(token, "%i", &partex);
                token = strtok(ptr, " ");
                sscanf(token, "%i", &layer);
                token = strtok(ptr, " ");
                sscanf(token, "%i", &coll);
                token = strtok(ptr, " ");
                sscanf(token, "%f", &angle);
                token = strtok(ptr, " ");
                sscanf(token, "%f", &length);
                if (temp->keyframeCount >= MAX_KFCOUNT) {
                    fprintf(stderr, "Can't add more keyframes\n");
                    continue;
                }
                k = &(temp->keyframe[temp->keyframeCount]);
                k->time = time;
                k->partex = partex;
                k->layer = layer;
                k->coll = coll;
                k->angle = angle;
                k->length = length;
                temp->keyframeCount++;
            }
        }
    }
    return root;
}
/*
Bone *boneLoadStructure(const char *path) {
    Bone *root = NULL, *temp = NULL;
    FILE *file;
    float x, y, angle, length;
    int layer, depth, actualLevel = 0, flags, partex, coll;
    uint32_t time;
    char name[99], depthStr[99], buffer[4096], animBuf[4096], *ptr, *token, *rest;
    if (!(file = fopen(path, "r"))) {
        fprintf(stderr, "Can't open file %s for reading\n", path);
        return NULL;
    }
    while (fgets(buffer, sizeof(buffer), file)) {
        if (strlen(buffer) < 3)
            continue;
        memset(animBuf, 0, sizeof(animBuf));
        sscanf(buffer, "%s %f %f %f %f %d %s %[^\n]", depthStr, &x, &y, &angle, &length, &flags, name, animBuf);
        depth = strlen(depthStr) - 1;
        if (depth < 0 || depth > MAX_CHCOUNT) {
            fprintf(stderr, "Wrong bone depth (%s)\n", depthStr);
            fclose(file);
            return NULL;
        }
        for (; actualLevel > depth; actualLevel--)
            temp = temp->parent;
        if (!root && depth == 0) {
            root = boneAddChild(NULL, x, y, angle, length, flags, name);
            temp = root;
        } else {
            temp = boneAddChild(temp, x, y, angle, length, flags, name);
        }
        if (strlen(animBuf) > 3) {
            ptr = animBuf;
            while ((token = strtok_r(ptr, " ", &rest))) {
                ptr = NULL;
                sscanf(token, "%d", &time);
                if ((token = strtok_r(NULL, " ", &rest)) == NULL) break;
                sscanf(token, "%i", &partex);
                if ((token = strtok_r(NULL, " ", &rest)) == NULL) break;
                sscanf(token, "%i", &layer);
                if ((token = strtok_r(NULL, " ", &rest)) == NULL) break;
                sscanf(token, "%i", &coll);
                if ((token = strtok_r(NULL, " ", &rest)) == NULL) break;
                sscanf(token, "%f", &angle);
                if ((token = strtok_r(NULL, " ", &rest)) == NULL) break;
                sscanf(token, "%f", &length);
                if (temp->keyframeCount >= MAX_KFCOUNT) {
                    continue;
                }
                Keyframe *k = &(temp->keyframe[temp->keyframeCount]);
                k->time = time;
                k->partex = partex;
                k->layer = layer;
                k->coll = coll;
                k->angle = angle;
                k->length = length;
                temp->keyframeCount++;
            }
        }
        actualLevel++;
    }
    fclose(file);
    return root;
}*/

Bone* boneLoadStructure(const char *path) {
    Bone *root = NULL, *temp = NULL;
    FILE *file;
    float x, y, angle, length;
    int layer, depth, actualLevel = 0, flags, partex, coll;
    uint32_t time;
    char name[99], depthStr[99], buffer[4096], animBuf[4096], *ptr, *token, *rest;

    if (!(file = fopen(path, "r"))) {
        fprintf(stderr, "Can't open file %s for reading\n", path);
        return NULL;
    }

    while (fgets(buffer, sizeof(buffer), file)) {
        if (strlen(buffer) < 3)
            continue;

        memset(animBuf, 0, sizeof(animBuf));
        sscanf(buffer, "%s %f %f %f %f %d %s %[^\n]", depthStr, &x, &y, &angle, &length, &flags, name, animBuf);
        depth = strlen(depthStr) - 1;

        if (depth < 0 || depth > MAX_CHCOUNT) {
            fprintf(stderr, "Wrong bone depth (%s)\n", depthStr);
            fclose(file);
            return NULL;
        }

        for (; actualLevel > depth; actualLevel--)
            temp = temp->parent;

        if (!root && depth == 0) {
            root = boneAddChild(NULL, x, y, angle, length, flags, name);
            temp = root;
        } else {
            temp = boneAddChild(temp, x, y, angle, length, flags, name);
        }

        actualLevel++;
    }

    fclose(file);
    return root;
}



Bone *boneAddChild(Bone *root, float x, float y, float a, float l, uint8_t flags, char *name) {
    Bone *t;
    int i;
    if (!root) {
        if (!(root = (Bone *)malloc(sizeof(Bone))))
            return NULL;
        root->parent = NULL;
        root->childCount = 0; 
    } else if (root->childCount < MAX_CHCOUNT) {
        if (!(t = (Bone *)malloc(sizeof(Bone))))
            return NULL;
        t->parent = root;
        root->child[root->childCount++] = t;
        root = t; 
    } else {
        return NULL;
    }
    root->x = x;
    root->y = y;
    root->a = a;
    root->l = l;
    root->flags = flags;
    root->childCount = 0; 
    if (name) {
        strncpy(root->name, name, sizeof(root->name) - 1);
        root->name[sizeof(root->name) - 1] = '\0';
    } else {
        strcpy(root->name, "Bone");
    }
    for (i = 0; i < MAX_CHCOUNT; i++) {
        root->child[i] = NULL;
    }
    return root;
}

void DrawBones(Bone *root) {
    if (root == NULL) return;

    Vector2 startPos = { root->x, root->y };
    Vector2 endPos = {
        startPos.x + root->l * cos(root->a),
        startPos.y + root->l * sin(root->a)
    };

    DrawLineEx(startPos, endPos, 2.0f, GREEN);
    DrawCircleV(startPos, 5, BLUE);

    for (int i = 0; i < root->childCount; i++) {
        if (root->child[i] != NULL) {
            root->child[i]->x = endPos.x;
            root->child[i]->y = endPos.y;
            DrawBones(root->child[i]);
        }
    }
}

void meshLoadData(char *file, t_mesh *mesh, Bone *root) {
    FILE *fd = fopen(file, "r");
    if (!fd) return;
    char buffer[4096], blist[4096];
    int i, t, j;
    float x, y;
    if (fgets(buffer, sizeof(buffer), fd) && sscanf(buffer, "%d", &mesh->vertexCount) == 1) {
        for (i = 0; i < mesh->vertexCount; i++) {
            if (fgets(buffer, sizeof(buffer), fd) && sscanf(buffer, "%d %f %f %[^\n]", &t, &x, &y, blist) == 4) {
                mesh->v[i].v.x = x;
                mesh->v[i].v.y = y;
                mesh->v[i].t = t;
                char *str = blist;
                j = 0;
                char *tok = strtok(str, " ");
                while (tok && j < 10) {
                    mesh->v[i].bone[j] = boneFindByName(root, tok);
                    if (mesh->v[i].bone[j]) {
                        tok = strtok(NULL, " ");
                        mesh->v[i].weight[j] = (tok) ? atof(tok) : 0.0f;
                        j++;
                    }
                    tok = strtok(NULL, " ");
                }
                mesh->v[i].boneCount = j;
            } else {
                fclose(fd);
                return;
            }
        }
    }
    fclose(fd);
}

void LoadTextures(void) {
    char path[64];
    int count = 19;

    for (int i = 1; i <= count; i++) {
        sprintf(path, "Textures/Skel_%d.png", i);
        Image image = LoadImage(path);
        if (image.data == NULL) {
            continue;
        }
        textures[i] = LoadTextureFromImage(image);
        UnloadImage(image);
    }
}

Matrix GetBoneMatrix(Bone *bone) {
    Matrix mat = MatrixIdentity();
    mat = MatrixMultiply(mat, MatrixRotateZ(bone->a));
    mat = MatrixMultiply(mat, MatrixTranslate(bone->x, bone->y, 0.0f));
    return mat;
}

void getPartTexture(int tex) {
    int ab = tex % 4;
    int ord = tex / 4;
    cut_x = (float)ab / 4.0f;
    cut_y = (float)ord / 4.0f;
    cut_xb = (float)(ab + 1) / 4.0f;
    cut_yb = (float)(ord + 1) / 4.0f;
}

float getBoneAngle(Bone* b) {
    if (!b || !b->parent) return 0.0f;
    
    Bone* b2 = b->parent;
    float dx = b2->x - b->x;
    float dy = b2->y - b->y;
    float angle = atan2(dy, dx) * 180.0f / M_PI;
    
    if (angle < 0.0f) angle += 360.0f;
    
    return (angle - 90.0f);
}

Vector2 AplBoneTrans(Bone *bone, Vector2 vertex) {
    float totalAngle = getBoneAngle(bone);
    float radAngle = totalAngle * M_PI / 180.0f;

    Vector2 transformed;
    transformed.x = bone->x + vertex.x * cos(radAngle) - vertex.y * sin(radAngle);
    transformed.y = bone->y + vertex.x * sin(radAngle) + vertex.y * cos(radAngle);
    return transformed;
}

void meshDraw(t_mesh *mesh, Bone *root, int time) {
    for (int i = 0; i < mesh->vertexCount; i++) {
        BoneVertex *boneVertex = &mesh->v[i];
        Vector2 trfrmedPos = {0.0f, 0.0f};
        float totalAngle = 0.0f;
        float scaleFactor = 1.0f;
		Texture2D texture;
		int	partex[2];

		for (int j = 0; j < boneVertex->boneCount; j++) {
			Bone *bone = boneVertex->bone[j];
			if (bone) {
				Vector2 boneTrfrmedPos = AplBoneTrans(bone, (Vector2){boneVertex->v.x, boneVertex->v.y});
				trfrmedPos.x = boneTrfrmedPos.x;
				trfrmedPos.y = boneTrfrmedPos.y;
				totalAngle = getBoneAngle(bone);
				scaleFactor = boneVertex->weight[j];

				partex[j] = bone->frame;
				printf("%s  %d\n",bone->name,bone->frame);
				//partex = boneFindByName(root, mesh->v[j].bone[1]->name)->frame;
				//getPartTexture(partex);
			}
		}
		texture = textures[boneVertex->t];
		getPartTexture(partex[0]);

		Rectangle sourceRect = {
            cut_x * texture.width,
            cut_y * texture.height,
            (cut_xb - cut_x) * texture.width,
            (cut_yb - cut_y) * texture.height
        };

        float destWidth = 100.0f * scaleFactor;
        float destHeight = 100.0f * scaleFactor;

        Rectangle destRect = {trfrmedPos.x, trfrmedPos.y, destWidth, destHeight};
        Vector2 origin = {destWidth / 2.0f, destHeight / 2.0f};

        DrawTexturePro(texture, sourceRect, destRect, origin, totalAngle, WHITE);
    }
}

