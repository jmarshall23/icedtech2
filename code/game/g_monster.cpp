// g_monster.c
//

#include "g_local.h"

float g_damage = 0.0f;

//
// monster weapons
//

//FIXME mosnters should call these with a totally accurate direction
// and we can mess it up based on skill.  Spread should be for normal
// and we can tighten or loosen based on skill.  We could muck with
// the damages too, but I'm not sure that's such a good idea.
void monster_fire_bullet(gentity_t* self, vec3_t start, vec3_t dir, int damage, int kick, int hspread, int vspread, int flashtype)
{
	Bullet_Fire(self, vspread, damage);
}

//void monster_fire_shotgun(gentity_t* self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int count, int flashtype)
//{
//	ShotgunPattern(start, aimdir, 50, self);
//}

void monster_fire_blaster(gentity_t* self, vec3_t start, vec3_t dir, int damage, int speed, int flashtype, int effect)
{

}

void monster_fire_grenade(gentity_t* self, vec3_t start, vec3_t aimdir, int damage, int speed, int flashtype)
{
	
}

void monster_fire_rocket(gentity_t* self, vec3_t start, vec3_t dir, int damage, int speed, int flashtype)
{
	fire_rocket(self, start, dir);
}

void monster_fire_railgun(gentity_t* self, vec3_t start, vec3_t aimdir, int damage, int kick, int flashtype)
{

}

void monster_fire_bfg(gentity_t* self, vec3_t start, vec3_t aimdir, int damage, int speed, int kick, float damage_radius, int flashtype)
{

}

void OgreFireGrenade(gentity_t* self) {
	vec3_t delta;
	VectorSubtract(self->enemy->r.currentOrigin, self->r.currentOrigin, delta);
	VectorNormalize(delta);

	fire_grenade(self, self->r.currentOrigin, delta);
}

void OrgeChainsaw(gentity_t* self, float side) {

	vec3_t	delta;
	float 	ldmg;

	if (!self->enemy)
		return;
	if (!CanDamage(self->enemy, self->s.origin))
		return;

	

	if (VectorLength(delta) > 100)
		return;

	ldmg = (random() + random() + random()) * 4;
	G_Damage(self->enemy, self, self, delta, self->enemy->r.currentOrigin,  ldmg, 0, 0);

}

//
// Monster utility functions
//

static void M_FliesOff(gentity_t* self)
{
	//self->s.effects &= ~EF_FLIES;
	//self->s.sound = 0;
}

static void M_FliesOn(gentity_t* self)
{
	if (self->waterlevel)
		return;
	//self->s.effects |= EF_FLIES;
	//self->s.sound = gi.soundindex("infantry/inflies1.wav");
	self->think = M_FliesOff;
	self->nextthink = level.time + 60;
}

void M_FlyCheck(gentity_t* self)
{
	if (self->waterlevel)
		return;

	if (random() > 0.5)
		return;

	self->think = M_FliesOn;
	self->nextthink = level.time + 5 + 10 * random();
}

void AttackFinished(gentity_t* self, float time)
{
	self->monsterinfo.attack_finished = level.time + time;
}


void M_CheckGround(gentity_t* ent)
{
	vec3_t		point;
	trace_t		trace;

	if (ent->flags & (FL_SWIM | FL_FLY))
		return;

	if (ent->velocity[2] > 100)
	{
		ent->groundentity = NULL;
		return;
	}

	// if the hull point one-quarter unit down is solid the entity is on ground
	point[0] = ent->r.currentOrigin[0];
	point[1] = ent->r.currentOrigin[1];
	point[2] = ent->r.currentOrigin[2] - 0.25;

	engine->SV_Trace(&trace, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, point, ent - g_entities, MASK_MONSTERSOLID);

	// check steepness
	if (trace.plane.normal[2] < 0.7 && !trace.startsolid)
	{
		ent->groundentity = NULL;
		return;
	}

	//	ent->groundentity = trace.ent;
	//	ent->groundentity_linkcount = trace.ent->linkcount;
	//	if (!trace.startsolid && !trace.allsolid)
	//		VectorCopy (trace.endpos, ent->r.currentOrigin);
	if (!trace.startsolid && !trace.allsolid)
	{
		VectorCopy(trace.endpos, ent->r.currentOrigin);
		ent->groundentity = &g_entities[trace.entityNum];
		//ent->groundentity_linkcount = trace.ent->linkcount;
		ent->velocity[2] = 0;
	}
}


