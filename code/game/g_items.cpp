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
#include "g_local.h"


/*
=============
SP_health_touch
=============
*/
void SP_health_touch(gentity_t* self, gentity_t* other, trace_t* trace) {
	playerState_t* ps;

	if (other->client == NULL) {
		return;
	}

	ps = &other->client->ps;

	if (ps->stats[STAT_HEALTH] >= ps->stats[STAT_MAX_HEALTH]) {
		return;
	}

	other->health += self->healamount;
	if (other->health >= ps->stats[STAT_MAX_HEALTH])
		other->health = ps->stats[STAT_MAX_HEALTH];

	ps->stats[STAT_HEALTH] = other->health;
	G_Printf("You receive %d health\n", self->healamount);

	G_PlaySound(self, self->noise1);

	G_FreeEntity(self);
}

/*
=============
SP_health
=============
*/
void SP_health(gentity_t* self) {
	if (self->spawnflags & 1) {
		self->s.modelindex = G_ModelIndex("models/items/b_bh10.md3");
		self->noise1 = G_SoundIndex("sound/items/r_item1.wav");
		self->healamount = 15;
		self->healtype = 0;
	}
	else if (self->spawnflags & 2) {
		self->s.modelindex = G_ModelIndex("models/items/b_bh100.md3");
		self->noise1 = G_SoundIndex("sound/items/r_item2.wav");
		self->healamount = 100;
		self->healtype = 2;
	}
	else {
		self->s.modelindex = G_ModelIndex("models/items/b_bh25.md3");
		self->noise1 = G_SoundIndex("sound/items/health1.wav");
		self->healamount = 25;
		self->healtype = 1;
	}

	self->r.contents = CONTENTS_TRIGGER;
	self->touch = SP_health_touch;

	VectorSet(self->r.mins, 0, 0, 0);
	VectorSet(self->r.maxs, 32, 32, 56);

	engine->SV_LinkEntity(self);

	G_SetOrigin(self, self->s.origin);
	VectorCopy(self->s.angles, self->s.apos.trBase);
}

/*
=============
SP_item_armor1
=============
*/
void SP_armor_touch(gentity_t* self, gentity_t* other, trace_t* trace) {
	playerState_t* ps;

	if (other->client == NULL) {
		return;
	}

	float type = 0.0;
	float value = 0.0;

	ps = &other->client->ps;

	if (ps->stats[STAT_ARMOR] >= 200) {
		return;
	}

	switch (self->armortype) {
	case 0:
		type = 0.3;
		value = 100;
		break;
	case 1:
		type = 0.6;
		value = 150;
		break;
	case 2:
		type = 0.8;
		value = 200;
		break;
		default:
			G_Error("SP_armor_touch: Unknown armor type!\n");
	}

	if (ps->stats[STAT_ARMOR] >= type * value)
		return;

	ps->stats[STAT_ARMOR] = value;

	G_PlaySound(self, self->noise1);

	G_FreeEntity(self);

	G_Printf("You got armor\n");
}

/*
=============
SP_item_armor1
=============
*/
void SP_item_armor1(gentity_t* self) {
	self->touch = SP_armor_touch;
	self->r.contents = CONTENTS_TRIGGER;
	self->s.modelindex = G_ModelIndex("models/items/armor.md3");
	self->noise1 = G_SoundIndex("sound/items/armor1.wav");

	self->s.clientTransformAnim = CLIENT_TRANSFORM_BOB_AND_ROTATE;
	
	self->s.origin[2] += 16;
	VectorSet(self->r.mins, -16, -16, 0);
	VectorSet(self->r.maxs, 16, 16, 56);

	self->armortype = 0;


	engine->SV_LinkEntity(self);

	G_SetOrigin(self, self->s.origin);
	VectorCopy(self->s.angles, self->s.apos.trBase);
}

