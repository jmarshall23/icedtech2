// fx_gibs.c
//

#include "cg_local.h"

#define MAX_GIB_MODELS		3
qhandle_t		gibModels[MAX_GIB_MODELS];

/*
=================
FX_InitGibs
=================
*/
void FX_InitGibs(void) {
	for (int i = 0; i < MAX_GIB_MODELS; i++) {
		gibModels[i] = engine->renderer->RegisterModel(va("models/gibs/gib%d.md3", i));
	}
}

/*
=================
FX_GibTick
=================
*/
void FX_GibTick(localEntity_t* le) {
	vec3_t dir;

	VectorSubtract(le->refEntity.origin, le->refEntity.oldorigin, dir);

	VectorNormalize(dir);

	if (VectorLength(dir) == 0)
		return;

	VectorSet(dir, 0, 0, 0); // We don't want the particles to actually have a velocity, just spawn and fall. 

	FX_AddBlood(le->refEntity.origin, dir, 700, 1);
}

/*
=================
FX_FastGibTick
=================
*/
void FX_FastGibTick(localEntity_t* le) {
	vec3_t dir;

	le->pos.trType = TR_GRAVITY;

	VectorSubtract(le->refEntity.origin, le->refEntity.oldorigin, dir);

	VectorNormalize(dir);

	if (VectorLength(dir) == 0)
		return;

	VectorSet(dir, 0, 0, 0); // We don't want the particles to actually have a velocity, just spawn and fall. 

	FX_AddBlood(le->refEntity.origin, dir, 700, 1);
}

/*
=================
FX_SpawnGibs
=================
*/
void FX_SpawnGibs(vec3_t origin2) {
	const int count = 4;

	for (int i = 0; i < count; i++) {
		vec3_t dir;
		vec3_t origin;

		origin[0] = origin2[0] + ((rand() % 32) - 16);
		origin[1] = origin2[1] + ((rand() % 32) - 16);
		origin[2] = origin2[2] + ((rand() % 32) - 16);

		dir[0] = 0;
		dir[1] = 0;
		dir[2] = 1;
		VectorNormalize(dir);

		FX_AddDebris(origin, dir, 380, 3800, 1, gibModels[rand() % MAX_GIB_MODELS], 1.0f, FX_GibTick);
	}
}

void FX_SpawnSingleGib(vec3_t origin2, float size, int duration) {
	vec3_t dir;
	vec3_t origin;

	origin[0] = origin2[0] + ((rand() % 32) - 16);
	origin[1] = origin2[1] + ((rand() % 32) - 16);
	origin[2] = origin2[2] + ((rand() % 32) - 16);

	dir[0] = 0;
	dir[1] = 0;
	dir[2] = 1;
	VectorNormalize(dir);

	FX_AddDebris(origin, dir, 380, duration, 1, gibModels[rand() % MAX_GIB_MODELS], size, FX_FastGibTick);
}