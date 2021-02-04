// g_worldspawn.c
//

#include "g_local.h"


/*QUAKED worldspawn (0 0 0) ?

Every map should have exactly one worldspawn.
"music"		music wav file
"gravity"	800 is default gravity
"message"	Text to print during connection process
*/
void SP_worldspawn(void) {
	char* s;

	G_SpawnString("classname", "", &s);
	if (Q_stricmp(s, "worldspawn")) {
		G_Error("SP_worldspawn: The first entity isn't 'worldspawn'");
	}

	// make some data visible to connecting client
	engine->SV_SetConfigstring(CS_GAME_VERSION, GAME_VERSION);

	engine->SV_SetConfigstring(CS_LEVEL_START_TIME, va("%i", level.startTime));

	int fogEnabled;
	G_SpawnInt("fog", "1", &fogEnabled);
	engine->SV_SetConfigstring(CS_FOGENABLED, va("%i", fogEnabled));

	G_SpawnString("music", "", &s);
	engine->SV_SetConfigstring(CS_MUSIC, s);

	G_SpawnString("message", "", &s);
	engine->SV_SetConfigstring(CS_MESSAGE, s);				// map specific message

	engine->SV_SetConfigstring(CS_MOTD, g_motd.string);		// message of the day

	G_SpawnString("gravity", "800", &s);
	engine->Cvar_Set("g_gravity", s);

	G_SpawnString("enableDust", "0", &s);
	engine->Cvar_Set("g_enableDust", s);

	G_SpawnString("enableBreath", "0", &s);
	engine->Cvar_Set("g_enableBreath", s);

	G_SpawnString("worldtype", "0", &s);
	level.worldtype = atoi(s);

	G_SpawnString("script_spawn", "", &s);
	strcpy(level.script_initial_spawn, s);
	
	g_entities[ENTITYNUM_WORLD].s.number = ENTITYNUM_WORLD;
	g_entities[ENTITYNUM_WORLD].classname = "worldspawn";

	// WorldSpawn initial script.
	G_SpawnString("script", "", &s);
	if (s != NULL && strlen(s) > 0)
	{
		G_CallScriptForEntity(s, &g_entities[ENTITYNUM_WORLD]);
	}

	// see if we want a warmup time
	engine->SV_SetConfigstring(CS_WARMUP, "");
	if (g_restarted.integer) {
		engine->Cvar_Set("g_restarted", "0");
		level.warmupTime = 0;
	}
	else if (g_doWarmup.integer) { // Turn it on
		level.warmupTime = -1;
		engine->SV_SetConfigstring(CS_WARMUP, va("%i", level.warmupTime));
		G_LogPrintf("Warmup:\n");
	}

}