/*
=============
SP_item_armor2
=============
*/
void SP_item_armor2(gentity_t* self) {
	self->touch = SP_armor_touch;
	self->r.contents = CONTENTS_TRIGGER;
	self->s.modelindex = G_ModelIndex("models/items/armor1.md3");
	self->noise1 = G_SoundIndex("sound/items/armor1.wav");

	self->s.clientTransformAnim = CLIENT_TRANSFORM_BOB_AND_ROTATE;

	self->s.origin[2] += 16;
	VectorSet(self->r.mins, -16, -16, 0);
	VectorSet(self->r.maxs, 16, 16, 56);

	self->armortype = 1;


	engine->SV_LinkEntity(self);

	G_SetOrigin(self, self->s.origin);
	VectorCopy(self->s.angles, self->s.apos.trBase);
}

/*
=============
SP_item_armorinv
=============
*/
void SP_item_armorinv(gentity_t* self) {
	self->touch = SP_armor_touch;
	self->r.contents = CONTENTS_TRIGGER;
	self->s.modelindex = G_ModelIndex("models/items/armor2.md3");
	self->noise1 = G_SoundIndex("sound/items/armor1.wav");

	self->s.clientTransformAnim = CLIENT_TRANSFORM_BOB_AND_ROTATE;

	self->s.origin[2] += 16;
	VectorSet(self->r.mins, -16, -16, 0);
	VectorSet(self->r.maxs, 16, 16, 56);

	self->armortype = 2;


	engine->SV_LinkEntity(self);

	G_SetOrigin(self, self->s.origin);
	VectorCopy(self->s.angles, self->s.apos.trBase);
}

/*
=============
SP_ammo_touch
=============
*/
void SP_ammo_touch(gentity_t* self, gentity_t* other, trace_t* trace) {
	playerState_t* ps;

	if (other->client == NULL) {
		return;
	}

	if (other->health <= 0) {
		return;
	}

	ps = &other->client->ps;

	switch (self->itemWeapon) {
		case WP_SHOTGUN:
			if (ps->ammo[WP_SHOTGUN] >= 100) {
				return;
			}
			ps->ammo[WP_SHOTGUN] = min(ps->ammo[WP_SHOTGUN] + self->itemAmmoWeaponFlag, 100);
			break;
		case WP_NAILGUN:
			if (ps->ammo[WP_NAILGUN] >= 200) {
				return;
			}
			ps->ammo[WP_NAILGUN] = min(ps->ammo[WP_NAILGUN] + self->itemAmmoWeaponFlag, 200);
			break;
		case WP_ROCKET_LAUNCHER:
			if (ps->ammo[WP_ROCKET_LAUNCHER] >= 100) {
				return;
			}
			ps->ammo[WP_ROCKET_LAUNCHER] = min(ps->ammo[WP_ROCKET_LAUNCHER] + self->itemAmmoWeaponFlag, 100);
			break;
		case WP_LIGHTNING:
			if (ps->ammo[WP_LIGHTNING] >= 200) {
				return;
			}
			ps->ammo[WP_LIGHTNING] = min(ps->ammo[WP_LIGHTNING] + self->itemAmmoWeaponFlag, 100);
			break;

		default:
			G_Error("SP_ammo_touch: Unknown item type!\n");
			break;
	}

	G_Printf("You got the %s\n", self->netname);
	G_PlaySound(self, self->noise1);
	G_FreeEntity(self);
}

/*
=============
SP_item_shells
=============
*/
void SP_item_shells(gentity_t* self) {
	self->netname = "shells";
	self->touch = SP_ammo_touch;
	self->r.contents = CONTENTS_TRIGGER;

	if (self->spawnflags & 1)
	{
		self->s.modelindex = G_ModelIndex("models/items/b_shell1.md3");
		self->itemAmmoWeaponFlag = 40;
	}
	else
	{
		self->s.modelindex = G_ModelIndex("models/items/b_shell0.md3");
		self->itemAmmoWeaponFlag = 20;
	}

	self->noise1 = G_SoundIndex("sound/weapons/lock4.wav");

	self->s.origin[2] += 16;
	VectorSet(self->r.mins, 0, 0, 0);
	VectorSet(self->r.maxs, 32, 32, 56);

	self->itemWeapon = WP_SHOTGUN;
	
	engine->SV_LinkEntity(self);

	G_SetOrigin(self, self->s.origin);
	VectorCopy(self->s.angles, self->s.apos.trBase);

	G_SetToGround(self);
}

