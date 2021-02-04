// g_ai.c
//

#include "g_local.h"

qboolean FindTarget(gentity_t* self);
extern cvar_t* maxclients;

qboolean ai_checkattack(gentity_t* self, float dist);

qboolean	enemy_vis;
qboolean	enemy_infront;
int			enemy_range;
float		enemy_yaw;

//============================================================================

void G_ProjectSource(vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
{
	result[0] = point[0] + forward[0] * distance[0] + right[0] * distance[1];
	result[1] = point[1] + forward[1] * distance[0] + right[1] * distance[1];
	result[2] = point[2] + forward[2] * distance[0] + right[2] * distance[1] + distance[2];
}


/*
=================
AI_SetSightClient

Called once each frame to set level.sight_client to the
player to be checked for in findtarget.

If all clients are either dead or in notarget, sight_client
will be null.

In coop games, sight_client will cycle between the clients.
=================
*/
void AI_SetSightClient(void)
{
	gentity_t* ent;
	int		start, check;

	if (level.sight_client == NULL)
		start = 0;
	else
		start = level.sight_client - g_entities;

	check = start;
	while (1)
	{		
		if (check > MAX_CLIENTS)
			check = 1;
		ent = &g_entities[check];
		if (ent->inuse
			&& ent->health > 0
			&& !(ent->flags & FL_NOTARGET))
		{
			level.sight_client = ent;
			return;		// got one
		}
		if (check == start)
		{
			level.sight_client = NULL;
			return;		// nobody to see
		}

		check++;
	}
}

//============================================================================

/*
=============
ai_move

Move the specified distance at current facing.
This replaces the QC functions: ai_forward, ai_back, ai_pain, and ai_painforward
==============
*/
void ai_move(gentity_t* self, float dist)
{
	M_walkmove(self, self->s.angles[YAW], dist);
}


/*
=============
ai_stand

Used for standing around and looking for players
Distance is for slight position adjustments needed by the animations
==============
*/
void ai_stand(gentity_t* self, float dist)
{
	vec3_t	v;

	if (dist)
		M_walkmove(self, self->s.angles[YAW], dist);

	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		if (self->enemy)
		{
			VectorSubtract(self->enemy->r.currentOrigin, self->r.currentOrigin, v);
			self->ideal_yaw = vectoyaw(v);
			if (self->s.angles[YAW] != self->ideal_yaw && self->monsterinfo.aiflags & AI_TEMP_STAND_GROUND)
			{
				self->monsterinfo.aiflags &= ~(AI_STAND_GROUND | AI_TEMP_STAND_GROUND);
				self->monsterinfo.run(self);
			}
			M_ChangeYaw(self);
			ai_checkattack(self, 0);
		}
		else
			FindTarget(self);
		return;
	}

	if (FindTarget(self))
		return;

	if (level.time > self->monsterinfo.pausetime)
	{
		self->monsterinfo.walk(self);
		return;
	}

	if (!(self->spawnflags & 1) && (self->monsterinfo.idle) && (level.time > self->monsterinfo.idle_time))
	{
		if (self->monsterinfo.idle_time)
		{
			self->monsterinfo.idle(self);
			self->monsterinfo.idle_time = level.time + 15 + random() * 15;
		}
		else
		{
			self->monsterinfo.idle_time = level.time + random() * 15;
		}
	}
}


/*
=============
ai_walk

The monster is walking it's beat
=============
*/
void ai_walk(gentity_t* self, float dist)
{
	M_MoveToGoal(self, dist);

	// check for noticing a player
	if (FindTarget(self))
		return;

	if ((self->monsterinfo.search) && (level.time > self->monsterinfo.idle_time))
	{
		if (self->monsterinfo.idle_time)
		{
			self->monsterinfo.search(self);
			self->monsterinfo.idle_time = level.time + 15 + random() * 15;
		}
		else
		{
			self->monsterinfo.idle_time = level.time + random() * 15;
		}
	}
}


/*
=============
ai_charge

Turns towards target and advances
Use this call with a distnace of 0 to replace ai_face
==============
*/
void ai_charge(gentity_t* self, float dist)
{
	vec3_t	v;

	VectorSubtract(self->enemy->r.currentOrigin, self->r.currentOrigin, v);
	self->ideal_yaw = vectoyaw(v);
	M_ChangeYaw(self);

	if (dist)
		M_walkmove(self, self->s.angles[YAW], dist);
}

