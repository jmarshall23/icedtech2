// fx_beam.c
//

#include "cg_local.h"

#define	MAX_BEAMS	24
typedef struct
{
	int		entity;
	qhandle_t model;
	float	endtime;
	vec3_t	start, end;
} beam_t;

beam_t		cl_beams[MAX_BEAMS];

/*
=================
FX_CreateBream
=================
*/
void FX_CreateBream(int ent, vec3_t start, vec3_t end, qhandle_t m)
{
	beam_t* b;
	int		i;

	// override any beam with the same entity
	for (i = 0, b = cl_beams; i < MAX_BEAMS; i++, b++)
		if (b->entity == ent)
		{
			b->entity = ent;
			b->model = m;
			b->endtime = cg.time + 0.2;
			VectorCopy(start, b->start);
			VectorCopy(end, b->end);
			return;
		}

	// find a free beam
	for (i = 0, b = cl_beams; i < MAX_BEAMS; i++, b++)
	{
		if (!b->model || b->endtime < cg.time)
		{
			b->entity = ent;
			b->model = m;
			b->endtime = cg.time + 0.2;
			VectorCopy(start, b->start);
			VectorCopy(end, b->end);
			return;
		}
	}	
}

/*
===================
FX_RenderBeams
===================
*/
void FX_RenderBeams(void) {
	int			i;
	beam_t* b;
	vec3_t		dist, org;
	float		d;
	float		yaw, pitch;
	float		forward;

	// update lightning
	for (i = 0, b = cl_beams; i < MAX_BEAMS; i++, b++)
	{
		if (!b->model || b->endtime < cg.time)
			continue;

		// if coming from the player, update the start position
		if (b->entity == cg.snap->ps.clientNum)
		{
			VectorCopy(cg.refdef.vieworg, b->start);
		}

		// calculate pitch and yaw
		VectorSubtract(b->end, b->start, dist);

		if (dist[1] == 0 && dist[0] == 0)
		{
			yaw = 0;
			if (dist[2] > 0)
				pitch = 90;
			else
				pitch = 270;
		}
		else
		{
			yaw = (int)(atan2(dist[1], dist[0]) * 180 / M_PI);
			if (yaw < 0)
				yaw += 360;

			forward = sqrt(dist[0] * dist[0] + dist[1] * dist[1]);
			pitch = (int)(atan2(dist[2], forward) * 180 / M_PI);
			if (pitch < 0)
				pitch += 360;
		}

		// add new entities for the lightning
		VectorCopy(b->start, org);
		d = VectorNormalize(dist);
		while (d > 0)
		{
			refEntity_t ref;
			vec3_t angles;
			memset(&ref, 0, sizeof(refEntity_t));
			ref.reType = RT_MODEL;
			ref.hModel = b->model;
			ref.scale = 1.0f;
			VectorCopy(org, ref.origin);
			VectorCopy(org, ref.oldorigin);
			angles[0] = pitch;
			angles[1] = yaw;
			angles[2] = rand() % 360;
			AnglesToAxis(angles, ref.axis);
			
			engine->renderer->AddRefEntityToScene(&ref);

			for (i = 0; i < 3; i++)
				org[i] += dist[i] * 30;
			d -= 30;
		}
	}
}