/*
=============
SP_item_nails
=============
*/
void SP_item_nails(gentity_t* self) {
	self->netname = "nails";
	self->touch = SP_ammo_touch;
	self->r.contents = CONTENTS_TRIGGER;

	if (self->spawnflags & 1)
	{
		self->s.modelindex = G_ModelIndex("models/items/b_nail1.md3");
		self->itemAmmoWeaponFlag = 50;
	}
	else
	{
		self->s.modelindex = G_ModelIndex("models/items/b_nail0.md3");
		self->itemAmmoWeaponFlag = 25;
	}

	self->noise1 = G_SoundIndex("sound/weapons/lock4.wav");

	self->s.origin[2] += 16;
	VectorSet(self->r.mins, 0, 0, 0);
	VectorSet(self->r.maxs, 32, 32, 56);

	self->itemWeapon = WP_NAILGUN;

	engine->SV_LinkEntity(self);

	G_SetOrigin(self, self->s.origin);
	VectorCopy(self->s.angles, self->s.apos.trBase);

	G_SetToGround(self);
}

/*
================
weapon_touch
================
*/
void weapon_touch(gentity_t* self, gentity_t* other, trace_t* trace) {
	if (other->client == NULL) {
		return;
	}

	// Give the new weapon to the player.
	other->client->ps.stats[STAT_WEAPONS] |= (1 << self->itemWeapon);
 	other->client->ps.ammo[self->itemWeapon] += self->itemAmmoWeaponFlag;

	G_ClientSwitchWeapon(other, self->itemWeapon);	

	G_Printf("You got the %s\n", self->netname);
	G_PlaySound(self, self->noise1);
	G_FreeEntity(self);
}

/*
================
SP_weapon_supershotgun
================
*/
void SP_weapon_supershotgun(gentity_t* self) {
	self->s.clientTransformAnim = CLIENT_TRANSFORM_BOB_AND_ROTATE;
	self->touch = weapon_touch;
	self->itemAmmoWeaponFlag = 5;
	self->s.modelindex = G_ModelIndex("models/weapons2/g_shot.md3");
	self->itemWeapon = WP_SUPER_SHOTGUN;
	self->netname = "Double-barrelled Shotgun";
	VectorSet(self->r.mins, -16, -16, 0);
	VectorSet(self->r.maxs, 16, 16, 56);
	self->r.contents = CONTENTS_TRIGGER;
	self->noise1 = G_SoundIndex("sound/weapons/pkup.wav");
	engine->SV_LinkEntity(self);

	G_SetOrigin(self, self->s.origin);
	VectorCopy(self->s.angles, self->s.apos.trBase);
}

/*
=============
SP_weapon_nailgun
=============
*/
void SP_weapon_nailgun(gentity_t* self) {
	self->s.clientTransformAnim = CLIENT_TRANSFORM_BOB_AND_ROTATE;
	self->touch = weapon_touch;
	self->itemAmmoWeaponFlag = 30;
	self->s.modelindex = G_ModelIndex("models/weapons2/g_nail.md3");
	self->itemWeapon = WP_NAILGUN;
	self->netname = "nailgun";
	VectorSet(self->r.mins, -16, -16, 0);
	VectorSet(self->r.maxs, 16, 16, 56);
	self->r.contents = CONTENTS_TRIGGER;
	self->noise1 = G_SoundIndex("sound/weapons/pkup.wav");
	engine->SV_LinkEntity(self);

	G_SetOrigin(self, self->s.origin);
	VectorCopy(self->s.angles, self->s.apos.trBase);
}