void M_CatagorizePosition(gentity_t* ent)
{
	vec3_t		point;
	int			cont;

	//
	// get waterlevel
	//
	point[0] = ent->r.currentOrigin[0];
	point[1] = ent->r.currentOrigin[1];
	point[2] = ent->r.currentOrigin[2] + ent->r.mins[2] + 1;
	cont = engine->SV_PointContents(point, -1);

	if (!(cont & MASK_WATER))
	{
		ent->waterlevel = 0;
		ent->watertype = 0;
		return;
	}

	ent->watertype = cont;
	ent->waterlevel = 1;
	point[2] += 26;
	cont = engine->SV_PointContents(point, -1);
	if (!(cont & MASK_WATER))
		return;

	ent->waterlevel = 2;
	point[2] += 22;
	cont = engine->SV_PointContents(point, -1);
	if (cont & MASK_WATER)
		ent->waterlevel = 3;
}


void M_WorldEffects(gentity_t* ent)
{
	int		dmg;

	if (ent->health > 0)
	{
		if (!(ent->flags & FL_SWIM))
		{
			if (ent->waterlevel < 3)
			{
				ent->air_finished = level.time + 12;
			}
			else if (ent->air_finished < level.time)
			{	// drown!
				if (ent->pain_debounce_time < level.time)
				{
					dmg = 2 + 2 * floor(level.time - ent->air_finished);
					if (dmg > 15)
						dmg = 15;
					//T_Damage(ent, world, world, vec3_origin, ent->r.currentOrigin, vec3_origin, dmg, 0, DAMAGE_NO_ARMOR, MOD_WATER);
					G_Damage(ent, NULL, NULL, vec3_origin, vec3_origin, dmg, 0, MOD_WATER);
					ent->pain_debounce_time = level.time + 1;
				}
			}
		}
		else
		{
			if (ent->waterlevel > 0)
			{
				ent->air_finished = level.time + 9;
			}
			else if (ent->air_finished < level.time)
			{	// suffocate!
				if (ent->pain_debounce_time < level.time)
				{
					dmg = 2 + 2 * floor(level.time - ent->air_finished);
					if (dmg > 15)
						dmg = 15;					
					G_Damage(ent, NULL, NULL, vec3_origin, vec3_origin, dmg, 0, MOD_WATER);
					ent->pain_debounce_time = level.time + 1;
				}
			}
		}
	}

	if (ent->waterlevel == 0)
	{
		if (ent->flags & FL_INWATER)
		{
		//	gi.sound(ent, CHAN_BODY, gi.soundindex("player/watr_out.wav"), 1, ATTN_NORM, 0);
			ent->flags &= ~FL_INWATER;
		}
		return;
	}

	if ((ent->watertype & CONTENTS_LAVA) && !(ent->flags & FL_IMMUNE_LAVA))
	{
		if (ent->damage_debounce_time < level.time)
		{
			ent->damage_debounce_time = level.time + 0.2;
			G_Damage(ent, NULL, NULL, vec3_origin, vec3_origin, 10 * ent->waterlevel, 0, MOD_LAVA);
		}
	}
	if ((ent->watertype & CONTENTS_SLIME) && !(ent->flags & FL_IMMUNE_SLIME))
	{
		if (ent->damage_debounce_time < level.time)
		{
			ent->damage_debounce_time = level.time + 1;			
			G_Damage(ent, NULL, NULL, vec3_origin, vec3_origin, 4 * ent->waterlevel, 0, MOD_SLIME);
		}
	}

	if (!(ent->flags & FL_INWATER))
	{
		//if (!(ent->svflags & SVF_DEADMONSTER))
		//{
		//	if (ent->watertype & CONTENTS_LAVA)
		//		if (random() <= 0.5)
		//			gi.sound(ent, CHAN_BODY, gi.soundindex("player/lava1.wav"), 1, ATTN_NORM, 0);
		//		else
		//			gi.sound(ent, CHAN_BODY, gi.soundindex("player/lava2.wav"), 1, ATTN_NORM, 0);
		//	else if (ent->watertype & CONTENTS_SLIME)
		//		gi.sound(ent, CHAN_BODY, gi.soundindex("player/watr_in.wav"), 1, ATTN_NORM, 0);
		//	else if (ent->watertype & CONTENTS_WATER)
		//		gi.sound(ent, CHAN_BODY, gi.soundindex("player/watr_in.wav"), 1, ATTN_NORM, 0);
		//}

		ent->flags |= FL_INWATER;
		ent->damage_debounce_time = 0;
	}
}