void ai_charge_side(gentity_t* self, float dist)
{
	vec3_t forward, right, up;
	vec3_t	dtemp;
	float	heading;

	// aim to the left of the enemy for a flyby

	vec3_t	v;

	VectorSubtract(self->enemy->r.currentOrigin, self->r.currentOrigin, v);
	self->ideal_yaw = vectoyaw(v);
	M_ChangeYaw(self);

	//makevectors(self.angles);
	AngleVectors(self->s.angles, forward, right, up);
	dtemp[0] = self->enemy->r.currentOrigin[0] - 30 * right[0];
	dtemp[1] = self->enemy->r.currentOrigin[1] - 30 * right[1];
	dtemp[2] = self->enemy->r.currentOrigin[2] - 30 * right[2];

	VectorSubtract(dtemp, self->r.currentOrigin, v);
	heading = vectoyaw(v);

	//walkmove(heading, 20);
	M_walkmove(self, heading, 20);
};

/*
=============
ai_melee

=============
*/
void ai_melee(gentity_t* self, float dist)
{
	vec3_t	delta;
	float 	ldmg;

	if (!self->enemy)
		return;		// removed before stroke

	VectorSubtract(self->enemy->r.currentOrigin, self->r.currentOrigin, delta);

	if (VectorLength(delta) > 60)
		return;

	ldmg = (random() + random() + random()) * 3;
	G_Damage(self->enemy, self, self, vec3_origin, self->r.currentOrigin, ldmg, 0, MOD_UNKNOWN);
};

void ai_melee_side(gentity_t* self, float dist)
{
	vec3_t	delta;
	float 	ldmg;

	if (!self->enemy)
		return;		// removed before stroke

	ai_charge_side(self, 0);

	VectorSubtract(self->enemy->r.currentOrigin, self->r.currentOrigin, delta);

	if (VectorLength(delta) > 60)
		return;
	if (!CanDamage(self->enemy, self->s.origin))
		return;
	ldmg = (random() + random() + random()) * 3;
	//T_Damage(self.enemy, self, self, ldmg);
	G_Damage(self->enemy, self, self, vec3_origin, self->r.currentOrigin, ldmg, 0, MOD_UNKNOWN);
};

/*
=============
ai_turn

don't move, but turn towards ideal_yaw
Distance is for slight position adjustments needed by the animations
=============
*/
void ai_turn(gentity_t* self, float dist)
{
	if (dist)
		M_walkmove(self, self->s.angles[YAW], dist);

	if (FindTarget(self))
		return;

	M_ChangeYaw(self);
}

/*
=============
ai_face

Stay facing the enemy
=============
*/
void ai_face(gentity_t* self, float dist)
{
	vec3_t d;

	VectorSubtract(self->enemy->r.currentOrigin, self->r.currentOrigin, d);

	self->ideal_yaw = vectoyaw(d);
	M_ChangeYaw(self);
};

/*
=============
ai_pain

stagger back a bit
=============
*/
void ai_pain(gentity_t* self, float dist)
{
	ai_move(self, -dist);
}

/*
=============
ai_back
=============
*/
void ai_back(gentity_t* self, float dist)
{
	ai_move(self, -dist);
}

/*
=============
ai_pain

stagger back a bit
=============
*/
void ai_painforward(gentity_t* self, float dist)
{
	ai_move(self, dist);
}

/*

.enemy
Will be world if not currently angry at anyone.

.movetarget
The next path spot to walk toward.  If .enemy, ignore .movetarget.
When an enemy is killed, the monster will try to return to it's path.

.hunt_time
Set to time + something when the player is in sight, but movement straight for
him is blocked.  This causes the monster to use wall following code for
movement direction instead of sighting on the player.

.ideal_yaw
A yaw angle of the intended direction, which will be turned towards at up
to 45 deg / state.  If the enemy is in view and hunt_time is not active,
this will be the exact line towards the enemy.

.pausetime
A monster will leave it's stand state and head towards it's .movetarget when
time > .pausetime.

walkmove(angle, speed) primitive is all or nothing
*/