/*
=============
SP_weapon_supernailgun
=============
*/
void SP_weapon_supernailgun(gentity_t* self) {
	self->s.clientTransformAnim = CLIENT_TRANSFORM_BOB_AND_ROTATE;
	self->touch = weapon_touch;
	self->itemAmmoWeaponFlag = 30;
	self->s.modelindex = G_ModelIndex("models/weapons2/g_nail2.md3");
	self->itemWeapon = WP_SUPER_NAILGUN;
	self->netname = "Super Nailgun";
	VectorSet(self->r.mins, -16, -16, 0);
	VectorSet(self->r.maxs, 16, 16, 56);
	self->r.contents = CONTENTS_TRIGGER;
	self->noise1 = G_SoundIndex("sound/weapons/pkup.wav");
	engine->SV_LinkEntity(self);

	G_SetOrigin(self, self->s.origin);
	VectorCopy(self->s.angles, self->s.apos.trBase);
}

/*
=============
SP_weapon_grenadelauncher
=============
*/
void SP_weapon_grenadelauncher(gentity_t* self) {
	self->s.clientTransformAnim = CLIENT_TRANSFORM_BOB_AND_ROTATE;
	self->itemAmmoWeaponFlag = 5;
	self->touch = weapon_touch;
	self->s.modelindex = G_ModelIndex("models/weapons2/g_rock.md3");
	self->itemWeapon = WP_GRENADE_LAUNCHER;
	self->netname = "Grenade Launcher";
	VectorSet(self->r.mins, -16, -16, 0);
	VectorSet(self->r.maxs, 16, 16, 56);
	self->r.contents = CONTENTS_TRIGGER;
	self->noise1 = G_SoundIndex("sound/weapons/pkup.wav");
	engine->SV_LinkEntity(self);

	G_SetOrigin(self, self->s.origin);
	VectorCopy(self->s.angles, self->s.apos.trBase);
}

/*
=============
SP_weapon_rocketlauncher
=============
*/
void SP_weapon_rocketlauncher(gentity_t* self) {
	self->s.clientTransformAnim = CLIENT_TRANSFORM_BOB_AND_ROTATE;
	self->itemAmmoWeaponFlag = 5;
	self->touch = weapon_touch;
	self->s.modelindex = G_ModelIndex("models/weapons2/g_rock2.md3");
	self->itemWeapon = WP_ROCKET_LAUNCHER;
	self->netname = "Rocket Launcher";
	VectorSet(self->r.mins, -16, -16, 0);
	VectorSet(self->r.maxs, 16, 16, 56);
	self->r.contents = CONTENTS_TRIGGER;
	self->noise1 = G_SoundIndex("sound/weapons/pkup.wav");
	engine->SV_LinkEntity(self);

	G_SetOrigin(self, self->s.origin);
	VectorCopy(self->s.angles, self->s.apos.trBase);
}

/*
=============
SP_weapon_lightning
=============
*/
void SP_weapon_lightning(gentity_t* self) {
	self->s.clientTransformAnim = CLIENT_TRANSFORM_BOB_AND_ROTATE;
	self->itemAmmoWeaponFlag = 15;
	self->touch = weapon_touch;
	self->s.modelindex = G_ModelIndex("models/weapons2/g_light.md3");
	self->itemWeapon = WP_LIGHTNING;
	self->netname = "Thunderbolt";
	VectorSet(self->r.mins, -16, -16, 0);
	VectorSet(self->r.maxs, 16, 16, 56);
	self->r.contents = CONTENTS_TRIGGER;
	self->noise1 = G_SoundIndex("sound/weapons/pkup.wav");
	engine->SV_LinkEntity(self);

	G_SetOrigin(self, self->s.origin);
	VectorCopy(self->s.angles, self->s.apos.trBase);
}


