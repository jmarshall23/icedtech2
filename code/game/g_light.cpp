// g_light.c
//

#include "g_local.h"

void LightUse(gentity_t* self, gentity_t* other, gentity_t* activator) {
	if (self->s.lightRadius == 0)
	{
		self->s.lightRadius = self->light_on;
	}
	else
	{
		self->s.lightRadius = 0;
	}

	engine->SV_LinkEntity(self);
}

void SpawnLight(gentity_t* ent, int style) {
	int offset, i;

// jmarshall - q3map2 does this for some reason.
	if (style > 11)
		style = 0;
// jmarshall end

	if (style) {		
		ent->dl_stylestring = predef_lightstyles[style - 1];    // these are input as 1-20
	}
	else {
		ent->dl_stylestring = "m";                          // default to always on
	}

	// make the initial offset a valid index into the stylestring
	offset = offset % (strlen(ent->dl_stylestring));

	i = (int)(ent->dl_stylestring[offset]) - (int)'a';
	i = i * (1000.0f / 24.0f);

	if (style) {
		ent->light = i;
	}

	if (ent->light <= 0)
	{
		ent->light = 300;
	}

	if ((ent->spawnflags & 1))
	{		
		ent->light_on = ent->light;
		ent->light = 0;
	}

	ent->s.lightStyle = style;
	ent->s.eType = ET_LIGHT;
	ent->s.lightRadius = ent->light;

	ent->use = LightUse;


	ent->s.generic1 = LDAT_QUADRAT;

	if (ent->spawnflags & 1) {
		ent->s.generic1 = LDAT_LINEAR;
	}
	if (ent->spawnflags & 2) {
		ent->s.generic1 = LDAT_NOSCALE;
	}

	ent->s.lightColor[0] = ent->color[0];
	ent->s.lightColor[1] = ent->color[1];
	ent->s.lightColor[2] = ent->color[2];

	// Light color can't be all black!
	if (ent->s.lightColor[0] == 0 && ent->s.lightColor[1] == 0 && ent->s.lightColor[2] == 0) {
		ent->s.lightColor[0] = 1.0f;
		ent->s.lightColor[1] = 1.0f;
		ent->s.lightColor[2] = 1.0f;
	}

	VectorSet(ent->r.mins, -16, -16, -16);
	VectorSet(ent->r.maxs, 16, 16, 16);
	engine->SV_LinkEntity(ent);

	G_SetOrigin(ent, ent->s.origin);
	VectorCopy(ent->s.angles, ent->s.apos.trBase);
}

void SP_light(gentity_t* ent) {
	SpawnLight(ent, ent->style);
}

void SP_light_fluorospark(gentity_t* ent) {
	int style = ent->style;	
	if (style == 0)
		style = 10;

	SpawnLight(ent, style);
}

void SP_light_fluoro(gentity_t* ent) {	
	SpawnLight(ent, ent->style);
}