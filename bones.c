#include <stddef.h> 
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include "raylib.h"
#include "bones.h"

char		*currentName = NULL;
BonesXY		bonesdata[MAX_BONECOUNT];
int			coordenada;
int			contTxt;
float		cut_x,cut_y,cut_xb,cut_yb;
char		nameFTx[99];
uint32_t	maxTime = 0;
Texture2D	*textures = NULL;
int			maxTextureIndex = -1;

Bone*	boneFreeTree(Bone *root)
{
	if (!root)
		return (NULL);
	for (int i = 0; i < root->childCount; i++)
		boneFreeTree(root->child[i]);
	free(root);
	return (NULL);
}

void	boneDumpAnim(Bone *root, uint8_t level)
{
	if (!root) 
		return;
	printf("%s ", root->name);
	for (int f = 0; f < root->keyframeCount; f++)
		printf("%i %d %i %i %3.1f %3.1f ", root->keyframe[f].time, 
				root->keyframe[f].partex, root->keyframe[f].layer, 
				root->keyframe[f].coll, root->keyframe[f].angle, 
				root->keyframe[f].length);
	printf("\n");
	for (int i = 0; i < root->childCount; i++)
		boneDumpAnim(root->child[i], level + 1);
}

Bone*	boneFindByName(Bone *root, char *name)
{
	int		i;
	Bone	*p;

	if (!root)
		return (NULL);
	if (!strcmp(root->name, name))
		return (root);
	for (i = 0; i < root->childCount; i++)
	{
		p = boneFindByName(root->child[i], name);
		if (p)
			return (p);
	}
	return (NULL);
}

int	boneInterAnimation(Bone *root, Bone *introot, int time, float intindex)
{
	int keyframeUpdated = 0;

	if (!root)
		return (0);
	for (int i = 0; i < root->keyframeCount; i++)
	{
		if (root->keyframe[i].time == time)
		{
			if (i < root->keyframeCount - 1)
			{
				float tim = root->keyframe[i + 1].time - root->keyframe[i].time;
				root->depth = root->keyframe[i].layer;
				root->effect = root->keyframe[i].coll;
				root->frame = root->keyframe[i].partex;
				root->offA = ((root->keyframe[i + 1].angle - root->keyframe[i].angle) +
						((introot->keyframe[i + 1].angle - introot->keyframe[i].angle) -
						 (root->keyframe[i + 1].angle - root->keyframe[i].angle)) * intindex) / tim;
				root->offL = ((root->keyframe[i + 1].length - root->keyframe[i].length) +
						((introot->keyframe[i + 1].length - introot->keyframe[i].length) -
						 (root->keyframe[i + 1].length - root->keyframe[i].length)) * intindex) / tim;
			}
			else
				root->offA = root->offL = 0;
			keyframeUpdated = 1;
			break;
		}
		else if (root->keyframe[i].time > time)
			break;
	}
	if (keyframeUpdated)
	{
		root->a += root->offA;
		root->l += root->offL;
	}
	int others = 0;
	for (int i = 0; i < root->childCount; i++)
	{
		if (boneInterAnimation(root->child[i], introot->child[i], time, intindex))
			others = 1;
	}
	return (keyframeUpdated || others);
}

Bone *boneCleanAnimation(Bone *root, char *path)
{
	if (!root)
		return (NULL);
	root->keyframeCount = 0;
	for (int i = 0; i < root->childCount; i++)
	{
		boneCleanAnimation(root->child[i],path);
	}
	return (root);
}

int boneAnimate(Bone *root, int time)
{
	int kfUpd = 0;

	if (!root)
		return (0);
	for (int kfIdx = 0; kfIdx < root->keyframeCount; kfIdx++)
	{
		if (root->keyframe[kfIdx].time < time && time <= root->keyframe[kfIdx + 1].time)
		{
			if (kfIdx < root->keyframeCount - 1)
			{
				float tim = root->keyframe[kfIdx + 1].time - root->keyframe[kfIdx].time;
				root->depth = root->keyframe[kfIdx].layer;
				root->effect = root->keyframe[kfIdx].coll;
				root->frame = root->keyframe[kfIdx].partex;
				root->offA = (root->keyframe[kfIdx + 1].angle - root->keyframe[kfIdx].angle) / tim;
				root->offL = (root->keyframe[kfIdx + 1].length - root->keyframe[kfIdx].length) / tim;
			}
			else
				root->offA = root->offL = 0;
			kfUpd = 1;
			break;
		}
		else if (root->keyframe[kfIdx].time > time)
			break;
	}
	if (kfUpd)
	{
		root->a += root->offA;
		root->l += root->offL;
	}
	int others = 0;
	for (int i = 0; i < root->childCount; i++)
	{
		if (boneAnimate(root->child[i], time))
			others = 1;
	}
	return (kfUpd || others);
}


