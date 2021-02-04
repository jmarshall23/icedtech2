// g_button.c
//

#include "g_local.h"


/*
===============================================================================

BUTTON

===============================================================================
*/

/*
==============
Touch_Button

===============
*/
void Touch_Button(gentity_t* ent, gentity_t* other, trace_t* trace) {
	if (!other->client) {
		return;
	}

	if (ent->moverState == MOVER_POS1) {
		Use_BinaryMover(ent, other, other);
	}
}


/*QUAKED func_button (0 .5 .8) ?
When a button is touched, it moves some distance in the direction of it's angle, triggers all of it's targets, waits some time, then returns to it's original position where it can be triggered again.

"model2"	.md3 model to also draw
"angle"		determines the opening direction
"target"	all entities with a matching targetname will be used
"speed"		override the default 40 speed
"wait"		override the default 1 second wait (-1 = never return)
"lip"		override the default 4 pixel lip remaining at end of move
"health"	if set, the button must be killed instead of touched
"color"		constantLight color
"light"		constantLight radius
*/
void SP_func_button(gentity_t* ent) {
	vec3_t		abs_movedir;
	float		distance;
	vec3_t		size;
	float		lip;

	if (ent->sounds == 0)
	{
		ent->sound1to2 = G_SoundIndex("sound/buttons/airbut1.wav");
	}
	if (ent->sounds == 1)
	{
		ent->sound1to2 = G_SoundIndex("sound/buttons/switch21.wav");
	}
	if (ent->sounds == 2)
	{
		ent->sound1to2 = G_SoundIndex("sound/buttons/switch02.wav");
	}
	if (ent->sounds == 3)
	{
		ent->sound1to2 = G_SoundIndex("sound/buttons/switch04.wav");
	}

	if (!ent->speed) {
		ent->speed = 40;
	}

	if (!ent->wait) {
		ent->wait = 1;
	}
	ent->wait *= 1000;

	// first position
	VectorCopy(ent->s.origin, ent->pos1);

	// calculate second position
	engine->SV_SetBrushModel(ent, ent->model);

	G_SpawnFloat("lip", "4", &lip);

	G_SetMovedir(ent->s.angles, ent->movedir);
	abs_movedir[0] = fabs(ent->movedir[0]);
	abs_movedir[1] = fabs(ent->movedir[1]);
	abs_movedir[2] = fabs(ent->movedir[2]);
	VectorSubtract(ent->r.maxs, ent->r.mins, size);
	distance = abs_movedir[0] * size[0] + abs_movedir[1] * size[1] + abs_movedir[2] * size[2] - lip;
	VectorMA(ent->pos1, distance, ent->movedir, ent->pos2);

	if (ent->health) {
		// shootable button
		ent->takedamage = qtrue;
	}
	else {
		// touchable button
		ent->touch = Touch_Button;
	}

	InitMover(ent);
}
