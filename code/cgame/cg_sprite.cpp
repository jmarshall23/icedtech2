// cg_sprite.c
//

#include "cg_local.h"

polyVert_t spriteVerts[] = {
	{   0.0,-1.0, -1.0,  0.0, 0.0, 1, 1, 1 , 1},
	{   0.0,1.0,  -1.0,  1.0, 0.0, 1, 1, 1 , 1},
	{   0.0,-1.0,  1.0,  0.0, 1.0, 1, 1, 1 , 1},

	{   0.0,-1.0, 1.0,   0.0, 1.0, 1, 1, 1 , 1},
	{   0.0,1.0, -1.0,   1.0, 0.0, 1, 1, 1 , 1},
	{   0.0,1.0,  1.0,   1.0, 1.0, 1, 1, 1 , 1},
};

/*
===============
CG_InitExplosionSprite
===============
*/
void CG_InitExplosionSprite(void) {
	for (int i = 0; i < 3; i++) {
		qhandle_t shader;
		shader = engine->renderer->RegisterShader(va("sprites/explosion/explode_%d", i));
		cgs.media.explodeModel[i] = engine->renderer->RegisterCustomModel(va("_explode_%d", i), shader, &spriteVerts[0], 6);
	}
}

/*
===============
CG_InitBloodPuffSprite
===============
*/
void CG_InitBloodPuffSprite(void) {
	qhandle_t shader;
	shader = engine->renderer->RegisterShader("sprites/blood");
	cgs.media.bloodPuffModel = engine->renderer->RegisterCustomModel("_bloodpuff", shader, &spriteVerts[0], 6);
}

/*
===============
CG_InitSprites
===============
*/
void CG_InitSprites(void) {
	//CG_InitPlasmaSprite();
	//CG_InitSmokePuffSprite();
	CG_InitExplosionSprite();
	CG_InitBloodPuffSprite();
}

/*
=================
CG_SpawnSprite
=================
*/
void CG_SpawnSprite(vec3_t origin, float scale, qhandle_t model) {
	refEntity_t	 ent;

	memset(&ent, 0, sizeof(refEntity_t));
	VectorCopy(origin, ent.origin);
	VectorCopy(origin, ent.oldorigin);

	ent.reType = RT_MODEL;
	ent.hModel = model;
	ent.scale = scale;	

	AnglesToAxis(cg.refdefViewAngles, ent.axis);

	engine->renderer->AddRefEntityToScene(&ent);
}

/*
=================
CG_SpawnSprite
=================
*/
void CG_SpawnSpriteEx(refEntity_t *ent, vec3_t origin, float scale, qhandle_t model) {
	VectorCopy(origin, ent->origin);
	VectorCopy(origin, ent->oldorigin);

	ent->reType = RT_MODEL;
	ent->hModel = model;
	ent->scale = scale;
	AnglesToAxis(cg.refdefViewAngles, ent->axis);	
}