int boneAnimateReverse(Bone *root, int time)
{
	int kfUpd = 0;

	if (!root)
		return 0;

	for (int kfIdx = root->keyframeCount - 1; kfIdx >= 0; kfIdx--)
	{
		if (root->keyframe[kfIdx].time > time && time >= root->keyframe[kfIdx - 1].time)
		{
			if (kfIdx > 0)
			{
				float tim = root->keyframe[kfIdx].time - root->keyframe[kfIdx - 1].time;
				root->depth = root->keyframe[kfIdx].layer;
				root->effect = root->keyframe[kfIdx].coll;
				root->frame = root->keyframe[kfIdx].partex;
				root->offA = (root->keyframe[kfIdx].angle - root->keyframe[kfIdx - 1].angle) / tim;
				root->offL = (root->keyframe[kfIdx].length - root->keyframe[kfIdx - 1].length) / tim;
			}
			else
				root->offA = root->offL = 0;
			kfUpd = 1;
			break ;
		}
		else if (root->keyframe[kfIdx].time < time)
			break ;
	}
	if (kfUpd)
	{
		root->a -= root->offA;
		root->l -= root->offL;
	}
	int others = 0;
	for (int i = 0; i < root->childCount; i++)
	{
		if (boneAnimateReverse(root->child[i], time))
			others = 1;
	}
	return (kfUpd || others);
}
  

void boneListNames(Bone *root, char names[MAX_BONECOUNT][99])
{
	int i, present;
	if (!root)
		return;
	present = 0;
	for (i = 0; (i < MAX_BONECOUNT) && (names[i][0] != '\0'); i++)
		if (!strcmp(names[i], root->name))
		{
			present = 1;
			break;
		}
	if (!present && (i < MAX_BONECOUNT))
	{
		strcpy(names[i], root->name);
		if (i + 1 < MAX_BONECOUNT)
			names[i + 1][0] = '\0';
	}
	for (i = 0; i < root->childCount; i++)
		boneListNames(root->child[i], names);
}

Bone *boneChangeAnimation(Bone *root, char *path)
{
	if (!root)
	{
		fprintf(stderr, "Root bone is NULL\n");
		return (NULL);
	}
	animationLoadKeyframes(path, root);
	return (root);
}

Bone* boneLoadStructure(const char *path)
{
	Bone *root = NULL, *temp = NULL;
	FILE *file;
	float x, y, angle, length;
	int depth, actualLevel = 0, flags;
	char name[99], depthStr[99], buffer[4096];

	if (!(file = fopen(path, "r")))
	{
		fprintf(stderr, "Can't open file %s for reading\n", path);
		return (NULL);
	}
	while (fgets(buffer, sizeof(buffer), file))
	{
		if (strlen(buffer) < 3)
			continue;
		sscanf(buffer, "%s %f %f %f %f %d %s", depthStr, &x, &y, &angle, &length, &flags, name);
		depth = strlen(depthStr) - 1;
		if (depth < 0 || depth > MAX_CHCOUNT)
		{
			fprintf(stderr, "Wrong bone depth (%s)\n", depthStr);
			fclose(file);
			return (NULL);
		}
		for (; actualLevel > depth; actualLevel--)
			temp = temp->parent;
		if (!root && depth == 0)
		{
			root = boneAddChild(NULL, x, y, angle, length, flags, name);
			temp = root;
		}
		else
			temp = boneAddChild(temp, x, y, angle, length, flags, name);
		actualLevel++;
	}
	fclose(file);
	return (root);
}

Bone *boneAddChild(Bone *root, float x, float y, float a, float l, uint8_t flags, char *name)
{
	Bone *t;
	int i;
	if (!root)
	{
		if (!(root = (Bone *)malloc(sizeof(Bone))))
			return NULL;
		root->parent = NULL;
		root->childCount = 0; 
	}
	else if (root->childCount < MAX_CHCOUNT)
	{
		if (!(t = (Bone *)malloc(sizeof(Bone))))
			return NULL;
		t->parent = root;
		root->child[root->childCount++] = t;
		root = t; 
	}
	else
		return (NULL);
	root->x = x;
	root->y = y;
	root->a = a;
	root->l = l;
	root->flags = flags;
	root->childCount = 0; 
	if (name)
	{
		strncpy(root->name, name, sizeof(root->name) - 1);
		root->name[sizeof(root->name) - 1] = '\0';
	}
	else
		strcpy(root->name, "Bone");
	for (i = 0; i < MAX_CHCOUNT; i++)
		root->child[i] = NULL;
	return (root);
}