/*
===============================================================================

KEYS

===============================================================================
*/

void key_touch(gentity_t* self, gentity_t* other, trace_t* trace)
{
	gentity_t	*stemp;
	float		best;

	if (other->client == NULL)
		return;

	if (other->health <= 0)
		return;

	if (other->client->keys[self->keyType])
		return;

	G_Printf("You got the %s\n", self->netname);
	G_PlaySound(self, self->noise1);

	//sound(other, CHAN_ITEM, self.noise, 1, ATTN_NORM);
	//stuffcmd(other, "bf\n");
	//other.items = other.items | self.items;
	other->client->keys[self->keyType] = qtrue;

	G_FreeEntity(self);
};


void key_setsounds(gentity_t *self)
{
	if (level.worldtype == 0)
	{
		self->noise1 = G_SoundIndex("sound/misc/medkey.wav");  
	}
	if (level.worldtype == 1)
	{
		self->noise1 = G_SoundIndex("sound/misc/runekey.wav");
	}
	if (level.worldtype == 2)
	{
		self->noise1 = G_SoundIndex("sound/misc/basekey.wav");
	}
};

/*QUAKED item_key1 (0 .5 .8) (-16 -16 -24) (16 16 32)
SILVER key
In order for keys to work
you MUST set your maps
worldtype to one of the
following:
0: medieval
1: metal
2: base
*/

void item_key1(gentity_t *self)
{
	VectorSet(self->r.mins, -16, -16, -24);
	VectorSet(self->r.maxs, 16, 16, 32);
	self->r.contents = CONTENTS_TRIGGER;
	engine->SV_LinkEntity(self);

	self->s.clientTransformAnim = CLIENT_TRANSFORM_BOB_AND_ROTATE;

	G_SetOrigin(self, self->s.origin);
	VectorCopy(self->s.angles, self->s.apos.trBase);

	if (level.worldtype == 0)
	{
		self->s.modelindex = G_ModelIndex("models/keys/w_s_key.md3");
		self->netname = "silver key";
	}
	else if (level.worldtype == 1)
	{
		self->s.modelindex = G_ModelIndex("models/keys/m_s_key.md3");
		self->netname = "silver runekey";
	}
	else if (level.worldtype == 2)
	{
		self->s.modelindex = G_ModelIndex("models/keys/b_s_key.md3");
		self->netname = "silver keycard";
	}
	key_setsounds(self);

	self->touch = key_touch;
	self->keyType = IT_KEY1;
};

/*QUAKED item_key2 (0 .5 .8) (-16 -16 -24) (16 16 32)
GOLD key
In order for keys to work
you MUST set your maps
worldtype to one of the
following:
0: medieval
1: metal
2: base
*/

void item_key2(gentity_t* self)
{
	VectorSet(self->r.mins, -16, -16, -24);
	VectorSet(self->r.maxs, 16, 16, 32);
	self->r.contents = CONTENTS_TRIGGER;
	engine->SV_LinkEntity(self);

	self->s.clientTransformAnim = CLIENT_TRANSFORM_BOB_AND_ROTATE;

	G_SetOrigin(self, self->s.origin);
	VectorCopy(self->s.angles, self->s.apos.trBase);

	if (level.worldtype == 0)
	{
		self->s.modelindex = G_ModelIndex("models/keys/w_g_key.md3");
		self->netname = "gold key";
	}
	else if (level.worldtype == 1)
	{
		self->s.modelindex = G_ModelIndex("models/keys/m_g_key.md3");
		self->netname = "gold runekey";
	}
	else if (level.worldtype == 2)
	{
		self->s.modelindex = G_ModelIndex("models/keys/b_g_key.md3");
		self->netname = "gold keycard";
	}
	key_setsounds(self);

	self->touch = key_touch;
	self->keyType = IT_KEY2;
};

