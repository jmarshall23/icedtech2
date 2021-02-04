// fx_debris.c
//

#include "cg_local.h"

cgDebrisModel_t _debBlock;
cgDebrisModel_t _debRock;


/*
=====================
CG_LoadDebrisMesh

For the debris we need models for each and every texture in the world, 
luckily these assets are low poly the memory overhead is small(max 2mb). 
=====================
*/
void CG_LoadDebrisMesh(const char* name, const char *filename, cgDebrisModel_t* model) {
	int numWorldShaders = engine->renderer->GetNumWorldShaders();

	memset(model, 0, sizeof(cgDebrisModel_t));

	if (numWorldShaders >= 1024) {
		CG_Error("Too many shaders for debris!");
	}

	for (int i = 0; i < numWorldShaders; i++) {
		qhandle_t worldShader = engine->renderer->GetWorldShader(i);
		model->models[i] = engine->renderer->RE_RegisterModelEx(va("_debris%s_%d", name, i), filename, worldShader);
	}
}

/*
==================
FX_InitDebris
==================
*/
void FX_InitDebris(void) {
	CG_LoadDebrisMesh("debbrick1", "models/debris/brick1.md3", &_debBlock);
	//CG_LoadDebrisMesh("debbrick2", "models/debris/brick2.md3", &_debBlock[1]);
	//CG_LoadDebrisMesh("debbrick3", "models/debris/brick3.md3", &_debBlock[2]);
	//CG_LoadDebrisMesh("debbrick4", "models/debris/brick4.md3", &_debBlock[3]);
	//CG_LoadDebrisMesh("debbrick5", "models/debris/brick5.md3", &_debBlock[4]);
	//CG_LoadDebrisMesh("debbrick6", "models/debris/brick6.md3", &_debBlock[5]);


	CG_LoadDebrisMesh("debrock1", "models/debris/rubble1.md3", &_debRock);
	//CG_LoadDebrisMesh("debrock2", "models/debris/rubble2.md3", &_debRock[1]);
	//CG_LoadDebrisMesh("debrock3", "models/debris/rubble3.md3", &_debRock[2]);	
}

/*
================
CG_AddDebrisElements
================
*/
void CG_AddDebrisElements(localEntity_t* le) {
	vec3_t newOrigin;
	trace_t trace;
	float lifeFrac;
	int t, step = 50;

	VectorCopy(le->refEntity.origin, le->refEntity.oldorigin);

	for (t = le->lastTrailTime + step; t < cg.time; t += step) {
		// calculate new position
		BG_EvaluateTrajectory(&le->pos, t, newOrigin);

		// trace a line from previous position to new position
		CG_Trace(&trace, le->refEntity.origin, NULL, NULL, newOrigin, -1, MASK_SHOT);

		// if stuck, kill it
		if (trace.startsolid) {
			// HACK, some walls screw up, so just pass through if starting in a solid
			VectorCopy(newOrigin, trace.endpos);
			trace.fraction = 1.0;
		}

		// moved some distance
		VectorCopy(trace.endpos, le->refEntity.origin);

		// add a trail
		lifeFrac = (float)(t - le->startTime) / (float)(le->endTime - le->startTime);

		// smoke
		//if (le->effectFlags & 1) {
		//	le->headJuncIndex2 = CG_AddSmokeJunc(le->headJuncIndex2,
		//		le, // rain - zinx's trail fix
		//		cgs.media.smokeTrailShader,
		//		le->refEntity.origin,
		//		(int)(2000.0 * (0.5 + 0.5 * (1.0 - lifeFrac))), // trail life
		//		1.0 * (trace.fraction == 1.0) * (0.5 + 0.5 * (1.0 - lifeFrac)), // alpha
		//		1, // start width
		//		(int)(60.0 * (0.5 + 0.5 * (1.0 - lifeFrac)))); // end width
		//}

		// if it is in a nodrop zone, remove it
		// this keeps gibs from waiting at the bottom of pits of death
		// and floating levels
//		if ( CG_PointContents( trace.endpos, 0 ) & CONTENTS_NODROP ) {
//			CG_FreeLocalEntity( le );
//			return;
//		}

		if (trace.fraction < 1.0) {
			// reflect the velocity on the trace plane
			CG_ReflectVelocity(le, &trace);
			if (VectorLengthSquared(le->pos.trDelta) < SQR(1)) {
				CG_FreeLocalEntity(le);
				return;
			}
			// the intersection is a fraction of the frametime
			le->pos.trTime = t;
		}

		le->lastTrailTime = t;
		}

	if (le->FX_RenderCallback) {
		le->FX_RenderCallback(le);
	}

	engine->renderer->AddRefEntityToScene(&le->refEntity);

}


