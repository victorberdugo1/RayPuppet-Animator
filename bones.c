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

Bone *boneAddChild(Bone *root, float x, float y, float a, float l, uint8_t flags, char *name) {
    Bone *t;
    if (!root) {
        root = (Bone *)malloc(sizeof(Bone));
        if (!root) return NULL;
        root->parent = NULL;
    } else if (root->childCount < MAX_CHCOUNT) {
        t = (Bone *)malloc(sizeof(Bone));
        if (!t) return NULL;
        t->parent = root;
        root->child[root->childCount++] = t;
        root = t;
    } else return NULL;
    root->x = x; root->y = y; root->a = a; root->l = l; root->flags = flags;
    root->childCount = 0;
    strcpy(root->name, name ? name : "Bone");
    memset(root->child, 0, sizeof(root->child));
    return root;
}

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

void LoadTextures(void) {
    char path[64];
    for (int i = 1; i <= contTxt; i++) {
        sprintf(path, "Textures/%s_%d.png", nameFTx, i);
        Texture2D texture = LoadTexture(path);
        if (texture.id == 0) {
            CloseWindow();
            exit(1);
        }
        textures[i - 1] = texture;
    }
}

Bone* boneFindByName(Bone *root, char *name) {
    if (!root) return NULL;
    if (!strcmp(root->name, name)) return root;
    for (int i = 0; i < root->childCount; i++) {
        Bone *p = boneFindByName(root->child[i], name);
        if (p) return p;
    }
    return NULL;
}

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
}

