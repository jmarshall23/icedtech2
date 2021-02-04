// cg_monster.c
//

#include "cg_local.h"

/*
==================
CG_Monster
==================
*/
void CG_Monster(centity_t* cent) {
	refEntity_t			ent;
	entityState_t* s1;

	s1 = &cent->currentState;

	// if set to invisible, skip
	if (!s1->modelindex) {
		return;
	}

	memset(&ent, 0, sizeof(ent));

	ent.hModel = cgs.gameModels[s1->modelindex];

	int otime = cg.time - cent->dl_time;
	if (otime == 0)
	{
		otime = 60;
	}
	float offset;

	// Check to see if our animation event changed. 
	if (cent->animEvent != s1->eventParm || otime > 2 * LS_FRAMETIME) {
		otime = 0;
		cent->dl_framef = cent->dl_frame = cent->dl_oldframe = s1->startframe;
		cent->dl_backlerp = 0;

		cent->animEvent = (animEventType_t)s1->eventParm;
	}

	cent->dl_time = cg.time;	
	offset = ((float)otime) / LS_FRAMETIME;	
	cent->dl_framef += offset;

	if (cent->dl_framef >= s1->generic1) {
		switch (s1->eventParm)
		{
			// For death just end at the end frame. 
			case anim_event_death:
			case anim_event_deathb:
			case anim_event_bdeath:
			case anim_event_deathc:
				cent->dl_framef = cent->dl_frame = cent->dl_oldframe = s1->generic1;
				cent->dl_backlerp = 0;
				break;

			// Default we loop.
			default:
				cent->dl_framef = cent->dl_frame = cent->dl_oldframe = s1->startframe;
				cent->dl_backlerp = 0;
				break;
		}		
	}

	ent.frame = cent->dl_framef;
	ent.oldframe = cent->dl_framef;

	if (s1->lightRadius > 0)
	{
		vec3_t offset;
		vec3_t forward, right, up;

		ent.skipShadows = qtrue;
		VectorCopy(cent->lerpOrigin, offset);
		offset[2] += 20.0f;

		AngleVectors(cent->lerpAngles, forward, right, up);
		engine->renderer->AddLightToScene(offset, s1->lightRadius, s1->lightColor[0], s1->lightColor[1], s1->lightColor[2], (lightDistanceType_t)s1->generic1);
	}

	VectorCopy(cent->lerpOrigin, ent.origin);
	VectorCopy(cent->lerpOrigin, ent.oldorigin);

	if (s1->clientTransformAnim == CLIENT_TRANSFORM_BOB || s1->clientTransformAnim == CLIENT_TRANSFORM_BOB_AND_ROTATE) {
		float scale = 0.005 + cent->currentState.number * 0.00001;
		ent.origin[2] = ent.oldorigin[2] += 4 + cos((cg.time + 1000) * scale) * 4;
	}

	if (s1->clientTransformAnim == CLIENT_TRANSFORM_ROTATE || s1->clientTransformAnim == CLIENT_TRANSFORM_BOB_AND_ROTATE) {
		vec3_t spinAngles;

		VectorClear(spinAngles);
		spinAngles[1] = (cg.time & 1023) * 360 / 1024.0f;

		AnglesToAxis(spinAngles, ent.axis);
	}
	else
	{
		// convert angles to axis
		AnglesToAxis(cent->lerpAngles, ent.axis);
	}

	// player model
	if (s1->number == cg.snap->ps.clientNum) {
		ent.renderfx |= RF_THIRD_PERSON;	// only draw from mirrors
	}

	// add to refresh list
	engine->renderer->AddRefEntityToScene(&ent);
}