/*
=============
range

returns the range catagorization of an entity reletive to self
0	melee range, will become hostile even if back is turned
1	visibility and infront, or visibility and show hostile
2	infront and show hostile
3	only triggered by damage
=============
*/
int range(gentity_t* self, gentity_t* other)
{
	vec3_t	v;
	float	len;

	VectorSubtract(self->r.currentOrigin, other->r.currentOrigin, v);
	len = VectorLength(v);
	if (len < MELEE_DISTANCE)
		return RANGE_MELEE;
	if (len < 500)
		return RANGE_NEAR;
	if (len < 1000)
		return RANGE_MID;
	return RANGE_FAR;
}

/*
=============
visible

returns 1 if the entity is visible to self, even if not infront ()
=============
*/
qboolean visible(gentity_t* self, gentity_t* other)
{
	vec3_t	spot1;
	vec3_t	spot2;
	trace_t	trace;

	VectorCopy(self->r.currentOrigin, spot1);
	spot1[2] += self->viewheight;
	VectorCopy(other->r.currentOrigin, spot2);
	spot2[2] += other->viewheight;
	engine->SV_Trace(&trace, spot1, vec3_origin, vec3_origin, spot2, self - g_entities, MASK_OPAQUE);

	if (trace.fraction == 1.0)
		return qtrue;
	return qfalse;
}


/*
=============
infront

returns 1 if the entity is in front (in sight) of self
=============
*/
qboolean infront(gentity_t* self, gentity_t* other)
{
	vec3_t	vec;
	float	dot;
	vec3_t	forward;

	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorSubtract(other->r.currentOrigin, self->r.currentOrigin, vec);
	VectorNormalize(vec);
	dot = DotProduct(vec, forward);

	if (dot > 0.3)
		return qtrue;
	return qfalse;
}


//============================================================================

void HuntTarget(gentity_t* self)
{
	vec3_t	vec;

	self->goalentity = self->enemy;
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.stand(self);
	else
		self->monsterinfo.run(self);
	VectorSubtract(self->enemy->r.currentOrigin, self->r.currentOrigin, vec);
	self->ideal_yaw = vectoyaw(vec);
	// wait a while before first attack
	if (!(self->monsterinfo.aiflags & AI_STAND_GROUND))
		AttackFinished(self, 1);
}

void FoundTarget(gentity_t* self)
{
	// let other monsters see this monster for a while
	if (self->enemy->client)
	{
		level.sight_entity = self;
		level.sight_entity_framenum = level.framenum;
		//level.sight_entity->light_level = 128;
	}

	self->show_hostile = level.time + 1;		// wake up other monsters

	VectorCopy(self->enemy->r.currentOrigin, self->monsterinfo.last_sighting);
	self->monsterinfo.trail_time = level.time;

	if (!self->combattarget)
	{
		HuntTarget(self);
		return;
	}

	self->goalentity = self->movetarget = G_PickTarget(self->combattarget->targetname);
	if (!self->movetarget)
	{
		self->goalentity = self->movetarget = self->enemy;
		HuntTarget(self);
		Com_Printf("%s at %s, combattarget %s not found\n", self->classname, vtos(self->r.currentOrigin), self->combattarget);
		return;
	}

	// clear out our combattarget, these are a one shot deal
	self->combattarget = NULL;
	self->monsterinfo.aiflags |= AI_COMBAT_POINT;

	// clear the targetname, that point is ours!
	self->movetarget->targetname = NULL;
	self->monsterinfo.pausetime = 0;

	// run for it
	self->monsterinfo.run(self);
}


