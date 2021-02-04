// g_door.c
//

#include "g_local.h"

typedef enum {
	DOOR_START_OPEN = 1,
	DOOR_DONT_LINK = 4,
	DOOR_GOLD_KEY = 8,
	DOOR_SILVER_KEY = 16,
	DOOR_TOGGLE = 32,
} doorSpawnType_t;

/*
===============================================================================

DOOR

A use can be triggered either by a touch function, by being shot, or by being
targeted by another entity.

===============================================================================
*/

void SP_func_bossgate(gentity_t* ent) {
	ent->s.eType = ET_MOVER;
	engine->SV_SetBrushModel(ent, ent->model);
	engine->SV_LinkEntity(ent);
}

/*
================
Blocked_Door
================
*/
void Blocked_Door(gentity_t* ent, gentity_t* other) {
	// remove anything other than a client
	if (other->s.eType != ET_PLAYER && other->s.eType != ET_MONSTER) {
		G_TempEntity(other->s.origin, EV_ITEM_POP);
		G_FreeEntity(other);
		return;
	}

	if (ent->damage) {
		G_Damage(other, ent, ent, vec_zero, vec_zero, ent->damage, 0, MOD_CRUSH);
	}
	if (ent->spawnflags & 4) {
		return;		// crushers don't reverse
	}

	// reverse direction
	Use_BinaryMover(ent, ent, other);
}

/*
================
Touch_DoorTrigger
================
*/
void Touch_DoorTrigger(gentity_t* ent, gentity_t* other, trace_t* trace) {
	if (ent->parent->spawnflags & DOOR_SILVER_KEY) {
		if (!other->client->keys[IT_KEY1])
		{
			if (level.worldtype == 2)
			{
				G_CenterPrint(other - g_entities, "You need the silver keycard");
				G_Sound(other, CHAN_VOICE, ent->noise3);
			}
			else if (level.worldtype == 1)
			{
				G_CenterPrint(other - g_entities, "You need the silver runekey");
				G_Sound(other, CHAN_VOICE, ent->noise3);
			}
			else if (level.worldtype == 0)
			{
				G_CenterPrint(other - g_entities, "You need the silver key");
				G_Sound(other, CHAN_VOICE, ent->noise3);
			}

			return;
		}
	}
	if (ent->parent->spawnflags & DOOR_GOLD_KEY)
	{
		if (!other->client->keys[IT_KEY2])
		{
			if (level.worldtype == 2)
			{
				G_CenterPrint(other - g_entities, "You need the gold keycard");
				G_Sound(other, CHAN_VOICE, ent->noise3);
			}
			else if (level.worldtype == 1)
			{
				G_CenterPrint(other - g_entities, "You need the gold runekey");
				G_Sound(other, CHAN_VOICE, ent->noise3);
			}
			else if (level.worldtype == 0)
			{
				G_CenterPrint(other - g_entities, "You need the gold key");
				G_Sound(other, CHAN_VOICE, ent->noise3);
			}

			return;
		}
	}

	if (ent->parent->moverState != MOVER_1TO2) {
		Use_BinaryMover(ent->parent, ent, other);
	}
}


/*
======================
Think_SpawnNewDoorTrigger

All of the parts of a door have been spawned, so create
a trigger that encloses all of them
======================
*/
void Think_SpawnNewDoorTrigger(gentity_t* ent) {
	gentity_t* other;
	vec3_t		mins, maxs;
	int			i, best;

	// set all of the slaves as shootable
	for (other = ent; other; other = other->teamchain) {
		other->takedamage = qtrue;
	}

	// find the bounds of everything on the team
	VectorCopy(ent->r.absmin, mins);
	VectorCopy(ent->r.absmax, maxs);

	for (other = ent->teamchain; other; other = other->teamchain) {
		AddPointToBounds(other->r.absmin, mins, maxs);
		AddPointToBounds(other->r.absmax, mins, maxs);
	}

	// find the thinnest axis, which will be the one we expand
	best = 0;
	for (i = 1; i < 3; i++) {
		if (maxs[i] - mins[i] < maxs[best] - mins[best]) {
			best = i;
		}
	}
	maxs[best] += 30;
	mins[best] -= 30;

	// create a trigger with this size
	if (!ent->secret)
	{
		other = G_Spawn();
		other->classname = "door_trigger";
		VectorCopy(mins, other->r.mins);
		VectorCopy(maxs, other->r.maxs);
		other->parent = ent;
		other->r.contents = CONTENTS_TRIGGER;
		other->touch = Touch_DoorTrigger;
		// remember the thinnest axis
		other->count = best;
		engine->SV_LinkEntity(other);
	}

	MatchTeam(ent, ent->moverState, level.time);
}

void Think_MatchTeam(gentity_t* ent) {
	MatchTeam(ent, ent->moverState, level.time);
}

