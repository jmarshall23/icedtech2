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
		gibModels[i] = trap_R_RegisterModel(va("models/gibs/gib%d.md3", i));
	}
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

		FX_AddDebris(origin, dir, 380, 3800, 1, gibModels[rand() % MAX_GIB_MODELS], 1.0f);
	}
}