/*
===========
FindTarget

Self is currently not attacking anything, so try to find a target

Returns qtrue if an enemy was sighted

When a player fires a missile, the point of impact becomes a fakeplayer so
that monsters that see the impact will respond as if they had seen the
player.

To avoid spending too much time, only a single client (or fakeclient) is
checked each frame.  This means multi player games will have slightly
slower noticing monsters.
============
*/
qboolean FindTarget(gentity_t* self)
{
	gentity_t* client;
	qboolean	heardit;
	int			r;

	if (self->monsterinfo.aiflags & AI_GOOD_GUY)
	{
		if (self->goalentity && self->goalentity->inuse && self->goalentity->classname)
		{
			if (strcmp(self->goalentity->classname, "target_actor") == 0)
				return qfalse;
		}

		//FIXME look for monsters?
		return qfalse;
	}

	// if we're going to a combat point, just proceed
	if (self->monsterinfo.aiflags & AI_COMBAT_POINT)
		return qfalse;

	// if the first spawnflag bit is set, the monster will only wake up on
	// really seeing the player, not another monster getting angry or hearing
	// something

	// revised behavior so they will wake up if they "see" a player make a noise
	// but not weapon impact/explosion noises

	heardit = qfalse;
	if ((level.sight_entity_framenum >= (level.framenum - 1)) && !(self->spawnflags & 1))
	{
		client = level.sight_entity;
		if (client->enemy == self->enemy)
		{
			return qfalse;
		}
	}
	else if (level.sound_entity_framenum >= (level.framenum - 1))
	{
		client = level.sound_entity;
		heardit = qtrue;
	}
	else if (!(self->enemy) && (level.sound2_entity_framenum >= (level.framenum - 1)) && !(self->spawnflags & 1))
	{
		client = level.sound2_entity;
		heardit = qtrue;
	}
	else
	{
		client = level.sight_client;
		if (!client)
			return qfalse;	// no clients to get mad at
	}

	// if the entity went away, forget it
	if (!client->inuse)
		return qfalse;

	if (client == self->enemy)
		return qtrue;	// JDC qfalse;

	if (client->client)
	{
		if (client->flags & FL_NOTARGET)
			return qfalse;
	}
	else if (client->svflags & SVF_MONSTER)
	{
		if (!client->enemy)
			return qfalse;
		if (client->enemy->flags & FL_NOTARGET)
			return qfalse;
	}
	else if (heardit)
	{
		if (client->flags & FL_NOTARGET)
			return qfalse;
	}
	else
		return qfalse;

	if (!heardit)
	{
		r = range(self, client);

		if (r == RANGE_FAR)
			return qfalse;

		// this is where we would check invisibility

				// is client in an spot too dark to be seen?
		//if (client->light_level <= 5)
		//	return qfalse;

		if (!visible(self, client))
		{
			return qfalse;
		}

		if (r == RANGE_NEAR)
		{
			if (client->show_hostile < level.time && !infront(self, client))
			{
				return qfalse;
			}
		}
		else if (r == RANGE_MID)
		{
			if (!infront(self, client))
			{
				return qfalse;
			}
		}

		self->enemy = client;

		if (strcmp(self->enemy->classname, "player_noise") != 0)
		{
			self->monsterinfo.aiflags &= ~AI_SOUND_TARGET;

			if (!self->enemy->client)
			{
				self->enemy = self->enemy->enemy;
				if (!self->enemy->client)
				{
					self->enemy = NULL;
					return qfalse;
				}
			}
		}
	}
	else	// heardit
	{
		vec3_t	temp;

		if (self->spawnflags & 1)
		{
			if (!visible(self, client))
				return qfalse;
		}
		else
		{
			if (!engine->SV_inPVS(self->r.currentOrigin, client->r.currentOrigin))
				return qfalse;
		}

		VectorSubtract(client->r.currentOrigin, self->r.currentOrigin, temp);

		if (VectorLength(temp) > 1000)	// too far to hear
		{
			return qfalse;
		}

		// check area portals - if they are different and not connected then we can't hear it
		if (client->r.areanum != self->r.areanum)
			if (!engine->CM_AreasConnected(self->r.areanum, client->r.areanum))
				return qfalse;

		self->ideal_yaw = vectoyaw(temp);
		M_ChangeYaw(self);

		// hunt the sound for a bit; hopefully find the real player
		self->monsterinfo.aiflags |= AI_SOUND_TARGET;
		self->enemy = client;
	}

	//
	// got one
	//
	FoundTarget(self);

	if (!(self->monsterinfo.aiflags & AI_SOUND_TARGET) && (self->monsterinfo.sight))
		self->monsterinfo.sight(self, self->enemy);

	return qtrue;
}


//=============================================================================

