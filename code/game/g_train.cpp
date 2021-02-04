// g_train.c
//

#include "g_local.h"


/*
===============================================================================

TRAIN

===============================================================================
*/


#define TRAIN_START_ON		1
#define TRAIN_TOGGLE		2
#define TRAIN_BLOCK_STOPS	4

/*
===============
Think_BeginMoving

The wait time at a corner has completed, so start moving again
===============
*/
void Think_BeginMoving(gentity_t* ent) {
	ent->s.pos.trTime = level.time;
	ent->s.pos.trType = TR_LINEAR_STOP;
}

/*
===============
Reached_Train
===============
*/
void Reached_Train(gentity_t* ent) {
	gentity_t* next;
	float			speed;
	vec3_t			move;
	float			length;

	// copy the apropriate values
	next = ent->nextTrain;
	if (!next || !next->nextTrain) {
		return;		// just stop
	}

	// fire all other targets
	G_UseTargets(next, NULL);

	// set the new trajectory
	ent->nextTrain = next->nextTrain;
	VectorCopy(next->s.origin, ent->pos1);
	VectorCopy(next->nextTrain->s.origin, ent->pos2);

	// if the path_corner has a speed, use that
	if (next->speed) {
		speed = next->speed;
	}
	else {
		// otherwise use the train's speed
		speed = ent->speed;
	}
	if (speed < 1) {
		speed = 1;
	}

	// calculate duration
	VectorSubtract(ent->pos2, ent->pos1, move);
	length = VectorLength(move);

	ent->s.pos.trDuration = length * 1000 / speed;

	// looping sound
	ent->s.loopSound = next->soundLoop;

	// start it going
	SetMoverState(ent, MOVER_1TO2, level.time);

	// if there is a "wait" value on the target, don't start moving yet
	if (next->wait) {
		ent->nextthink = level.time + next->wait * 1000;
		ent->think = Think_BeginMoving;
		ent->s.pos.trType = TR_STATIONARY;
	}
}


/*
===============
Think_SetupTrainTargets

Link all the corners together
===============
*/
void Think_SetupTrainTargets(gentity_t* ent) {
	gentity_t* path, * next, * start;

	ent->nextTrain = G_Find(NULL, FOFS(targetname), ent->target);
	if (!ent->nextTrain) {
		G_Printf("func_train at %s with an unfound target\n",
			vtos(ent->r.absmin));
		return;
	}

	start = NULL;
	for (path = ent->nextTrain; path != start; path = next) {
		if (!start) {
			start = path;
		}

		if (!path->target) {
			G_Printf("Train corner at %s without a target\n",
				vtos(path->s.origin));
			return;
		}

		if (path->nextTrain != NULL)
			break;

		// find a path_corner among the targets
		// there may also be other targets that get fired when the corner
		// is reached
		next = NULL;
		do {
			next = G_Find(next, FOFS(targetname), path->target);
			if (!next) {
				G_Printf("Train corner at %s without a target path_corner\n",
					vtos(path->s.origin));
				return;
			}
		} while (strcmp(next->classname, "path_corner"));

		path->nextTrain = next;
	}

	// start the train moving from the first corner
	Reached_Train(ent);
}



/*QUAKED path_corner (.5 .3 0) (-8 -8 -8) (8 8 8)
Train path corners.
Target: next path corner and other targets to fire
"speed" speed to move to the next corner
"wait" seconds to wait before behining move to next corner
*/
void SP_path_corner(gentity_t* self) {
	if (!self->targetname) {
		G_Printf("path_corner with no targetname at %s\n", vtos(self->s.origin));
		G_FreeEntity(self);
		return;
	}
	// path corners don't need to be linked in
}



/*QUAKED func_train (0 .5 .8) ? START_ON TOGGLE BLOCK_STOPS
A train is a mover that moves between path_corner target points.
Trains MUST HAVE AN ORIGIN BRUSH.
The train spawns at the first target it is pointing at.
"model2"	.md3 model to also draw
"speed"		default 100
"dmg"		default	2
"noise"		looping sound to play when the train is in motion
"target"	next path corner
"color"		constantLight color
"light"		constantLight radius
*/
void SP_func_train(gentity_t* self) {
	VectorClear(self->s.angles);

	if (self->spawnflags & TRAIN_BLOCK_STOPS) {
		self->damage = 0;
	}
	else {
		if (!self->damage) {
			self->damage = 2;
		}
	}

	if (!self->speed) {
		self->speed = 100;
	}

	if (!self->target) {
		G_Printf("func_train without a target at %s\n", vtos(self->r.absmin));
		G_FreeEntity(self);
		return;
	}

	engine->SV_SetBrushModel(self, self->model);
	InitMover(self);

	self->reached = Reached_Train;

	// start trains on the second frame, to make sure their targets have had
	// a chance to spawn
	self->nextthink = level.time + FRAMETIME;
	self->think = Think_SetupTrainTargets;
}