void DrawBones(Bone *root, bool drawBonesEnabled)
{
    if (root == NULL)
        return;

    Vector2 startPos = { root->x, root->y };
    Vector2 endPos = {
        startPos.x + root->l * cos(root->a),
        startPos.y + root->l * sin(root->a)
    };

    if (drawBonesEnabled)
    {
        DrawLineEx(startPos, endPos, 2.0f, GREEN);
        DrawCircleV(startPos, 5, BLUE);
    }

    for (int i = 0; i < root->childCount; i++)
    {
        if (root->child[i] != NULL)
        {
            root->child[i]->x = endPos.x;
            root->child[i]->y = endPos.y;
            DrawBones(root->child[i], drawBonesEnabled); 
        }
    }
}

void UnloadTextures(void)
{
    if (textures)
    {
        for (int i = 0; i <= maxTextureIndex; i++)
        {
            if (textures[i].id > 0)
                UnloadTexture(textures[i]);
        }
        free(textures);
        textures = NULL;
        maxTextureIndex = -1;
    }
}

void LoadTextures(t_mesh *mesh) {
    char path[512];
    char resultPath[512];

    maxTextureIndex = -1;
    for (int i = 0; i < mesh->vertexCount; i++) {
        if (mesh->v[i].t > maxTextureIndex) {
            maxTextureIndex = mesh->v[i].t;
        }
    }
    if (maxTextureIndex == -1) {
        printf("Error: No hay índices de textura válidos\n");
        return;
    }
    textures = (Texture2D *)calloc(maxTextureIndex + 1, sizeof(Texture2D));
    
    snprintf(resultPath, sizeof(resultPath), "%s/Textures/%s", nameFTx, nameFTx);
    for (int t = 0; t <= maxTextureIndex; t++) {
        int len = snprintf(path, sizeof(path), "%s_%d.png", resultPath, t);
        if (len < 0 || len >= sizeof(path)) {
            printf("Ruta inválida para t=%d\n", t);
            continue;
        }
        if (FileExists(path)) {
            Image image = LoadImage(path);
            textures[t] = LoadTextureFromImage(image);
            UnloadImage(image);
            //printf("Cargada textura t=%d\n", t);
        }
    }
}


Matrix GetBoneMatrix(Bone *bone)
{
	Matrix mat = MatrixIdentity();
	mat = MatrixMultiply(mat, MatrixRotateZ(bone->a));
	mat = MatrixMultiply(mat, MatrixTranslate(bone->x, bone->y, 0.0f));
	return mat;
}

void getPartTexture(int tex, int contTxt)
{
    int ab = tex % contTxt;
    int ord = tex / contTxt;
    cut_x = (float)ab / (float)contTxt;
    cut_y = (float)ord / (float)contTxt;
    cut_xb = (float)(ab + 1) / (float)contTxt;
    cut_yb = (float)(ord + 1) / (float)contTxt;
}

float getBoneAngle(Bone* b)
{
	if (!b || !b->parent)
		return (0.0f);

	Bone* b2 = b->parent;
	float dx = b2->x - b->x;
	float dy = b2->y - b->y;
	float angle = atan2(dy, dx) * 180.0f / M_PI;

	if (angle < 0.0f)
		angle += 360.0f;

	return (angle - 90.0f);
}

Vector2 applyBoneMove(Bone *bone, Vector2 vertex)
{
	float totalAngle = getBoneAngle(bone);
	float radAngle = totalAngle * M_PI / 180.0f;

	Vector2 transformed;
	transformed.x = bone->x + vertex.x * cos(radAngle) - vertex.y * sin(radAngle);
	transformed.y = bone->y + vertex.x * sin(radAngle) + vertex.y * cos(radAngle);
	return transformed;
}

int compareVerticesByLayer(const void *a, const void *b) {
    const BoneVertex *vertA = (const BoneVertex *)a;
    const BoneVertex *vertB = (const BoneVertex *)b;

    if (!vertA->bone[0] || !vertB->bone[0]) {
        return 0;
    }

    int layerA = vertA->bone[0]->depth;
    int layerB = vertB->bone[0]->depth;

    if (layerA < layerB) return -1;
    if (layerA > layerB) return 1;
    return 0;
}