/*
============
FacingIdeal

============
*/
qboolean FacingIdeal(gentity_t* self)
{
	float	delta;

	delta = anglemod(self->s.angles[YAW] - self->ideal_yaw);
	if (delta > 45 && delta < 315)
		return qfalse;
	return qtrue;
}


//=============================================================================

qboolean M_CheckAttack(gentity_t* self)
{
	vec3_t	spot1, spot2;
	float	chance;
	trace_t	tr;

	if (!self->enemy)
		return qfalse;

	if (self->enemy->health > 0)
	{
		// see if any entities are in the way of the shot
		VectorCopy(self->r.currentOrigin, spot1);
		spot1[2] += self->viewheight;
		VectorCopy(self->enemy->r.currentOrigin, spot2);
		spot2[2] += self->enemy->viewheight;

		engine->SV_Trace(&tr, spot1, vec_zero, vec_zero, spot2, self - g_entities, CONTENTS_SOLID | CONTENTS_MONSTER | CONTENTS_SLIME | CONTENTS_LAVA | CONTENTS_WINDOW);

		// do we have a clear shot?
		if (&g_entities[tr.entityNum] != self->enemy)
			return qfalse;
	}

	// melee attack
	if (enemy_range == RANGE_MELEE)
	{
		// don't always melee in easy mode
		if (g_skill.value == 0 && (rand() & 3))
			return qfalse;
		if (self->monsterinfo.melee)
			self->monsterinfo.attack_state = AS_MELEE;
		else
			self->monsterinfo.attack_state = AS_MISSILE;
		return qtrue;
	}

	// missile attack
	if (!self->monsterinfo.attack)
		return qfalse;

	if (level.time < self->monsterinfo.attack_finished)
		return qfalse;

	if (enemy_range == RANGE_FAR)
		return qfalse;

	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		chance = 0.4;
	}
	else if (enemy_range == RANGE_MELEE)
	{
		chance = 0.2;
	}
	else if (enemy_range == RANGE_NEAR)
	{
		chance = 0.1;
	}
	else if (enemy_range == RANGE_MID)
	{
		chance = 0.02;
	}
	else
	{
		return qfalse;
	}

	if (g_skill.value == 0)
		chance *= 0.5;
	else if (g_skill.value >= 2)
		chance *= 2;

	if (random() < chance)
	{
		self->monsterinfo.attack_state = AS_MISSILE;
		self->monsterinfo.attack_finished = level.time + 2 * random();
		return qtrue;
	}

	if (self->flags & FL_FLY)
	{
		if (random() < 0.3)
			self->monsterinfo.attack_state = AS_SLIDING;
		else
			self->monsterinfo.attack_state = AS_STRAIGHT;
	}

	return qfalse;
}


/*
=============
ai_run_melee

Turn and close until within an angle to launch a melee attack
=============
*/
void ai_run_melee(gentity_t* self)
{
	self->ideal_yaw = enemy_yaw;
	M_ChangeYaw(self);

	if (FacingIdeal(self))
	{
		self->monsterinfo.melee(self);
		self->monsterinfo.attack_state = AS_STRAIGHT;
	}
}


/*
=============
ai_run_missile

Turn in place until within an angle to launch a missile attack
=============
*/
void ai_run_missile(gentity_t* self)
{
	self->ideal_yaw = enemy_yaw;
	M_ChangeYaw(self);

	if (FacingIdeal(self))
	{
		self->monsterinfo.attack(self);
		self->monsterinfo.attack_state = AS_STRAIGHT;
	}
};


/*
=============
ai_run_slide

Strafe sideways, but stay at aproximately the same range
=============
*/
void ai_run_slide(gentity_t* self, float distance)
{
	float	ofs;

	self->ideal_yaw = enemy_yaw;
	M_ChangeYaw(self);

	if (self->monsterinfo.lefty)
		ofs = 90;
	else
		ofs = -90;

	if (M_walkmove(self, self->ideal_yaw + ofs, distance))
		return;

	self->monsterinfo.lefty = 1 - self->monsterinfo.lefty;
	M_walkmove(self, self->ideal_yaw - ofs, distance);
}