/*
=================
FX_AddDebris
=================
*/
void FX_AddDebris(vec3_t origin, vec3_t dir, int speed, int duration, int count, qhandle_t mesh, float size, void	(*FX_RenderCallback)(localEntity_t* le)) {
	localEntity_t* le;
	refEntity_t* re;


	vec3_t velocity, unitvel;
	float timeAdd;
	int i;

	for (i = 0; i < count; i++) {
		le = CG_AllocLocalEntity();
		re = &le->refEntity;

		re->reType = RT_MODEL;
		re->hModel = mesh;
		re->scale = size;

		VectorSet(unitvel, dir[0] + crandom() * 0.9, dir[1] + crandom() * 0.9, fabs(dir[2]) > 0.5 ? dir[2] * (0.2 + 0.8 * random()) : random() * 0.6);
		VectorScale(unitvel, (float)speed + (float)speed * 0.5 * crandom(), velocity);

		le->leType = LE_DEBRIS;
		le->startTime = cg.time;
		le->endTime = le->startTime + duration + (int)((float)duration * 0.8 * crandom());
		le->lastTrailTime = cg.time;

		le->FX_RenderCallback = (void(__cdecl*)(void*))FX_RenderCallback;

		VectorCopy(origin, re->origin);
		AxisCopy(axisDefault, re->axis);

		le->pos.trType = TR_GRAVITY_LOW;
		VectorCopy(origin, le->pos.trBase);
		VectorCopy(velocity, le->pos.trDelta);
		le->pos.trTime = cg.time;

		timeAdd = 10.0 + random() * 40.0;
		BG_EvaluateTrajectory(&le->pos, cg.time + (int)timeAdd, le->pos.trBase);

		le->bounceFactor = 0.5;
	}
}

void FX_SpawnWallDebris(vec3_t origin, vec3_t dir, qhandle_t shader) {
	//trace_t trace;
	//vec3_t newOrigin;
	vec3_t dir2;

	//newOrigin[0] = origin[0] + (dir[0] * 100);
	//newOrigin[1] = origin[1] + (dir[1] * 100);
	//newOrigin[2] = origin[2] + (dir[2] * 100);
	//
	//CG_Trace(&trace, origin, NULL, NULL, newOrigin, -1, MASK_SHOT);
	//
	//if (trace.fraction == 1.0f) {
	//	CG_Printf("WARNING: Debris not able to find wall\n");
	//	return;
	//}

	dir2[0] = -dir[0];
	dir2[1] = -dir[1];
	dir2[2] = -dir[2];

	FX_AddDebris(origin, dir2, irand(250, 320), 2800, irand(8, 12), _debBlock.models[cg_shaderLookup[shader]], flrand(0.5, 1.0f), NULL);
	FX_AddDebris(origin, dir2, irand(250, 320), 2800, irand(8, 12), _debRock.models[cg_shaderLookup[shader]], flrand(0.5, 1.0f), NULL);
}

void FX_SpawnWallDebrisBig(vec3_t origin, vec3_t dir, qhandle_t shader) {
	//trace_t trace;
//vec3_t newOrigin;
	vec3_t dir2;

	//newOrigin[0] = origin[0] + (dir[0] * 100);
	//newOrigin[1] = origin[1] + (dir[1] * 100);
	//newOrigin[2] = origin[2] + (dir[2] * 100);
	//
	//CG_Trace(&trace, origin, NULL, NULL, newOrigin, -1, MASK_SHOT);
	//
	//if (trace.fraction == 1.0f) {
	//	CG_Printf("WARNING: Debris not able to find wall\n");
	//	return;
	//}

	dir2[0] = -dir[0];
	dir2[1] = -dir[1];
	dir2[2] = -dir[2];

	FX_AddDebris(origin, dir2, 280, 2800, 13 + rand() % 4, _debBlock.models[cg_shaderLookup[shader]], 3.0f, NULL);
	FX_AddDebris(origin, dir2, 280, 2800, 13 + rand() % 4, _debRock.models[cg_shaderLookup[shader]], 3.0f, NULL);
}