void DrawTexturePoly(Texture2D texture, Vector2 center, Vector2 *points, Vector2 *texcoords, int pointCount, Color tint)
{
	rlSetTexture(texture.id);

	rlBegin(RL_QUADS);

	rlColor4ub(tint.r, tint.g, tint.b, tint.a);

	for (int i = 0; i < pointCount - 1; i++)
	{
		rlTexCoord2f(0.5f, 0.5f);
		rlVertex2f(center.x, center.y);

		rlTexCoord2f(texcoords[i].x, texcoords[i].y);
		rlVertex2f(points[i].x + center.x, points[i].y + center.y);

		rlTexCoord2f(texcoords[i + 1].x, texcoords[i + 1].y);
		rlVertex2f(points[i + 1].x + center.x, points[i + 1].y + center.y);

		rlTexCoord2f(texcoords[i + 1].x, texcoords[i + 1].y);
		rlVertex2f(points[i + 1].x + center.x, points[i + 1].y + center.y);
	}
	rlEnd();
	rlSetTexture(0);
}

void meshDraw(t_mesh *mesh, Bone *root, int time)
{
	qsort(mesh->v, mesh->vertexCount, sizeof(BoneVertex), compareVerticesByLayer);
	for (int i = 0; i < mesh->vertexCount; i++)
	{
		BoneVertex *boneVertex = &mesh->v[i];
		Vector2 trfrmedPos = {0.0f, 0.0f};
		float totalAngle = 0.0f;
		float scaleFactor = 1.0f;
		Texture2D texture;
		int	partex[2];
		int coll[2];
		for (int j = 0; j < boneVertex->boneCount; j++)
		{
			Bone *bone = boneVertex->bone[j];
			if (bone) {
				Vector2 boneTrfrmedPos = applyBoneMove(bone, (Vector2){boneVertex->v.x, boneVertex->v.y});
				trfrmedPos.x = boneTrfrmedPos.x;
				trfrmedPos.y = boneTrfrmedPos.y;
				totalAngle = getBoneAngle(bone);
				scaleFactor = boneVertex->weight[j];

				partex[j] = bone->frame;
				coll[j] = bone->effect;
			}
		}
		texture = textures[boneVertex->t];
		//printf("testura %d\n", boneVertex->t );
		getPartTexture(partex[0],contTxt);

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

		switch (coll[0])
		{
			case 0:
				break;
			case 1:
				sourceRect.x = cut_x * texture.width;
				sourceRect.width *= -1;
				break;
			case 2:
				sourceRect.y = cut_y * texture.height;
				sourceRect.height *= -1;
				break;
			case 3:
				sourceRect.y = cut_y * texture.height;
				sourceRect.height *= -1;
				sourceRect.x = cut_x * texture.width;
				sourceRect.width *= -1;
				break;
			case 4:
				sourceRect.width = (cut_xb - cut_x) * texture.width * (1.0f + 0.02f *
						sin(GetTime() * 2.0f * PI));
				sourceRect.height = (cut_yb - cut_y) * texture.height * (1.0f + 0.02f *
						sin(GetTime() * 2.0f * PI));
				sourceRect.x = cut_x * texture.width - (sourceRect.width - (cut_xb - cut_x) * 
						texture.width) / 2.0f;
				sourceRect.y = cut_y * texture.height - (sourceRect.height - (cut_yb - cut_y) * 
						texture.height) / 2.0f;
				break;
			case 5:
				float windStrength = sin(GetTime() * 0.5f);
				float offset = windStrength * 0.2f;
				sourceRect.width = (cut_xb - cut_x) * texture.width * (1.0f + 0.05f *
						sin(GetTime() * 2.0f * PI));
				sourceRect.height = (cut_yb - cut_y) * texture.height * (1.0f + 0.05f * 
						sin(GetTime() * 2.0f * PI));
				sourceRect.x = cut_x * texture.width;
				sourceRect.y = cut_y * texture.height - (sourceRect.height - (cut_yb - cut_y) *
						texture.height);
				sourceRect.x += offset;
				destRect.x = trfrmedPos.x;
				destRect.y = trfrmedPos.y;
				destRect.width = 100.0f * scaleFactor;
				destRect.height = 100.0f * scaleFactor;
				break;
			case 6:
				float baseWidth = (cut_xb - cut_x) * texture.width;
				float baseHeight = (cut_yb - cut_y) * texture.height;

				float wave = 0.007f * sin(GetTime() * 0.8f * PI);
				float waveOffset = baseHeight * wave;

				// RECTÁNGULO DE TEXTURA: fijo
				sourceRect.x = cut_x * texture.width;
				sourceRect.y = cut_y * texture.height;
				sourceRect.width = baseWidth;
				sourceRect.height = baseHeight;

				// RECTÁNGULO DE DESTINO: se estira hacia arriba
				destRect.width = 100.0f * scaleFactor;
				destRect.height = 100.0f * scaleFactor + waveOffset * scaleFactor;
				destRect.x = trfrmedPos.x;
				destRect.y = trfrmedPos.y - waveOffset * scaleFactor; 
				// subirlo para mantener la base fija
				break;
			case 7:
				baseWidth = (cut_xb - cut_x) * texture.width;
				baseHeight = (cut_yb - cut_y) * texture.height;
				wave = 0.01f * sin(GetTime() * 2.0f * PI); // Efecto de onda
				waveOffset = baseHeight * wave; // Desplazamiento vertical
				// RECTÁNGULO DE TEXTURA: Fijo
				sourceRect.x = cut_x * texture.width; // Punto 1 se mueve
				sourceRect.y = cut_y * texture.height; // Punto 3 se mueve
				sourceRect.width = baseWidth;
				sourceRect.height = baseHeight;
				// RECTÁNGULO DE DESTINO: Fijamos el punto 4 (inferior derecho)
				destRect.width = 100.0f * scaleFactor;
				destRect.height = 100.0f * scaleFactor; // Punto 4 no cambia
				destRect.x = trfrmedPos.x; // Punto 1 se mueve
				destRect.y = trfrmedPos.y - waveOffset * scaleFactor; // Punto 3 se mueve
				// Ajustamos la posición para que el punto 4 (inferior derecho) permanezca fijo
				destRect.x -= waveOffset * scaleFactor;
				// Desplazamos el punto 2 hacia la izquierda
				break;
			default:
				break;
		}
		DrawTexturePro(texture, sourceRect, destRect, origin, totalAngle, WHITE);
	}
}