/*
=============
ai_checkattack

Decides if we're going to attack or do something else
used by ai_run and ai_stand
=============
*/
qboolean ai_checkattack(gentity_t* self, float dist)
{
	vec3_t		temp;
	qboolean	hesDeadJim;

	// this causes monsters to run blindly to the combat point w/o firing
	if (self->goalentity)
	{
		if (self->monsterinfo.aiflags & AI_COMBAT_POINT)
			return qfalse;

		if (self->monsterinfo.aiflags & AI_SOUND_TARGET)
		{
			if ((level.time - self->enemy->teleport_time) > 5.0)
			{
				if (self->goalentity == self->enemy)
					if (self->movetarget)
						self->goalentity = self->movetarget;
					else
						self->goalentity = NULL;
				self->monsterinfo.aiflags &= ~AI_SOUND_TARGET;
				if (self->monsterinfo.aiflags & AI_TEMP_STAND_GROUND)
					self->monsterinfo.aiflags &= ~(AI_STAND_GROUND | AI_TEMP_STAND_GROUND);
			}
			else
			{
				self->show_hostile = level.time + 1;
				return qfalse;
			}
		}
	}

	enemy_vis = qfalse;

	// see if the enemy is dead
	hesDeadJim = qfalse;
	if ((!self->enemy) || (!self->enemy->inuse))
	{
		hesDeadJim = qtrue;
	}
	else if (self->monsterinfo.aiflags & AI_MEDIC)
	{
		if (self->enemy->health > 0)
		{
			hesDeadJim = qtrue;
			self->monsterinfo.aiflags &= ~AI_MEDIC;
		}
	}
	else
	{
		if (self->monsterinfo.aiflags & AI_BRUTAL)
		{
			if (self->enemy->health <= -80)
				hesDeadJim = qtrue;
		}
		else
		{
			if (self->enemy->health <= 0)
				hesDeadJim = qtrue;
		}
	}

	if (hesDeadJim)
	{
		self->enemy = NULL;
		// FIXME: look all around for other targets
		if (self->oldenemy && self->oldenemy->health > 0)
		{
			self->enemy = self->oldenemy;
			self->oldenemy = NULL;
			HuntTarget(self);
		}
		else
		{
			if (self->movetarget)
			{
				self->goalentity = self->movetarget;
				self->monsterinfo.walk(self);
			}
			else
			{
				// we need the pausetime otherwise the stand code
				// will just revert to walking with no target and
				// the monsters will wonder around aimlessly trying
				// to hunt the world entity
				self->monsterinfo.pausetime = level.time + 100000000;
				self->monsterinfo.stand(self);
			}
			return qtrue;
		}
	}

	self->show_hostile = level.time + 1;		// wake up other monsters

// check knowledge of enemy
	enemy_vis = visible(self, self->enemy);
	if (enemy_vis)
	{
		self->monsterinfo.search_time = level.time + 5;
		VectorCopy(self->enemy->r.currentOrigin, self->monsterinfo.last_sighting);
	}

	// look for other coop players here
	//	if (coop && self->monsterinfo.search_time < level.time)
	//	{
	//		if (FindTarget (self))
	//			return qtrue;
	//	}

	enemy_infront = infront(self, self->enemy);
	enemy_range = range(self, self->enemy);
	VectorSubtract(self->enemy->r.currentOrigin, self->r.currentOrigin, temp);
	enemy_yaw = vectoyaw(temp);


	// JDC self->ideal_yaw = enemy_yaw;

	if (self->monsterinfo.attack_state == AS_MISSILE)
	{
		ai_run_missile(self);
		return qtrue;
	}
	if (self->monsterinfo.attack_state == AS_MELEE)
	{
		ai_run_melee(self);
		return qtrue;
	}

	// if enemy is not currently visible, we will never attack
	if (!enemy_vis)
		return qfalse;

	return (qboolean)self->monsterinfo.checkattack(self);
}