void M_droptofloor(gentity_t* ent)
{
	vec3_t		end;
	trace_t		trace;

	ent->r.currentOrigin[2] += 1;
	VectorCopy(ent->r.currentOrigin, end);
	end[2] -= 256;

	engine->SV_Trace(&trace, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, end, ent - g_entities, MASK_MONSTERSOLID);

	if (trace.fraction == 1 || trace.allsolid)
		return;

	VectorCopy(trace.endpos, ent->r.currentOrigin);

	engine->SV_LinkEntity(ent);
	M_CheckGround(ent);
	M_CatagorizePosition(ent);
}


void M_SetEffects(gentity_t* ent)
{
	//ent->s.effects &= ~(EF_COLOR_SHELL | EF_POWERSCREEN);
	//ent->s.renderfx &= ~(RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE);
	//
	//if (ent->monsterinfo.aiflags & AI_RESURRECTING)
	//{
	//	ent->s.effects |= EF_COLOR_SHELL;
	//	ent->s.renderfx |= RF_SHELL_RED;
	//}
	//
	//if (ent->health <= 0)
	//	return;
	//
	//if (ent->powerarmor_time > level.time)
	//{
	//	if (ent->monsterinfo.power_armor_type == POWER_ARMOR_SCREEN)
	//	{
	//		ent->s.effects |= EF_POWERSCREEN;
	//	}
	//	else if (ent->monsterinfo.power_armor_type == POWER_ARMOR_SHIELD)
	//	{
	//		ent->s.effects |= EF_COLOR_SHELL;
	//		ent->s.renderfx |= RF_SHELL_GREEN;
	//	}
	//}
}


void M_MoveFrame(gentity_t* self)
{
	mmove_t* move;
	int		index;

	move = self->monsterinfo.currentmove;
	self->nextthink = level.time + FRAMETIME;

	if ((self->monsterinfo.nextframe) && (self->monsterinfo.nextframe >= move->firstframe) && (self->monsterinfo.nextframe <= move->lastframe))
	{
		self->monsterinfo.framenum = self->monsterinfo.nextframe;
		self->monsterinfo.nextframe = 0;
	}
	else
	{
		if (self->monsterinfo.framenum == move->lastframe)
		{
			if (move->endfunc)
			{
				move->endfunc(self);

				// regrab move, endfunc is very likely to change it
				move = self->monsterinfo.currentmove;

				// check for death
				if (self->svflags & SVF_DEADMONSTER)
					return;
			}
		}

		if (self->monsterinfo.framenum < move->firstframe || self->monsterinfo.framenum > move->lastframe)
		{
			self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
			self->monsterinfo.framenum = move->firstframe;
		}
		else
		{
			if (!(self->monsterinfo.aiflags & AI_HOLD_FRAME))
			{
				self->monsterinfo.framenum++;
				if (self->monsterinfo.framenum > move->lastframe)
					self->monsterinfo.framenum = move->firstframe;
			}
		}
	}

	index = self->monsterinfo.framenum - move->firstframe;
	if (move->frame[index].aifunc)
		if (!(self->monsterinfo.aiflags & AI_HOLD_FRAME))
			move->frame[index].aifunc(self, move->frame[index].dist);
		else
			move->frame[index].aifunc(self, 0);

	if (move->frame[index].thinkfunc)
		move->frame[index].thinkfunc(self);

	G_SetOrigin(self, self->r.currentOrigin);
	G_SetAngle(self, self->s.angles);
	G_TouchTriggers(self);
}


void monster_think(gentity_t* self)
{
	M_MoveFrame(self);
	M_CheckGround(self);
	M_CatagorizePosition(self);
	M_WorldEffects(self);
	M_SetEffects(self);
}


/*
================
monster_use

Using a monster makes it angry at the current activator
================
*/
void monster_use(gentity_t* self, gentity_t* other, gentity_t* activator)
{
	if (self->enemy)
		return;
	if (self->health <= 0)
		return;
	if (activator->flags & FL_NOTARGET)
		return;
	if (!(activator->client) && !(activator->monsterinfo.aiflags & AI_GOOD_GUY))
		return;

	// delay reaction so if the monster is teleported, its sound is still heard
	self->enemy = activator;
	FoundTarget(self);
}


void monster_start_go(gentity_t* self);


void monster_triggered_spawn(gentity_t* self)
{
	self->r.currentOrigin[2] += 1;
	//KillBox(self);

	//self->solid = SOLID_BBOX;
	self->movetype = MOVETYPE_STEP;
	self->svflags &= ~SVF_NOCLIENT;
	self->air_finished = level.time + 12;
	//gi.linkentity(self);
	engine->SV_LinkEntity(self);

	monster_start_go(self);

	if (self->enemy && !(self->spawnflags & 1) && !(self->enemy->flags & FL_NOTARGET))
	{
		FoundTarget(self);
	}
	else
	{
		self->enemy = NULL;
	}
}