int bonePlusAnimate(Bone *root, Bone *introot, int time, float intindex) {
    if (!root) return 0;
    for (int i = 0; i < root->keyframeCount; i++) {
        if (root->keyframe[i].time == time) {
            if (i < root->keyframeCount - 1) {
                float tim = root->keyframe[i + 1].time - root->keyframe[i].time;
                root->depth = root->keyframe[i].layer;
                root->collition = root->keyframe[i].coll;
                root->frame = root->keyframe[i].partex;
                root->offA = (root->keyframe[i + 1].angle - root->keyframe[i].angle) / tim;
                root->offL = (root->keyframe[i + 1].length - root->keyframe[i].length) / tim;
            } else {
                root->offA = root->offL = 0;
            }
        } else if (root->keyframe[i].time > time) return 1;
    }
    root->a += root->offA;
    root->l += root->offL;
    int others = 0;
    for (int i = 0; i < root->childCount; i++) {
        if (bonePlusAnimate(root->child[i], introot->child[i], time, intindex))
            others = 1;
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

Bone *boneLoadStructure(const char *path) {
    FILE *file = fopen(path, "r");
    if (!file) {
        fprintf(stderr, "Can't open file %s for reading\n", path);
        return NULL;
    }

    Bone *root = NULL, *temp = NULL;
    int actualLevel = 0;
    char depthStr[99], name[99], buffer[4096], animBuf[4096];
    float x, y, angle, length;
    int flags, partex, layer, coll;
    uint32_t time;

    while (fgets(buffer, sizeof(buffer), file)) {
        memset(animBuf, 0, sizeof(animBuf));
        sscanf(buffer, "%s %f %f %f %f %d %s %[^\n]", depthStr, &x, &y, &angle, &length, &flags, name, animBuf);

        if (strlen(buffer) < 3) continue;
        int depth = strlen(depthStr) - 1;
        if (depth < 0 || depth > MAX_CHCOUNT) {
            fprintf(stderr, "Wrong bone depth (%s)\n", depthStr);
            fclose(file);
            return NULL;
        }
        while (actualLevel > depth) {
            temp = temp->parent;
            actualLevel--;
        }
        if (!root && depth == 0) {
            root = boneAddChild(NULL, x, y, angle, length, flags, name);
            temp = root;
        } else {
            temp = boneAddChild(temp, x, y, angle, length, flags, name);
        }
        char *ptr = animBuf;
        while (sscanf(ptr, "%u %d %d %d %d %f %f", &time, &partex, &layer, &coll, &partex, &angle, &length) == 7) {
            if (temp->keyframeCount < MAX_KFCOUNT) {
                Keyframe *k = &temp->keyframe[temp->keyframeCount++];
                k->time = time;
                k->partex = partex;
                k->layer = layer;
                k->coll = coll;
                k->angle = angle;
                k->length = length;
            } else {
                fprintf(stderr, "Can't add more keyframes\n");
                break;
            }
            while (*ptr && *ptr != ' ') ptr++;
            while (*ptr == ' ') ptr++;
        }
        actualLevel++;
    }
    fclose(file);
    return root;
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

void DrawGLBone(Bone *root, int selected) {
    int i;
    if (strcmp(root->name, currentName) == 0) {
        selected = 1;
    }
    Vector2 position = { root->x, root->y };
    float angle = RAD2DEG * root->a;
    if (selected) {
        DrawLineEx(position, (Vector2){position.x + root->l, position.y}, 2.0f, RED);
    } else {
        DrawLineEx(position, (Vector2){position.x + root->l, position.y}, 2.0f, GREEN);
    }
    if (selected) {
        DrawCircleV(position, 5, RED);
    } else {
        DrawCircleV(position, 5, BLUE);
    }
    position.x += root->l * cos(root->a);
    position.y += root->l * sin(root->a);
    for (i = 0; i < root->childCount; i++) {
        DrawGLBone(root->child[i], selected);
        bonesdata[i].bonex = root->child[i]->l * cos(root->child[i]->a) - root->child[i]->l * sin(root->child[i]->a);
        bonesdata[i].boney = root->child[i]->l * sin(root->child[i]->a) + root->child[i]->l * cos(root->child[i]->a);
    }
}

void getBoneMatrix(Bone *b, Matrix *mat) {
    if (!b) return;
    *mat = MatrixIdentity();
    if (b->parent) {
        Matrix parentMat = MatrixIdentity();
        getBoneMatrix(b->parent, &parentMat);
        *mat = MatrixMultiply(*mat, parentMat);
    }
    Matrix translation = MatrixTranslate(b->x, b->y, 0.0f);
    Matrix rotation = MatrixRotateZ(b->a);
    *mat = MatrixMultiply(*mat, translation);
    *mat = MatrixMultiply(*mat, rotation);
}

Texture2D getPartTexture(int tex)
{
    int cont;
    int ab = 0;
    int ord = 0;
    for (cont = 0; cont < tex; cont++)
    {
        ab++;
        if (ab > 3)
        {
            ab = 0;
            ord++;
        }
    }

    cut_x = (float)ab / 4.0f;    // Cortar en 4 partes
    cut_y = (float)ord / 4.0f;
    cut_xb = (float)(ab + 1) / 4.0f;
    cut_yb = (float)(ord + 1) / 4.0f;
    Texture2D dummyTexture = {0};
    return dummyTexture;
}

void meshLoadData(char *file, t_mesh *mesh, Bone *root) {
    int i, j, t;
    char buffer[4096], blist[4096], *tok, *str;
    float x, y;
    FILE *fd = fopen(file, "r");
    if (!fd) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    fgets(buffer, sizeof(buffer), fd);
    sscanf(buffer, "%d %s %d", &(mesh->vertexCount), nameFTx, &contTxt);
    for (i = 0; i < mesh->vertexCount; i++) {
        fgets(buffer, sizeof(buffer), fd);
        sscanf(buffer, "%i %f %f %[^\n]\n", &t, &x, &y, blist);
        mesh->v[i].v.x = x;
        mesh->v[i].v.y = y;
        mesh->v[i].t = t;
        str = blist;
        j = 0;
        while ((tok = strtok(str, " "))) {
            str = NULL;
            mesh->v[i].bone[j] = boneFindByName(root, tok);
            tok = strtok(NULL, " ");
            if (tok) {
                mesh->v[i].weight[j] = atof(tok);
                j++;
            }
        }
        mesh->v[i].boneCount = j;
    }
    fclose(fd);
}
/*
void meshDraw(t_mesh *mesh, Bone *root, int time) {
    int i, j, n;
    Vector2 v[MAX_VXCOUNT * MAX_BONECOUNT];
    Matrix mat;
    float tmp, x, y;

    if (mesh == NULL || root == NULL) {
        return; // Manejo de punteros nulos
    }

    n = mesh->vertexCount;
    if (n <= 0 || n > MAX_VXCOUNT * MAX_BONECOUNT) {
        return; // Manejo de número de vértices fuera de rango
    }

    for (i = 0; i < n; i++) {
        if (mesh->v[i].bone[1] == NULL) {
            continue; // Manejo de punteros nulos
        }
        getBoneMatrix(mesh->v[i].bone[1], &mat);  // Pasa la dirección de mat
        Vector2 translation = { mat.m0, mat.m1 };  // Obtén la traslación de la matriz
        x = translation.x;
        y = translation.y;
        tmp = mesh->v[i].v.y;
        v[i].x = mat.m0 + tmp * mat.m4 + mat.m12;
        v[i].y = mat.m1 + tmp * mat.m5 + mat.m13;
    }

    for (j = 1; j < n-3; j += 2) {
        int textureIndex = mesh->v[j].t;
        if (textureIndex >= 0 && textureIndex < sizeof(textures) / sizeof(textures[0]) && textures[textureIndex].id != 0) {
            Bone *bone = boneFindByName(root, mesh->v[j].bone[1]->name);
            if (bone != NULL) {
                Texture2D partTexture = getPartTexture(bone->frame);
                if (partTexture.id != 0) {
                    Vector2 points[4] = {
                        {v[j].x, v[j].y},
                        {v[j+2].x, v[j+2].y},
                        {v[j+3].x, v[j+3].y},
                        {v[j+1].x, v[j+1].y}
                    };
                    DrawTexturePro(partTexture, (Rectangle){cut_x, cut_y, cut_xb - cut_x, cut_yb - cut_y},
                                   (Rectangle){v[j].x, v[j].y, v[j+2].x - v[j].x, v[j+2].y - v[j].y},
                                   (Vector2){0, 0}, 0.0f, WHITE);
                }
            }
        }
    }
}
*/

void meshDraw(t_mesh *mesh, Bone *root, int time) {
    if (mesh == NULL || root == NULL) return;

    int n = mesh->vertexCount;
    if (n <= 0 || n > MAX_VXCOUNT * MAX_BONECOUNT) return;

    // Seleccionar la primera textura válida que encontremos
    Texture2D texture = { 0 };
    for (int j = 0; j < n; j++) {
        int textureIndex = mesh->v[j].t;
        if (textureIndex >= 0 && textureIndex < (sizeof(textures) / sizeof(textures[0]))) {
            if (textures[textureIndex].id != 0) {
                texture = textures[textureIndex];
                break; // Solo usamos la primera textura válida encontrada
            }
        }
    }

    if (texture.id == 0) return; // No hay textura válida para dibujar

    // Configura la posición y tamaño del rectángulo para dibujar la textura
    Vector2 position = { GetScreenWidth() / 2 - texture.width / 2, GetScreenHeight() / 2 - texture.height / 2 };
    Rectangle sourceRect = { 0, 0, texture.width, texture.height };
    Rectangle destRect = { position.x, position.y, texture.width, texture.height };

    DrawTexturePro(texture, sourceRect, destRect, (Vector2){0, 0}, 0.0f, WHITE);
}