/*
=============
ai_run

The monster has an enemy it is trying to kill
=============
*/
void ai_run(gentity_t* self, float dist)
{
	vec3_t		v;
	gentity_t* tempgoal;
	gentity_t* save;
	qboolean	_new;
	gentity_t* marker;
	float		d1, d2;
	trace_t		tr;
	vec3_t		v_forward, v_right;
	float		left, center, right;
	vec3_t		left_target, right_target;

	// if we're going to a combat point, just proceed
	if (self->monsterinfo.aiflags & AI_COMBAT_POINT)
	{
		M_MoveToGoal(self, dist);
		return;
	}

	if (self->monsterinfo.aiflags & AI_SOUND_TARGET)
	{
		VectorSubtract(self->r.currentOrigin, self->enemy->r.currentOrigin, v);
		if (VectorLength(v) < 64)
		{
			self->monsterinfo.aiflags |= (AI_STAND_GROUND | AI_TEMP_STAND_GROUND);
			self->monsterinfo.stand(self);
			return;
		}

		M_MoveToGoal(self, dist);

		if (!FindTarget(self))
			return;
	}

	if (ai_checkattack(self, dist))
		return;

	if (self->monsterinfo.attack_state == AS_SLIDING)
	{
		ai_run_slide(self, dist);
		return;
	}

	if (enemy_vis)
	{
		//		if (self.aiflags & AI_LOST_SIGHT)
		//			dprint("regained sight\n");
		M_MoveToGoal(self, dist);
		self->monsterinfo.aiflags &= ~AI_LOST_SIGHT;
		VectorCopy(self->enemy->r.currentOrigin, self->monsterinfo.last_sighting);
		self->monsterinfo.trail_time = level.time;
		return;
	}

// jmarshall
	// coop will change to another enemy if visible
	//if (coop->value)
	//{	// FIXME: insane guys get mad with this, which causes crashes!
	//	if (FindTarget(self))
	//		return;
	//}
// j marshall end

	if ((self->monsterinfo.search_time) && (level.time > (self->monsterinfo.search_time + 20)))
	{
		M_MoveToGoal(self, dist);
		self->monsterinfo.search_time = 0;
		//		dprint("search timeout\n");
		return;
	}

	save = self->goalentity;
	tempgoal = G_Spawn();
	self->goalentity = tempgoal;

	_new = qfalse;

	if (!(self->monsterinfo.aiflags & AI_LOST_SIGHT))
	{
		// just lost sight of the player, decide where to go first
//		dprint("lost sight of player, last seen at "); dprint(vtos(self.last_sighting)); dprint("\n");
		self->monsterinfo.aiflags |= (AI_LOST_SIGHT | AI_PURSUIT_LAST_SEEN);
		self->monsterinfo.aiflags &= ~(AI_PURSUE_NEXT | AI_PURSUE_TEMP);
		_new = qtrue;
	}

	if (self->monsterinfo.aiflags & AI_PURSUE_NEXT)
	{
		self->monsterinfo.aiflags &= ~AI_PURSUE_NEXT;
		//		dprint("reached current goal: "); dprint(vtos(self.origin)); dprint(" "); dprint(vtos(self.last_sighting)); dprint(" "); dprint(ftos(vlen(self.origin - self.last_sighting))); dprint("\n");

				// give ourself more time since we got this far
		self->monsterinfo.search_time = level.time + 5;

		if (self->monsterinfo.aiflags & AI_PURSUE_TEMP)
		{
			//			dprint("was temp goal; retrying original\n");
			self->monsterinfo.aiflags &= ~AI_PURSUE_TEMP;
			marker = NULL;
			VectorCopy(self->monsterinfo.saved_goal, self->monsterinfo.last_sighting);
			_new = qtrue;
		}
		else if (self->monsterinfo.aiflags & AI_PURSUIT_LAST_SEEN)
		{
			self->monsterinfo.aiflags &= ~AI_PURSUIT_LAST_SEEN;
			marker = PlayerTrail_PickFirst(self);
		}
		else
		{
			marker = PlayerTrail_PickNext(self);
		}

		if (marker)
		{
			VectorCopy(marker->r.currentOrigin, self->monsterinfo.last_sighting);
			self->monsterinfo.trail_time = marker->timestamp;
			self->s.angles[YAW] = self->ideal_yaw = marker->s.angles[YAW];
			//			dprint("heading is "); dprint(ftos(self.ideal_yaw)); dprint("\n");

			//			debug_drawline(self.origin, self.last_sighting, 52);
			_new = qtrue;
		}
	}

	VectorSubtract(self->r.currentOrigin, self->monsterinfo.last_sighting, v);
	d1 = VectorLength(v);
	if (d1 <= dist)
	{
		self->monsterinfo.aiflags |= AI_PURSUE_NEXT;
		dist = d1;
	}

	VectorCopy(self->monsterinfo.last_sighting, self->goalentity->r.currentOrigin);

	if (_new)
	{
		//		gi.dprintf("checking for course correction\n");

		//tr = gi.trace(self->r.currentOrigin, self->r.mins, self->r.maxs, self->monsterinfo.last_sighting, self, MASK_PLAYERSOLID);
		engine->SV_Trace(&tr, self->r.currentOrigin, self->r.mins, self->r.maxs, right_target, self - g_entities, MASK_PLAYERSOLID);
		if (tr.fraction < 1)
		{
			VectorSubtract(self->goalentity->r.currentOrigin, self->r.currentOrigin, v);
			d1 = VectorLength(v);
			center = tr.fraction;
			d2 = d1 * ((center + 1) / 2);
			self->s.angles[YAW] = self->ideal_yaw = vectoyaw(v);
			AngleVectors(self->s.angles, v_forward, v_right, NULL);

			VectorSet(v, d2, -16, 0);
			G_ProjectSource(self->r.currentOrigin, v, v_forward, v_right, left_target);
			//tr = gi.trace(self->r.currentOrigin, self->r.mins, self->r.maxs, left_target, self, MASK_PLAYERSOLID);
			engine->SV_Trace(&tr, self->r.currentOrigin, self->r.mins, self->r.maxs, left_target, self - g_entities, MASK_PLAYERSOLID);
			left = tr.fraction;

			VectorSet(v, d2, 16, 0);
			G_ProjectSource(self->r.currentOrigin, v, v_forward, v_right, right_target);
			//tr = gi.trace(self->r.currentOrigin, self->r.mins, self->r.maxs, right_target, self, MASK_PLAYERSOLID);
			engine->SV_Trace(&tr, self->r.currentOrigin, self->r.mins, self->r.maxs, right_target, self - g_entities, MASK_PLAYERSOLID);
			right = tr.fraction;

			center = (d1 * center) / d2;
			if (left >= center && left > right)
			{
				if (left < 1)
				{
					VectorSet(v, d2 * left * 0.5, -16, 0);
					G_ProjectSource(self->r.currentOrigin, v, v_forward, v_right, left_target);
					//					gi.dprintf("incomplete path, go part way and adjust again\n");
				}
				VectorCopy(self->monsterinfo.last_sighting, self->monsterinfo.saved_goal);
				self->monsterinfo.aiflags |= AI_PURSUE_TEMP;
				VectorCopy(left_target, self->goalentity->r.currentOrigin);
				VectorCopy(left_target, self->monsterinfo.last_sighting);
				VectorSubtract(self->goalentity->r.currentOrigin, self->r.currentOrigin, v);
				self->s.angles[YAW] = self->ideal_yaw = vectoyaw(v);
				//				gi.dprintf("adjusted left\n");
				//				debug_drawline(self.origin, self.last_sighting, 152);
			}
			else if (right >= center && right > left)
			{
				if (right < 1)
				{
					VectorSet(v, d2 * right * 0.5, 16, 0);
					G_ProjectSource(self->r.currentOrigin, v, v_forward, v_right, right_target);
					//					gi.dprintf("incomplete path, go part way and adjust again\n");
				}
				VectorCopy(self->monsterinfo.last_sighting, self->monsterinfo.saved_goal);
				self->monsterinfo.aiflags |= AI_PURSUE_TEMP;
				VectorCopy(right_target, self->goalentity->r.currentOrigin);
				VectorCopy(right_target, self->monsterinfo.last_sighting);
				VectorSubtract(self->goalentity->r.currentOrigin, self->r.currentOrigin, v);
				self->s.angles[YAW] = self->ideal_yaw = vectoyaw(v);
				//				gi.dprintf("adjusted right\n");
				//				debug_drawline(self.origin, self.last_sighting, 152);
			}
		}
		//		else gi.dprintf("course was fine\n");
	}

	M_MoveToGoal(self, dist);

	G_FreeEntity(tempgoal);

	if (self)
		self->goalentity = save;
}