void DoorSpawn(gentity_t* ent) {
	vec3_t	abs_movedir;
	float	distance;
	vec3_t	size;
	float	lip;

	ent->sound1to2 = ent->sound2to1 = ent->noise1;
	ent->soundPos1 = ent->soundPos2 = ent->noise2;

	ent->blocked = Blocked_Door;

	// default speed of 400
	if (!ent->speed)
		ent->speed = 400;

	// default wait of 2 seconds
	if (!ent->wait)
		ent->wait = 2;
	ent->wait *= 1000;

	// default lip of 8 units
	G_SpawnFloat("lip", "8", &lip);

	// default damage of 2 points
	G_SpawnInt("dmg", "2", &ent->damage);

	// first position at start
	VectorCopy(ent->s.origin, ent->pos1);

	// calculate second position
	engine->SV_SetBrushModel(ent, ent->model);
	G_SetMovedir(ent->s.angles, ent->movedir);
	abs_movedir[0] = fabs(ent->movedir[0]);
	abs_movedir[1] = fabs(ent->movedir[1]);
	abs_movedir[2] = fabs(ent->movedir[2]);
	VectorSubtract(ent->r.maxs, ent->r.mins, size);
	distance = DotProduct(abs_movedir, size) - lip;
	VectorMA(ent->pos1, distance, ent->movedir, ent->pos2);

	// if "start_open", reverse position 1 and 2
	if ((ent->spawnflags & DOOR_START_OPEN) && !ent->secret) {
		vec3_t	temp;

		VectorCopy(ent->pos2, temp);
		VectorCopy(ent->s.origin, ent->pos2);
		VectorCopy(temp, ent->pos1);
	}

	InitMover(ent);

	ent->nextthink = level.time + FRAMETIME;

	if (!(ent->flags & FL_TEAMSLAVE)) {
		int health;

		G_SpawnInt("health", "0", &health);
		if (health) {
			ent->takedamage = qtrue;
		}
		if (ent->targetname || health) {
			// non touch/shoot doors
			ent->think = Think_MatchTeam;
		}
		else {
			ent->think = Think_SpawnNewDoorTrigger;
		}
	}
}


/*QUAKED func_door (0 .5 .8) ? START_OPEN x CRUSHER
TOGGLE		wait in both the start and end states for a trigger event.
START_OPEN	the door to moves to its destination when spawned, and operate in reverse.  It is used to temporarily or permanently close off an area when triggered (not useful for touch or takedamage doors).
NOMONSTER	monsters will not trigger this door

"model2"	.md3 model to also draw
"angle"		determines the opening direction
"targetname" if set, no touch field will be spawned and a remote button or trigger field activates the door.
"speed"		movement speed (100 default)
"wait"		wait before returning (3 default, -1 = never return)
"lip"		lip remaining at end of move (8 default)
"dmg"		damage to inflict when blocked (2 default)
"color"		constantLight color
"light"		constantLight radius
"health"	if set, the door must be shot open
*/
void SP_func_door(gentity_t* ent) {
	switch (level.worldtype)
	{
	case 0:
		ent->noise1 = 0;
		ent->noise2 = 0;
		ent->noise3 = G_SoundIndex("sound/doors/medtry.wav");
		ent->noise4 = G_SoundIndex("sound/doors/meduse.wav");
		break;
	case 1:
		ent->noise1 = G_SoundIndex("sound/doors/drclos4.wav");
		ent->noise2 = G_SoundIndex("sound/doors/doormv1.wav");
		ent->noise3 = G_SoundIndex("sound/doors/runetry.wav");
		ent->noise4 = G_SoundIndex("sound/doors/runeuse.wav");
		break;
	case 2:
		ent->noise1 = G_SoundIndex("sound/doors/hydro1.wav");
		ent->noise2 = G_SoundIndex("sound/doors/hydro2.wav");
		ent->noise3 = G_SoundIndex("sound/doors/basetry.wav");
		ent->noise4 = G_SoundIndex("sound/doors/baseuse.wav");
		break;
	case 3:
		ent->noise1 = G_SoundIndex("sound/doors/stndr1.wav");
		ent->noise2 = G_SoundIndex("sound/doors/stndr2.wav");
		break;
	case 4:
		ent->noise1 = G_SoundIndex("sound/doors/ddoor2.wav");
		ent->noise2 = G_SoundIndex("sound/doors/ddoor1.wav");
		break;
	}

	DoorSpawn(ent);
}

void SP_func_secret_door(gentity_t* ent) {
	ent->secret = qtrue;

	if (ent->targetname) {
		ent->doorNoClose = qtrue;
	}

	switch (level.worldtype)
	{
	case 1:
		ent->noise1 = G_SoundIndex("sound/doors/latch2.wav");
		ent->noise2 = G_SoundIndex("sound/doors/winch2.wav");
		ent->noise3 = G_SoundIndex("sound/doors/drclos4.wav");
		ent->noise4 = 0;
		break;
	case 2:
		ent->noise1 = G_SoundIndex("sound/doors/airdoor1.wav");
		ent->noise2 = G_SoundIndex("sound/doors/airdoor2.wav");
		ent->noise3 = G_SoundIndex("sound/doors/airdoor2.wav");
		ent->noise4 = 0;
		break;
	case 0:
	case 3:
		ent->noise1 = G_SoundIndex("sound/doors/basesec1.wav");
		ent->noise2 = G_SoundIndex("sound/doors/basesec2.wav");
		ent->noise3 = G_SoundIndex("sound/doors/basesec2.wav");
		break;
	}

	DoorSpawn(ent);
}