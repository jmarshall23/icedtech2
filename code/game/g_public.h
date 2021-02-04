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

// g_public.h -- game module information visible to server

#define	GAME_API_VERSION	1008

// entity->svFlags
// the server does not know how to interpret most of the values
// in entityStates (level eType), so the game must explicitly flag
// special server behaviors
#define	SVF_NOCLIENT			0x00000001	// don't send entity to clients, even if it has effects

// TTimo
// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=551
#define SVF_CLIENTMASK 0x00000002

#define SVF_BOT					0x00000008	// set if the entity is a bot
#define	SVF_BROADCAST			0x00000020	// send to all connected clients
#define	SVF_PORTAL				0x00000040	// merge a second pvs at origin2 into snapshots
#define	SVF_USE_CURRENT_ORIGIN	0x00000080	// entity->r.currentOrigin instead of entity->s.origin
											// for link position (missiles and movers)
#define SVF_SINGLECLIENT		0x00000100	// only send to a single client (entityShared_t->singleClient)
#define SVF_NOSERVERINFO		0x00000200	// don't send CS_SERVERINFO updates to this client
											// so that it can be updated for ping tools without
											// lagging clients
#define SVF_CAPSULE				0x00000400	// use capsule for collision detection instead of bbox
#define SVF_NOTSINGLECLIENT		0x00000800	// send entity to everyone but one client
											// (entityShared_t->singleClient)


//
// Persistant data across singleplayer map transitions
//
typedef struct {
	int health;
	int armor;
	int skill;
	int ammo[MAX_WEAPONS];
	int current_weapon;
	int weapons;
	qboolean isValid;

	// Mods can use these for whatever.
	int userVar1;
	int userVar2;
	int userVar3;
	int userVar4;
} PlayerPersistant_t;

//===============================================================


typedef struct {
	entityState_t	s;				// communicated by server to clients

	qboolean	linked;				// qfalse if not in any good cluster
	int			linkcount;

	int			svFlags;			// SVF_NOCLIENT, SVF_BROADCAST, etc

	// only send to this client when SVF_SINGLECLIENT is set	
	// if SVF_CLIENTMASK is set, use bitmask for clients to send to (maxclients must be <= 32, up to the mod to enforce this)
	int			singleClient;		

	qboolean	bmodel;				// if false, assume an explicit mins / maxs bounding box
									// only set by engine->SV_SetBrushModel
	vec3_t		mins, maxs;
	int			contents;			// CONTENTS_TRIGGER, CONTENTS_SOLID, CONTENTS_BODY, etc
									// a non-solid entity should set to 0

	vec3_t		absmin, absmax;		// derived from mins/maxs and origin + rotation

	// currentOrigin will be used for all collision detection and world linking.
	// it will not necessarily be the same as the trajectory evaluation for the current
	// time, because each entity must be moved one at a time after time is advanced
	// to avoid simultanious collision issues
	vec3_t		currentOrigin;
	vec3_t		currentAngles;

	// when a trace call is made and passEntityNum != ENTITYNUM_NONE,
	// an ent will be excluded from testing if:
	// ent->s.number == passEntityNum	(don't interact with self)
	// ent->s.ownerNum = passEntityNum	(don't interact with your own missiles)
	// entity[ent->s.ownerNum].ownerNum = passEntityNum	(don't interact with other missiles from owner)
	int			ownerNum;
// jmarshall - moved this to entityShared, this is set by LinkEntity and should be considered read only.
	int			areanum;
	int			areanum2;
// jmarshall end
} entityShared_t;



// the server looks at a sharedEntity, which is the start of the game's gentity_t structure
#ifdef QUAKE_ENGINE
typedef struct {
	entityState_t	s;				// communicated by server to clients
	entityShared_t	r;				// shared by both the server system and game
} sharedEntity_t;
#else
typedef struct gentity_s gentity_t;
#define sharedEntity_t	gentity_t
#endif



//===============================================================