void monster_triggered_spawn_use(gentity_t* self, gentity_t* other, gentity_t* activator)
{
	// we have a one frame delay here so we don't telefrag the guy who activated us
	self->think = monster_triggered_spawn;
	self->nextthink = level.time + FRAMETIME;
	if (activator->client)
		self->enemy = activator;
	self->use = monster_use;
}

void monster_triggered_start(gentity_t* self)
{
	//self->solid = SOLID_NOT;
	self->movetype = MOVETYPE_NONE;
	self->svflags |= SVF_NOCLIENT;
	self->nextthink = 0;
	self->use = monster_triggered_spawn_use;
}


/*
================
monster_death_use

When a monster dies, it fires all of its targets with the current
enemy as activator.
================
*/
void monster_death_use(gentity_t* self)
{
	self->flags &= ~(FL_FLY | FL_SWIM);
	self->monsterinfo.aiflags &= AI_GOOD_GUY;
// backpacks
	//if (self->item)
	//{
	//	Drop_Item(self, self->item, self->angle);
	//	self->item = NULL;
	//}
// jmarshall end

	if (self->deathtarget)
		self->target = self->deathtarget->targetname;

	if (!self->target)
		return;

	G_UseTargets(self, self->enemy);
}

//============================================================================

void monster_noattack(gentity_t* self) {
	
}

void monster_die(gentity_t* self, gentity_t* inflictor, gentity_t* attacker, int damage, int mod) {
	if (self->deadflag != DEAD_NO)
		return;

	self->monsterinfo.die(self);
	self->deadflag = DEAD_DEAD;
	self->svflags |= SVF_DEADMONSTER;

	if (self->isGibbed) {
		G_FreeEntity(self);
	}
}

void monster_pain(gentity_t* self, gentity_t* attacker, int damage) {
	if (self->deadflag == DEAD_DEAD)
		return;

	if (self->health < 0) {
		monster_die(self, attacker, attacker, damage, 0);
		return;
	}

	if (self->enemy == NULL) {
		self->enemy = attacker;
		FoundTarget(self);
	}

	g_damage = damage;

	self->monsterinfo.pain(self);
}


//============================================================================

qboolean monster_start(gentity_t* self)
{
	//if (deathmatch->value)
	//{
	//	G_FreeEdict(self);
	//	return false;
	//}

	if (self->monsterinfo.attack == NULL)
		self->monsterinfo.attack = monster_noattack;

	self->pain = monster_pain;
	self->die = monster_die;

	if ((self->spawnflags & 4) && !(self->monsterinfo.aiflags & AI_GOOD_GUY))
	{
		self->spawnflags &= ~4;
		self->spawnflags |= 1;
		//		gi.dprintf("fixed spawnflags on %s at %s\n", self->classname, vtos(self->r.currentOrigin));
	}

	if (!(self->monsterinfo.aiflags & AI_GOOD_GUY))
		level.total_monsters++;

	self->nextthink = level.time + FRAMETIME;
	self->svflags |= SVF_MONSTER;
	//self->s.renderfx |= RF_FRAMELERP;
	self->takedamage = qtrue; // DAMAGE_AIM;
	self->air_finished = level.time + 12;
	self->use = monster_use;
	self->max_health = self->health;
	self->clipmask = MASK_MONSTERSOLID;
	self->s.eType = ET_MONSTER;

	//self->s.skinnum = 0;
	self->deadflag = DEAD_NO;
	self->svflags &= ~SVF_DEADMONSTER;

	if (!self->monsterinfo.checkattack)
		self->monsterinfo.checkattack = (float (__cdecl *)(gentity_t *))M_CheckAttack;
	VectorCopy(self->r.currentOrigin, self->old_origin);

	//if (st.item)
	//{
	//	self->item = FindItemByClassname(st.item);
	//	if (!self->item)
	//		gi.dprintf("%s at %s has bad item: %s\n", self->classname, vtos(self->r.currentOrigin), st.item);
	//}

	// randomize what frame they start on
	if (self->monsterinfo.currentmove)
		self->monsterinfo.framenum = self->monsterinfo.currentmove->firstframe + (rand() % (self->monsterinfo.currentmove->lastframe - self->monsterinfo.currentmove->firstframe + 1));

	return qtrue;
}

