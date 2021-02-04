/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
// g_misc.c

#include "g_local.h"

void SP_start_ambient(gentity_t* ent, const char* filename, float volume) {
	ent->s.eType = ET_SPEAKER;
	ent->s.clientNum = 1; // This is auto trigger, id had it this way, we probably should change it. 
	ent->s.eventParm = G_SoundIndex((char *)filename);
	VectorSet(ent->r.mins, -16, -16, -16);
	VectorSet(ent->r.maxs, 16, 16, 16);
	engine->SV_LinkEntity(ent);

	G_SetOrigin(ent, ent->s.origin);
	VectorCopy(ent->s.angles, ent->s.apos.trBase);
}

void ambient_suck_wind(gentity_t* ent) {
	SP_start_ambient(ent, "sound/ambience/suck1.wav", 1.0f);
}

void ambient_drone(gentity_t* ent) {
	SP_start_ambient(ent, "sound/ambience/drone6.wav", 0.5f);
}

void ambient_flouro_buzz(gentity_t* ent) {
	SP_start_ambient(ent, "sound/ambience/buzz1.wav", 0.5f);
}

void ambient_drip(gentity_t* ent) {
	SP_start_ambient(ent, "sound/ambience/drip1.wav", 0.5f);
}

void ambient_comp_hum(gentity_t* ent) {
	SP_start_ambient(ent, "sound/ambience/comp1.wav", 0.5f);
}

void ambient_thunder(gentity_t* ent) {
	SP_start_ambient(ent, "sound/ambience/thunder1.wav", 0.5f);
}

void ambient_light_buzz(gentity_t* ent) {
	SP_start_ambient(ent, "sound/ambience/fl_hum1.wav", 0.5f);
}

void ambient_swamp1(gentity_t* ent) {
	SP_start_ambient(ent, "sound/ambience/swamp1.wav", 0.5f);
}

void ambient_swamp2(gentity_t* ent) {
	SP_start_ambient(ent, "sound/ambience/swamp2.wav", 0.5f);
}

/*QUAKED func_group (0 0 0) ?
Used to group brushes together just for editor convenience.  They are turned into normal brushes by the utilities.
*/


/*QUAKED info_camp (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for calculations in the utilities (spotlights, etc), but removed during gameplay.
*/
void SP_info_camp( gentity_t *self ) {
	G_SetOrigin( self, self->s.origin );
}


/*QUAKED info_null (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for calculations in the utilities (spotlights, etc), but removed during gameplay.
*/
void SP_info_null( gentity_t *self ) {
	G_FreeEntity( self );
}


/*QUAKED info_notnull (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for in-game calculation, like jumppad targets.
target_position does the same thing
*/
void SP_info_notnull( gentity_t *self ){
	G_SetOrigin( self, self->s.origin );
}

/*
=================================================================================

TELEPORTERS

=================================================================================
*/

void TeleportPlayer( gentity_t *player, vec3_t origin, vec3_t angles ) {
	gentity_t	*tent;

	// use temp events at source and destination to prevent the effect
	// from getting dropped by a second player event
	if ( player->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		tent = G_TempEntity( player->client->ps.origin, EV_PLAYER_TELEPORT_OUT );
		tent->s.clientNum = player->s.clientNum;

		tent = G_TempEntity( origin, EV_PLAYER_TELEPORT_IN );
		tent->s.clientNum = player->s.clientNum;
	}

	// unlink to make sure it can't possibly interfere with G_KillBox
	engine->SV_UnlinkEntity (player);

	VectorCopy ( origin, player->client->ps.origin );
	player->client->ps.origin[2] += 1;

	// spit the player out
	AngleVectors( angles, player->client->ps.velocity, NULL, NULL );
	VectorScale( player->client->ps.velocity, 400, player->client->ps.velocity );
	player->client->ps.pm_time = 160;		// hold time
	player->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;

	// toggle the teleport bit so the client knows to not lerp
	player->client->ps.eFlags ^= EF_TELEPORT_BIT;

	// set angles
	SetClientViewAngle( player, angles );

	// kill anything at the destination
	if ( player->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		G_KillBox (player);
	}

	// save results of pmove
	BG_PlayerStateToEntityState( &player->client->ps, &player->s, qtrue );

	// use the precise origin for linking
	VectorCopy( player->client->ps.origin, player->r.currentOrigin );

	if ( player->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		engine->SV_LinkEntity (player);
	}
}


/*QUAKED misc_teleporter_dest (1 0 0) (-32 -32 -24) (32 32 -16)
Point teleporters at these.
Now that we don't have teleport destination pads, this is just
an info_notnull
*/
void SP_misc_teleporter_dest( gentity_t *ent ) {
}

//===========================================================

/*QUAKED misc_model (1 0 0) (-16 -16 -16) (16 16 16)
"model"		arbitrary .md3 file to display
*/
void SP_misc_model( gentity_t *ent ) {
	gameModelCacheEntry_t* cache = G_LoadModel(ent->model);
	if (!cache) {
		G_FreeEntity(ent);
		return;
	}

	ent->s.modelindex = cache->modelIndex;
	ent->r.contents = CONTENTS_SOLID;
	VectorCopy(cache->mins, ent->r.mins);
	VectorCopy(cache->maxs, ent->r.maxs);	

	RotateBounds(ent->r.mins, ent->r.maxs, ent->s.angles);

	engine->SV_LinkEntity (ent);

	G_SetOrigin( ent, ent->s.origin );
	VectorCopy( ent->s.angles, ent->s.apos.trBase );
}

