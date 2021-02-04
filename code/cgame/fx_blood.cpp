// fx_blood.c
//

#include "cg_local.h"

/*
================
FX_RenderBlood
================
*/
void FX_RenderAddBloodElements(localEntity_t* le) {
	refEntity_t* ent;

	ent = &le->refEntity;

	int dist = le->endTime - le->startTime;

	AnglesToAxis(cg.refdefViewAngles, ent->axis);

	ent->hModel = cgs.media.bloodPuffModel;
	ent->scale = flrand(3.0f, 7.0f);
	ent->skipShadows = qtrue;

	vec3_t newOrigin;
	trace_t trace;
	float lifeFrac;
	int t, step = 50;

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

		if (trace.entityNum == ENTITYNUM_WORLD) {
			CG_FreeLocalEntity(le);
			return;
		}

		if (trace.fraction < 1.0) {
			// reflect the velocity on the trace plane
			CG_ReflectVelocity(le, &trace);
			//if (VectorLengthSquared(le->pos.trDelta) < SQR(1)) {
			//	CG_FreeLocalEntity(le);
			//	return;
			//}
			// the intersection is a fraction of the frametime
			le->pos.trTime = t;
		}

		le->lastTrailTime = t;
	}

	// add the entity
	engine->renderer->AddRefEntityToScene(ent);

	// add the dlight
	//if (ex->light) {
	//	float		light;
	//
	//	light = (float)(cg.time - ex->startTime) / (ex->endTime - ex->startTime);
	//	if (light < 0.5) {
	//		light = 1.0;
	//	}
	//	else {
	//		light = 1.0 - (light - 0.5) * 2;
	//	}
	//	light = ex->light * light;
	//	engine->renderer->AddLightToScene(ent->origin, light, ex->lightColor[0], ex->lightColor[1], ex->lightColor[2], LDAT_QUADRAT);
	//}
}

/*
===================
FX_AddBlood
===================
*/
void FX_AddBlood(vec3_t origin2, vec3_t dir, int duration, int count) {
	localEntity_t* le;
	refEntity_t* re;
	vec3_t velocity;
	float timeAdd;
	int i;

	int speed = 220;
	int range = count;

	VectorNormalize(dir);

	for (i = 0; i < count; i++) {
		vec3_t origin;

		origin[0] = origin2[0] + flrand(-range, range);
		origin[1] = origin2[1] + flrand(-range, range);
		origin[2] = origin2[2] + flrand(-range, range);

		//VectorSet(dir, flrand(-speed, speed), flrand(-speed, speed), flrand(speed * 4.0, speed * 8.0));

		//VectorNormalize(dir);

		le = CG_AllocLocalEntity();
		re = &le->refEntity;

		re->reType = RT_MODEL;
		//re->hModel = mesh;

		//VectorSet(unitvel, dir[0] + crandom() * 0.9, dir[1] + crandom() * 0.9, fabs(dir[2]) > 0.5 ? dir[2] * (0.2 + 0.8 * random()) : random() * 0.6);
		//VectorScale(dir, flrand(-speed, speed), velocity);

		velocity[0] = dir[0] * flrand(-speed, speed);
		velocity[1] = dir[1] * flrand(-speed, speed);
		velocity[2] = dir[2] * flrand(1.5f * -speed, 2.0f * speed);

		le->leType = LE_BLOOD;
		le->startTime = cg.time;
		le->endTime = le->startTime + duration + (int)((float)duration * 0.8 * crandom());
		le->lastTrailTime = cg.time;

		VectorCopy(origin, re->origin);
		AxisCopy(axisDefault, re->axis);

		le->pos.trType = TR_GRAVITY;
		VectorCopy(origin, le->pos.trBase);
		VectorCopy(velocity, le->pos.trDelta);
		le->pos.trTime = cg.time;

		timeAdd = 10.0 + random() * 40.0;
		BG_EvaluateTrajectory(&le->pos, cg.time + (int)timeAdd, le->pos.trBase);

		le->bounceFactor = 0.5;
	}
}
