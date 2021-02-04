// g_save.c
//

#include "g_local.h"
#include "../superscript/generated/save_func.h"

/*
===============
G_UpdatePersistant
===============
*/
const char* G_FindSaveFunctionName(void* ptr) {
	for (int i = 0; i < NUM_SUPERSCRIPT_FUNCS; i++) {
		if (superScriptTable[i].func1 == ptr || superScriptTable[i].func2 == ptr || superScriptTable[i].func3 == ptr)
			return superScriptTable[i].func_name;
	}

	G_Error("Failed to find save function name!\n");
	return NULL;
}

/*
===============
G_UpdatePersistant
===============
*/
void G_UpdatePersistant(void) {
	PlayerPersistant_t persistant;
	memset(&persistant, 0, sizeof(PlayerPersistant_t));

	persistant.isValid = qtrue;
	persistant.skill = g_skill.integer;
	persistant.health = g_entities[0].health;
	persistant.armor = level.clients[0].ps.stats[STAT_ARMOR];
	persistant.current_weapon = level.clients[0].ps.weapon;
	persistant.weapons = level.clients[0].ps.stats[STAT_WEAPONS];
	memcpy(persistant.ammo, level.clients[0].ps.ammo, sizeof(int) * MAX_WEAPONS);

	engine->SV_SetPersistant(&persistant);
}

/*
===============
G_SaveGame
===============
*/
void G_SaveGame(const char* name) {
	qhandle_t f;

	// Open the save file for write.
	f = engine->SV_OpenSaveForWrite(name);
	if (f == -1) {
		G_Error("Failed to open save file for writing!\n");
		return;
	}

	// Write the save version.
	fsSaveHeader_t header;
	header.version = SAVEGAME_VERSION;
	header.skill = g_skill.integer;
	strcpy(header.mapName, g_mapname.string);
	engine->FS_Write(&header, sizeof(fsSaveHeader_t), f);

	// Write out all the player info.
	FS_WriteInt(f, g_entities[0].health);
	FS_WriteVec3(f, g_entities[0].r.currentOrigin);
	FS_WriteVec3(f, g_entities[0].client->ps.viewangles);
	FS_WriteInt(f, level.clients[0].ps.stats[STAT_ARMOR]);
	FS_WriteInt(f, level.clients[0].ps.weapon);
	FS_WriteInt(f, level.clients[0].ps.stats[STAT_WEAPONS]);
	engine->FS_Write(level.clients[0].ps.ammo, sizeof(int) * MAX_WEAPONS, f);

	// Close the file.
	engine->FS_FCloseFile(f);
}

/*
===============
G_LoadGame
===============
*/
void G_LoadGame(const char* name) {
	qhandle_t f;

	f = engine->SV_OpenSave(name);
	if (f <= 0 ) {
		G_Error("Failed to open save file for reading!\n");
		return;
	}

	fsSaveHeader_t header;
	engine->FS_Read2(&header, sizeof(fsSaveHeader_t), f);

	level.clients[0].ps.stats[STAT_HEALTH] = g_entities[0].health = FS_ReadInt(f);
	FS_ReadVec(f, g_entities[0].r.currentOrigin, 3);
	FS_ReadVec(f, g_entities[0].r.currentAngles, 3);
	level.clients[0].ps.stats[STAT_ARMOR] = FS_ReadInt(f);
	level.clients[0].ps.weapon = FS_ReadInt(f);
	level.clients[0].ps.stats[STAT_WEAPONS] = FS_ReadInt(f);
	engine->FS_Read2(level.clients[0].ps.ammo, sizeof(int) * MAX_WEAPONS, f);

	// Update the target entity.
	G_ClientSwitchWeapon(&g_entities[0], level.clients[0].ps.weapon);
	TeleportPlayer(&g_entities[0], g_entities[0].r.currentOrigin, g_entities[0].client->ps.viewangles);
}