void SP_light_torch_small_walltorch(gentity_t* ent) {
	ent->s.modelindex = G_ModelIndex("models/enviro/flame.md3");
	ent->s.clientAnimate = 1;
	ent->s.lightRadius = 400;
	ent->s.lightColor[0] = 1.0f;
	ent->s.lightColor[1] = 1.0f;
	ent->s.lightColor[2] = 1.0f;
	VectorSet(ent->r.mins, -16, -16, -16);
	VectorSet(ent->r.maxs, 16, 16, 16);
	engine->SV_LinkEntity(ent);

	G_SetOrigin(ent, ent->s.origin);
	VectorCopy(ent->s.angles, ent->s.apos.trBase);
}

void SP_light_flame_large_yellow(gentity_t* ent) {
	ent->s.modelindex = G_ModelIndex("models/enviro/flame2.md3");
	ent->s.clientAnimate = 1;
	ent->s.lightRadius = 400;
	ent->s.lightColor[0] = 1.0f;
	ent->s.lightColor[1] = 1.0f;
	ent->s.lightColor[2] = 1.0f;
	VectorSet(ent->r.mins, -16, -16, -16);
	VectorSet(ent->r.maxs, 16, 16, 16);
	engine->SV_LinkEntity(ent);

	G_SetOrigin(ent, ent->s.origin);
	VectorCopy(ent->s.angles, ent->s.apos.trBase);
}

void SP_light_flame_small_yellow(gentity_t* ent) {
	ent->s.modelindex = G_ModelIndex("models/enviro/flame2.md3");
	ent->s.clientAnimate = 1;
	ent->s.lightRadius = 400;
	ent->s.lightColor[0] = 1.0f;
	ent->s.lightColor[1] = 1.0f;
	ent->s.lightColor[2] = 1.0f;
	VectorSet(ent->r.mins, -16, -16, -16);
	VectorSet(ent->r.maxs, 16, 16, 16);
	engine->SV_LinkEntity(ent);

	G_SetOrigin(ent, ent->s.origin);
	VectorCopy(ent->s.angles, ent->s.apos.trBase);
}


/*
======================================================================

  SHOOTERS

======================================================================
*/

void Use_Shooter( gentity_t *ent, gentity_t *other, gentity_t *activator ) {
	vec3_t		dir;
	float		deg;
	vec3_t		up, right;

	// see if we have a target
	if ( ent->enemy ) {
		VectorSubtract( ent->enemy->r.currentOrigin, ent->s.origin, dir );
		VectorNormalize( dir );
	} else {
		VectorCopy( ent->movedir, dir );
	}

	// randomize a bit
	PerpendicularVector( up, dir );
	CrossProduct( up, dir, right );

	deg = crandom() * ent->random;
	VectorMA( dir, deg, up, dir );

	deg = crandom() * ent->random;
	VectorMA( dir, deg, right, dir );

	VectorNormalize( dir );

	switch ( ent->s.weapon ) {
	case WP_GRENADE_LAUNCHER:
		fire_grenade( ent, ent->s.origin, dir );
		break;
	case WP_ROCKET_LAUNCHER:
		fire_rocket( ent, ent->s.origin, dir );
		break;
	//case WP_PLASMAGUN:
	//	fire_plasma( ent, ent->s.origin, dir );
	//	break;
	}

	G_AddEvent( ent, EV_FIRE_WEAPON, 0 );
}


static void InitShooter_Finish( gentity_t *ent ) {
	ent->enemy = G_PickTarget( ent->target );
	ent->think = 0;
	ent->nextthink = 0;
}

void InitShooter( gentity_t *ent, int weapon ) {
	//ent->use = Use_Shooter;
	//ent->s.weapon = weapon;
	//
	//RegisterItem( BG_FindItemForWeapon( weapon ) );
	//
	//G_SetMovedir( ent->s.angles, ent->movedir );
	//
	//if ( !ent->random ) {
	//	ent->random = 1.0;
	//}
	//ent->random = sin( M_PI * ent->random / 180 );
	//// target might be a moving object, so we can't set movedir for it
	//if ( ent->target ) {
	//	ent->think = InitShooter_Finish;
	//	ent->nextthink = level.time + 500;
	//}
	//engine->SV_LinkEntity( ent );
}

/*QUAKED shooter_rocket (1 0 0) (-16 -16 -16) (16 16 16)
Fires at either the target or the current direction.
"random" the number of degrees of deviance from the taget. (1.0 default)
*/
void SP_shooter_rocket( gentity_t *ent ) {
	InitShooter( ent, WP_ROCKET_LAUNCHER );
}

/*QUAKED shooter_plasma (1 0 0) (-16 -16 -16) (16 16 16)
Fires at either the target or the current direction.
"random" is the number of degrees of deviance from the taget. (1.0 default)
*/
void SP_shooter_plasma( gentity_t *ent ) {
	//InitShooter( ent, WP_PLASMAGUN);
}

/*QUAKED shooter_grenade (1 0 0) (-16 -16 -16) (16 16 16)
Fires at either the target or the current direction.
"random" is the number of degrees of deviance from the taget. (1.0 default)
*/
void SP_shooter_grenade( gentity_t *ent ) {
	InitShooter( ent, WP_GRENADE_LAUNCHER);
}