void animationLoadKeyframes(const char *path, Bone *root)
{
	FILE		*file;
	float		angle, length;
	int			layer, partex, coll;
	uint32_t	time;
	char		name[100], buffer[4096], *ptr, *token, *rest;
	Bone		*bone;

	if (!(file = fopen(path, "r")))
	{
		fprintf(stderr, "Can't open file %s for reading\n", path);
		return;
	}
	while (fgets(buffer, sizeof(buffer), file))
	{
		if (strlen(buffer) < 3)
			continue;
		sscanf(buffer, "%s", name);
		bone = boneFindByName(root, name);
		if (!bone)
		{
			fprintf(stderr, "Bone %s not found\n", name);
			continue;
		}
		ptr = buffer + strlen(name) + 1;
		while ((token = strtok_r(ptr, " ", &rest)))
		{
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
			if (bone->keyframeCount >= MAX_KFCOUNT)
			{
				bone->keyframeCount = 0;
				//printf("%d",bone->keyframeCount );
				//fprintf(stderr, "Warning: Keyframe count exceeded for bone %s\n", name);
				//continue;
			}
			Keyframe *k = &(bone->keyframe[bone->keyframeCount]);
			k->time = time;
			k->partex = partex;
			k->layer = layer;
			k->coll = coll;
			k->angle = angle;
			k->length = length;
			bone->keyframeCount++;
			if (time > maxTime)
				maxTime = time;
		}
	}
	fclose(file);
}

void meshLoadData(char *file, t_mesh *mesh, Bone *root)
{
	FILE *fd = fopen(file, "r");
	if (!fd) return;
	char buffer[4096], blist[4096];
	int i, t, j;
	float x, y;
	if (fgets(buffer, sizeof(buffer), fd) &&
			sscanf(buffer, "%49s %d %d", nameFTx, &(mesh->vertexCount), &contTxt) == 3)
	{
		for (i = 0; i < mesh->vertexCount; i++)
		{
			if (fgets(buffer, sizeof(buffer), fd) &&
					sscanf(buffer, "%d %f %f %[^\n]", &t, &x, &y, blist) == 4)
			{
				mesh->v[i].v.x = x;
				mesh->v[i].v.y = y;
				mesh->v[i].t = t;
				char *str = blist;
				j = 0;
				char *tok = strtok(str, " ");
				while (tok && j < 10)
				{
					mesh->v[i].bone[j] = boneFindByName(root, tok);
					if (!mesh->v[i].bone[j]) {
						fprintf(stderr, "ERROR: Hueso '%s' no encontrado para el vértice %d\n", tok, i);
						break;
					}
					if (mesh->v[i].bone[j])
					{
						tok = strtok(NULL, " ");
						mesh->v[i].weight[j] = (tok) ? atof(tok) : 0.0f;
						j++;
					}
					tok = strtok(NULL, " ");
				}
				mesh->v[i].boneCount = j;
			}
			else
			{
				fclose(fd);
				return ;
			}
		}
	}
	fclose(fd);
}