//
// system traps provided by the main engine
//
typedef struct {
	// Server specific functions.
	void	 (*SV_LocateGameData)(sharedEntity_t* gEnts, int numGEntities, int sizeofGEntity_t, playerState_t* clients, int sizeofGameClient);
	void	 (*SV_GameDropClient)(int clientNum, const char* reason);
	void	 (*SV_LinkEntity)(sharedEntity_t* ent);
	void	 (*SV_UnlinkEntity)(sharedEntity_t* ent);
	void	 (*SV_GameSendServerCommand)(int clientNum, const char* text);
	int		 (*SV_AreaEntities)(const vec3_t mins, const vec3_t maxs, int* entityList, int maxcount);
	qboolean (*SV_EntityContact)(vec3_t mins, vec3_t maxs, const sharedEntity_t* gEnt, int capsule);
	void	 (*SV_Trace)(trace_t* results, const vec3_t start, vec3_t mins, vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask);
	void	 (*SV_TraceCapsule)(trace_t* results, const vec3_t start, vec3_t mins, vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask);
	void	 (*SV_GetPersistant)(PlayerPersistant_t* persistant);
	void	 (*SV_SetPersistant)(PlayerPersistant_t* persistant);
	int		 (*SV_PointContents)(const vec3_t p, int passEntityNum);
	void	 (*SV_SetBrushModel)(sharedEntity_t* ent, const char* name);
	qboolean (*SV_inPVS)(const vec3_t p1, const vec3_t p2);
	qboolean (*SV_inPVSIgnorePortals)(const vec3_t p1, const vec3_t p2);
	void	 (*SV_SetConfigstring)(int index, const char* val);
	void	 (*SV_GetConfigstring)(int index, char* buffer, int bufferSize);
	void	 (*SV_SetUserinfo)(int index, const char* val);
	void	 (*SV_GetUserinfo)(int index, char* buffer, int bufferSize);
	void	 (*SV_GetServerinfo)(char* buffer, int bufferSize);
	void	 (*SV_AdjustAreaPortalState)(sharedEntity_t* ent, qboolean open);	
	void	 (*SV_GetUsercmd)(int clientNum, usercmd_t* cmd);
	qboolean (*SV_GetEntityToken)(char* buffer, int bufferSize);
	qboolean (*SV_GetModelBounds)(const char* filename, vec3_t mins, vec3_t maxs);

	// Common
	void	(*Com_Error)(int level, const char* error, ...);
	void	(*Com_Printf)(const char* msg, ...);
	int		(*Com_RealTime)(qtime_t* qtime);
	int		(*Sys_Milliseconds)(void);

	// Cvars
	void	(*Cvar_Register)(vmCvar_t* vmCvar, const char* varName, const char* defaultValue, int flags);
	void	(*Cvar_Update)(vmCvar_t* vmCvar);
	void 	(*Cvar_Set)(const char* var_name, const char* value);
	void	(*Cvar_VariableStringBuffer)(const char* var_name, char* buffer, int bufsize);
	int		(*Cvar_VariableIntegerValue)(const char* var_name);
	int		(*Cmd_Argc)(void);
	void	(*Cmd_ArgvBuffer)(int arg, char* buffer, int bufferLength);
	void	(*Cmd_ArgsBuffer)(char* buffer, int bufferLength);

	// FileSystem
	int		(*FS_FOpenFileByMode)(const char* qpath, fileHandle_t* f, fsMode_t mode);
	int		(*FS_Read2)(void* buffer, int len, fileHandle_t f);
	int		(*FS_Write)(const void* buffer, int len, fileHandle_t f);
	int		(*FS_Seek)(fileHandle_t f, long offset, int origin);
	void	(*FS_FCloseFile)(fileHandle_t f);
	int		(*FS_GetFileList)(const char* path, const char* extension, char* listbuf, int bufsize);

	// Command System
	void	(*Cbuf_ExecuteText)(int exec_when, const char* text);
	void	(*Cbuf_AddText)(const char* text);
	void	(*Cmd_RemoveCommand)(const char* cmd_name);

	// Collision System
	qboolean	(*CM_AreasConnected)(int area1, int area2);

	// Save Game
	qhandle_t	(*SV_OpenSaveForWrite)(const char* name);
	qboolean	(*SV_GetSaveGameName)(const char* name);
	qhandle_t	(*SV_OpenSave)(const char* name);

	// System
	void		(*Sys_SnapVector)(float* v);
} gameImport_t;


//
// functions exported by the game subsystem
//
typedef struct {
	void (*G_InitGame) ( int levelTime, int randomSeed, int restart );
	// init and shutdown will be called every single level
	// The game should call G_GET_ENTITY_TOKEN to parse through all the
	// entity configuration text and spawn gentities.

	void (*G_ShutdownGame)(int restart);

	char *(*ClientConnect) ( int clientNum, qboolean firstTime, qboolean isBot );
	
	// return NULL if the client is allowed to connect, otherwise return
	// a text string with the reason for denial

	void (*ClientBegin) ( int clientNum );

	void (*ClientUserinfoChanged)( int clientNum );

	void (*ClientDisconnect)( int clientNum );

	void (*ClientCommand) ( int clientNum );

	void (*ClientThink) ( int clientNum );

	void (*G_RunFrame) ( int levelTime );

	qboolean (*ConsoleCommand) ( void );
} gameExport_t;

