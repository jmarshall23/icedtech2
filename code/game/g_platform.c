// g_platform.c
//

#include "g_local.h"


/*
===============================================================================

PLAT

===============================================================================
*/

/*
==============
Touch_Plat

Don't allow decent if a living player is on it
===============
*/
void Touch_Plat(gentity_t* ent, gentity_t* other, trace_t* trace) {
	if (!other->client || other->client->ps.stats[STAT_HEALTH] <= 0) {
		return;
	}

	// delay return-to-pos1 by one second
	if (ent->moverState == MOVER_POS2) {
		ent->nextthink = level.time + 1000;
	}
}

/*
==============
Touch_PlatCenterTrigger

If the plat is at the bottom position, start it going up
===============
*/
void Touch_PlatCenterTrigger(gentity_t* ent, gentity_t* other, trace_t* trace) {
	if (!other->client) {
		return;
	}

	if (ent->parent->moverState == MOVER_POS1) {
		Use_BinaryMover(ent->parent, ent, other);
	}
}


/*
================
SpawnPlatTrigger

Spawn a trigger in the middle of the plat's low position
Elevator cars require that the trigger extend through the entire low position,
not just sit on top of it.
================
*/
void SpawnPlatTrigger(gentity_t* ent) {
	gentity_t* trigger;
	vec3_t	tmin, tmax;

	// the middle trigger will be a thin trigger just
	// above the starting position
	trigger = G_Spawn();
	trigger->classname = "plat_trigger";
	trigger->touch = Touch_PlatCenterTrigger;
	trigger->r.contents = CONTENTS_TRIGGER;
	trigger->parent = ent;

	tmin[0] = ent->pos1[0] + ent->r.mins[0] + 33;
	tmin[1] = ent->pos1[1] + ent->r.mins[1] + 33;
	tmin[2] = ent->pos1[2] + ent->r.mins[2];

	tmax[0] = ent->pos1[0] + ent->r.maxs[0] - 33;
	tmax[1] = ent->pos1[1] + ent->r.maxs[1] - 33;
	tmax[2] = ent->pos1[2] + ent->r.maxs[2] + 8;

	if (tmax[0] <= tmin[0]) {
		tmin[0] = ent->pos1[0] + (ent->r.mins[0] + ent->r.maxs[0]) * 0.5;
		tmax[0] = tmin[0] + 1;
	}
	if (tmax[1] <= tmin[1]) {
		tmin[1] = ent->pos1[1] + (ent->r.mins[1] + ent->r.maxs[1]) * 0.5;
		tmax[1] = tmin[1] + 1;
	}

	VectorCopy(tmin, trigger->r.mins);
	VectorCopy(tmax, trigger->r.maxs);

	engine->SV_LinkEntity(trigger);
}


/*QUAKED func_plat (0 .5 .8) ?
Plats are always drawn in the extended position so they will light correctly.

"lip"		default 8, protrusion above rest position
"height"	total height of movement, defaults to model height
"speed"		overrides default 200.
"dmg"		overrides default 2
"model2"	.md3 model to also draw
"color"		constantLight color
"light"		constantLight radius
*/
void SP_func_plat(gentity_t* ent) {
	float		lip, height;

	if (ent->sounds == 1)
	{
		ent->noise1 = G_SoundIndex("sound/plats/plat1.wav");
		ent->noise2 = G_SoundIndex("sound/plats/plat2.wav");
	}

	if (ent->sounds == 2)
	{
		ent->noise1 = G_SoundIndex("sound/plats/medplat1.wav");
		ent->noise2 = G_SoundIndex("sound/plats/medplat2.wav");
	}

	ent->sound1to2 = ent->sound2to1 = ent->noise1;
	ent->soundPos1 = ent->soundPos2 = ent->noise2;

	VectorClear(ent->s.angles);

	G_SpawnFloat("speed", "200", &ent->speed);
	G_SpawnInt("dmg", "2", &ent->damage);
	G_SpawnFloat("wait", "1", &ent->wait);
	G_SpawnFloat("lip", "8", &lip);

	// create second position
	engine->SV_SetBrushModel(ent, ent->model);

	if (!G_SpawnFloat("height", "0", &height)) {
		height = (ent->r.maxs[2] - ent->r.mins[2]) - lip;
	}

	// pos1 is the rest (bottom) position, pos2 is the top
	VectorCopy(ent->s.origin, ent->pos2);
	VectorCopy(ent->pos2, ent->pos1);
	ent->pos1[2] -= height;

	InitMover(ent);

	// touch function keeps the plat from returning while
	// a live player is standing on it
	ent->touch = Touch_Plat;

	ent->blocked = Blocked_Door;

	ent->parent = ent;	// so it can be treated as a door

	// spawn the trigger if one hasn't been custom made
	if (!ent->targetname) {
		SpawnPlatTrigger(ent);
	}
}