void monster_start_go(gentity_t* self)
{
	vec3_t	v;

	if (self->health <= 0) {
		return;
	}

	// check for target to combat_point and change to combattarget
	//if (self->target)
	//{
	//	qboolean	notcombat;
	//	qboolean	fixup;
	//	gentity_t* target;
	//
	//	target = NULL;
	//	notcombat = qfalse;
	//	fixup = qfalse;
	//	while ((target = G_Find(target, FOFS(targetname), self->target)) != NULL)
	//	{
	//		if (strcmp(target->classname, "point_combat") == 0)
	//		{
	//			self->combattarget = self->target;
	//			fixup = qtrue;
	//		}
	//		else
	//		{
	//			notcombat = qtrue;
	//		}
	//	}
	//	if (notcombat && self->combattarget)
	//		Com_Printf("%s at %s has target with mixed types\n", self->classname, vtos(self->r.currentOrigin));
	//	if (fixup)
	//		self->target = NULL;
	//}

	// validate combattarget
	//if (self->combattarget)
	//{
	//	gentity_t* target;
	//
	//	target = NULL;
	//	while ((target = G_Find(target, FOFS(targetname), self->combattarget->targetname)) != NULL)
	//	{
	//		if (strcmp(target->classname, "point_combat") != 0)
	//		{
	//			Com_Printf("%s at (%i %i %i) has a bad combattarget %s : %s at (%i %i %i)\n",
	//				self->classname, (int)self->r.currentOrigin[0], (int)self->r.currentOrigin[1], (int)self->r.currentOrigin[2],
	//				self->combattarget, target->classname, (int)target->r.currentOrigin[0], (int)target->r.currentOrigin[1],
	//				(int)target->r.currentOrigin[2]);
	//		}
	//	}
	//}

	if (self->target)
	{
		self->goalentity = self->movetarget = G_PickTarget(self->target);
		if (!self->movetarget)
		{
			Com_Printf("%s can't find target %s at %s\n", self->classname, self->target, vtos(self->r.currentOrigin));
			self->target = NULL;
			self->monsterinfo.pausetime = 100000000;
			self->monsterinfo.stand(self);
		}
		else if (strcmp(self->movetarget->classname, "path_corner") == 0)
		{
			VectorSubtract(self->goalentity->r.currentOrigin, self->r.currentOrigin, v);
			self->ideal_yaw = self->s.angles[YAW] = vectoyaw(v);
			self->monsterinfo.walk(self);
			self->target = NULL;
		}
		else
		{
			self->goalentity = self->movetarget = NULL;
			self->monsterinfo.pausetime = 100000000;
			self->monsterinfo.stand(self);
		}
	}
	else
	{
		self->monsterinfo.pausetime = 100000000;
		self->monsterinfo.stand(self);
	}

	self->think = monster_think;
	self->nextthink = level.time + FRAMETIME;

	engine->SV_LinkEntity(self);
}


void walkmonster_start_go(gentity_t* self)
{
	if (!(self->spawnflags & 2) && level.time < 1)
	{
		M_droptofloor(self);

		if (self->groundentity)
			if (!M_walkmove(self, 0, 0))
				Com_Printf("%s in solid at %s\n", self->classname, vtos(self->r.currentOrigin));
	}

	if (!self->yaw_speed)
		self->yaw_speed = 20;
	self->viewheight = 25;

	monster_start_go(self);

	if (self->spawnflags & 2)
		monster_triggered_start(self);
}

void walkmonster_start(gentity_t* self)
{
	self->think = walkmonster_start_go;
	monster_start(self);
}


void flymonster_start_go(gentity_t* self)
{
	if (!M_walkmove(self, 0, 0))
		Com_Printf("%s in solid at %s\n", self->classname, vtos(self->r.currentOrigin));

	if (!self->yaw_speed)
		self->yaw_speed = 10;
	self->viewheight = 25;

	monster_start_go(self);

	if (self->spawnflags & 2)
		monster_triggered_start(self);
}


void flymonster_start(gentity_t* self)
{
	self->flags |= FL_FLY;
	self->think = flymonster_start_go;
	monster_start(self);
}


void swimmonster_start_go(gentity_t* self)
{
	if (!self->yaw_speed)
		self->yaw_speed = 10;
	self->viewheight = 10;

	monster_start_go(self);

	if (self->spawnflags & 2)
		monster_triggered_start(self);
}

void swimmonster_start(gentity_t* self)
{
	self->flags |= FL_SWIM;
	self->think = swimmonster_start_go;
	monster_start(self);
}
