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
// sv_game.c -- interface to the game dll

#include "server.h"

intptr_t *gameDLLhandle = NULL;
gameExport_t* gameVM = NULL;

const char* currentSaveGameName = NULL;
char saveGameName[512];

PlayerPersistant_t playerPersistant;

qboolean SV_GetModelBounds(const char* filename, vec3_t mins, vec3_t maxs) {
	return RE_GetModelBounds(filename, mins, maxs);
}

qhandle_t	SV_OpenSaveForWrite(const char* name) {
	const char* fileName = va("savegames/%s.sav", name);

	return FS_FOpenFileWrite(fileName);
}

qhandle_t	SV_OpenSave(const char* name) {
	const char* fileName = va("savegames/%s.sav", name);

	fileHandle_t f;
	FS_FOpenFileRead(fileName, &f, qfalse);
	return f;
}

qboolean SV_GetSaveGameName(const char *name) {
	if (currentSaveGameName == NULL)
		return qfalse;

	strcpy(name, currentSaveGameName);
	currentSaveGameName = NULL;

	return qtrue;
}

void SV_SetPersistant(PlayerPersistant_t* persistant) {
	memcpy(&playerPersistant, persistant, sizeof(PlayerPersistant_t));
	playerPersistant.isValid = qtrue;
}

void SV_GetPersistant(PlayerPersistant_t* persistant) {
	memcpy(persistant, &playerPersistant, sizeof(PlayerPersistant_t));
	memset(&playerPersistant, 0, sizeof(PlayerPersistant_t)); // Its been consumed!
}

void SV_GameError( const char *string ) {
	Com_Error( ERR_DROP, "%s", string );
}

void SV_GamePrint( const char *string ) {
	Com_Printf( "%s", string );
}

// these functions must be used instead of pointer arithmetic, because
// the game allocates gentities with private information after the server shared part
int	SV_NumForGentity( sharedEntity_t *ent ) {
	int		num;

	num = ( (byte *)ent - (byte *)sv.gentities ) / sv.gentitySize;

	return num;
}

sharedEntity_t *SV_GentityNum( int num ) {
	sharedEntity_t *ent;

	ent = (sharedEntity_t *)((byte *)sv.gentities + sv.gentitySize*(num));

	return ent;
}

playerState_t *SV_GameClientNum( int num ) {
	playerState_t	*ps;

	ps = (playerState_t *)((byte *)sv.gameClients + sv.gameClientSize*(num));

	return ps;
}

svEntity_t	*SV_SvEntityForGentity( sharedEntity_t *gEnt ) {
	if ( !gEnt || gEnt->s.number < 0 || gEnt->s.number >= MAX_GENTITIES ) {
		Com_Error( ERR_DROP, "SV_SvEntityForGentity: bad gEnt" );
	}
	return &sv.svEntities[ gEnt->s.number ];
}

sharedEntity_t *SV_GEntityForSvEntity( svEntity_t *svEnt ) {
	int		num;

	num = svEnt - sv.svEntities;
	return SV_GentityNum( num );
}

/*
===============
SV_GameSendServerCommand

Sends a command string to a client
===============
*/
void SV_GameSendServerCommand( int clientNum, const char *text ) {
	if ( clientNum == -1 ) {
		SV_SendServerCommand( NULL, "%s", text );
	} else {
		if ( clientNum < 0 || clientNum >= sv_maxclients->integer ) {
			return;
		}
		SV_SendServerCommand( svs.clients + clientNum, "%s", text );	
	}
}


/*
===============
SV_GameDropClient

Disconnects the client with a message
===============
*/
void SV_GameDropClient( int clientNum, const char *reason ) {
	if ( clientNum < 0 || clientNum >= sv_maxclients->integer ) {
		return;
	}
	SV_DropClient( svs.clients + clientNum, reason );	
}


/*
=================
SV_SetBrushModel

sets mins and maxs for inline bmodels
=================
*/
void SV_SetBrushModel( sharedEntity_t *ent, const char *name ) {
	clipHandle_t	h;
	vec3_t			mins, maxs;

	if (!name) {
		Com_Error( ERR_DROP, "SV_SetBrushModel: NULL" );
	}

	if (name[0] != '*') {
		Com_Error( ERR_DROP, "SV_SetBrushModel: %s isn't a brush model", name );
	}


	ent->s.modelindex = atoi( name + 1 );

	h = CM_InlineModel( ent->s.modelindex );
	CM_ModelBounds( h, mins, maxs );
	VectorCopy (mins, ent->r.mins);
	VectorCopy (maxs, ent->r.maxs);
	ent->r.bmodel = qtrue;

	ent->r.contents = -1;		// we don't know exactly what is in the brushes

	SV_LinkEntity( ent );		// FIXME: remove
}



/*
=================
SV_inPVS

Also checks portalareas so that doors block sight
=================
*/
qboolean SV_inPVS (const vec3_t p1, const vec3_t p2)
{
	int		leafnum;
	int		cluster;
	int		area1, area2;
	byte	*mask;

	leafnum = CM_PointLeafnum (p1);
	cluster = CM_LeafCluster (leafnum);
	area1 = CM_LeafArea (leafnum);
	mask = CM_ClusterPVS (cluster);

	leafnum = CM_PointLeafnum (p2);
	cluster = CM_LeafCluster (leafnum);
	area2 = CM_LeafArea (leafnum);
	if ( mask && (!(mask[cluster>>3] & (1<<(cluster&7)) ) ) )
		return qfalse;
	if (!CM_AreasConnected (area1, area2))
		return qfalse;		// a door blocks sight
	return qtrue;
}


/*
=================
SV_inPVSIgnorePortals

Does NOT check portalareas
=================
*/
qboolean SV_inPVSIgnorePortals( const vec3_t p1, const vec3_t p2)
{
	int		leafnum;
	int		cluster;
	int		area1, area2;
	byte	*mask;

	leafnum = CM_PointLeafnum (p1);
	cluster = CM_LeafCluster (leafnum);
	area1 = CM_LeafArea (leafnum);
	mask = CM_ClusterPVS (cluster);

	leafnum = CM_PointLeafnum (p2);
	cluster = CM_LeafCluster (leafnum);
	area2 = CM_LeafArea (leafnum);

	if ( mask && (!(mask[cluster>>3] & (1<<(cluster&7)) ) ) )
		return qfalse;

	return qtrue;
}


/*
========================
SV_AdjustAreaPortalState
========================
*/
void SV_AdjustAreaPortalState( sharedEntity_t *ent, qboolean open ) {
	svEntity_t	*svEnt;

	svEnt = SV_SvEntityForGentity( ent );
	if (ent->r.areanum2 == -1 ) {
		return;
	}
	CM_AdjustAreaPortalState(ent->r.areanum, ent->r.areanum2, open );
}

/*
==================
SV_GameAreaEntities
==================
*/
qboolean	SV_EntityContact( vec3_t mins, vec3_t maxs, const sharedEntity_t *gEnt, int capsule ) {
	const float	*origin, *angles;
	clipHandle_t	ch;
	trace_t			trace;

	// check for exact collision
	origin = gEnt->r.currentOrigin;
	angles = gEnt->r.currentAngles;

	ch = SV_ClipHandleForEntity( gEnt );
	CM_TransformedBoxTrace ( &trace, vec3_origin, vec3_origin, mins, maxs,
		ch, -1, origin, angles, capsule );

	return trace.startsolid;
}


/*
===============
SV_GetServerinfo

===============
*/
void SV_GetServerinfo( char *buffer, int bufferSize ) {
	if ( bufferSize < 1 ) {
		Com_Error( ERR_DROP, "SV_GetServerinfo: bufferSize == %i", bufferSize );
	}
	Q_strncpyz( buffer, Cvar_InfoString( CVAR_SERVERINFO ), bufferSize );
}

/*
===============
SV_LocateGameData

===============
*/
void SV_LocateGameData( sharedEntity_t *gEnts, int numGEntities, int sizeofGEntity_t, playerState_t *clients, int sizeofGameClient ) {
	sv.gentities = gEnts;
	sv.gentitySize = sizeofGEntity_t;
	sv.num_entities = numGEntities;

	sv.gameClients = clients;
	sv.gameClientSize = sizeofGameClient;
}


/*
===============
SV_GetUsercmd

===============
*/
void SV_GetUsercmd( int clientNum, usercmd_t *cmd ) {
	if ( clientNum < 0 || clientNum >= sv_maxclients->integer ) {
		Com_Error( ERR_DROP, "SV_GetUsercmd: bad clientNum:%i", clientNum );
	}
	*cmd = svs.clients[clientNum].lastUsercmd;
}

//==============================================

static int	FloatAsInt( float f ) {
	union
	{
	    int i;
	    float f;
	} temp;
	
	temp.f = f;
	return temp.i;
}

qboolean SV_GetEntityToken(char* buffer, int bufferSize) {
	const char* s;

	s = COM_Parse(&sv.entityParsePoint);
	Q_strncpyz(buffer, s, bufferSize);
	if (!sv.entityParsePoint && !s[0]) {
		return qfalse;
	}
	return qtrue;
}

#define GAME_BIND_FUNCTION(function) engineApi->##function = ##function;

void SV_Trace2(trace_t* results, const vec3_t start, vec3_t mins, vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask) {
	SV_Trace(results, start, mins, maxs, end, passEntityNum, contentmask, qfalse);
}

void SV_TraceCapsule(trace_t* results, const vec3_t start, vec3_t mins, vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask) {
	SV_Trace(results, start, mins, maxs, end, passEntityNum, contentmask, qtrue);
}

/*
====================
CL_BuildGameImport
====================
*/
void SV_BuildGameImport(gameImport_t* engineApi) {
	// Server specific functions.
	GAME_BIND_FUNCTION(SV_LocateGameData);
	GAME_BIND_FUNCTION(SV_GameDropClient);
	GAME_BIND_FUNCTION(SV_LinkEntity);
	GAME_BIND_FUNCTION(SV_UnlinkEntity);
	GAME_BIND_FUNCTION(SV_GameSendServerCommand);
	GAME_BIND_FUNCTION(SV_AreaEntities);
	GAME_BIND_FUNCTION(SV_EntityContact);
	//GAME_BIND_FUNCTION(SV_Trace);
	engineApi->SV_Trace = SV_Trace2;
	GAME_BIND_FUNCTION(SV_GetModelBounds);
	GAME_BIND_FUNCTION(SV_TraceCapsule);
	GAME_BIND_FUNCTION(SV_GetPersistant);
	GAME_BIND_FUNCTION(SV_SetPersistant);
	GAME_BIND_FUNCTION(SV_PointContents);
	GAME_BIND_FUNCTION(SV_SetBrushModel);
	GAME_BIND_FUNCTION(SV_inPVS);
	GAME_BIND_FUNCTION(SV_inPVSIgnorePortals);
	GAME_BIND_FUNCTION(SV_SetConfigstring);
	GAME_BIND_FUNCTION(SV_GetConfigstring);
	GAME_BIND_FUNCTION(SV_SetUserinfo);
	GAME_BIND_FUNCTION(SV_GetUserinfo);
	GAME_BIND_FUNCTION(SV_GetServerinfo);
	GAME_BIND_FUNCTION(SV_AdjustAreaPortalState);
	GAME_BIND_FUNCTION(SV_GetUsercmd);
	GAME_BIND_FUNCTION(SV_GetEntityToken);

	// Common
	GAME_BIND_FUNCTION(Com_Error);
	GAME_BIND_FUNCTION(Com_Printf);
	GAME_BIND_FUNCTION(Com_RealTime);
	GAME_BIND_FUNCTION(Sys_Milliseconds);

	// Cvars
	GAME_BIND_FUNCTION(Cvar_Register);
	GAME_BIND_FUNCTION(Cvar_Update);
	GAME_BIND_FUNCTION(Cvar_Set);
	GAME_BIND_FUNCTION(Cvar_VariableStringBuffer);
	GAME_BIND_FUNCTION(Cvar_VariableIntegerValue);
	GAME_BIND_FUNCTION(Cmd_Argc);
	GAME_BIND_FUNCTION(Cmd_ArgvBuffer);
	GAME_BIND_FUNCTION(Cmd_ArgsBuffer);

	// FileSystem
	GAME_BIND_FUNCTION(FS_FOpenFileByMode);
	GAME_BIND_FUNCTION(FS_Read2);
	GAME_BIND_FUNCTION(FS_Write);
	GAME_BIND_FUNCTION(FS_Seek);
	GAME_BIND_FUNCTION(FS_FCloseFile);
	GAME_BIND_FUNCTION(FS_GetFileList);

	// Command System
	GAME_BIND_FUNCTION(Cbuf_ExecuteText);
	GAME_BIND_FUNCTION(Cbuf_AddText);	
	GAME_BIND_FUNCTION(Cmd_RemoveCommand);	

	// Collision System
	GAME_BIND_FUNCTION(CM_AreasConnected);

	// Save Game
	GAME_BIND_FUNCTION(SV_OpenSaveForWrite);
	GAME_BIND_FUNCTION(SV_GetSaveGameName);
	GAME_BIND_FUNCTION(SV_OpenSave);

	// System
	GAME_BIND_FUNCTION(Sys_SnapVector);
}

/*
===============
SV_ShutdownGameProgs

Called every time a map changes
===============
*/
void SV_ShutdownGameProgs( void ) {
	if ( !gameVM) {
		return;
	}
	//VM_Call( gvm, GAME_SHUTDOWN, qfalse );
	gameVM->G_ShutdownGame(qfalse);

	Sys_UnloadDll(gameDLLhandle);
	
	gameVM = NULL;
	gameDLLhandle = NULL;
}

/*
==================
SV_InitGameVM

Called for both a full init and a restart
==================
*/
static void SV_InitGameVM( qboolean restart, const char* saveGame) {
	int		i;

	// start the entity parsing at the beginning
	sv.entityParsePoint = CM_EntityString();

	// clear all gentity pointers that might still be set from
	// a previous level
	// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=522
	//   now done before GAME_INIT call
	for ( i = 0 ; i < sv_maxclients->integer ; i++ ) {
		svs.clients[i].gentity = NULL;
	}

	if (saveGame == NULL)
	{
		currentSaveGameName = NULL;
		memset(saveGameName, 0, sizeof(saveGameName));
	}
	else
	{
		strcpy(saveGameName, saveGame);
		currentSaveGameName = &saveGameName[0];
	}
	
	// use the current msec count for a random seed
	// init for this gamestate
	//VM_Call( gvm, GAME_INIT, svs.time, Com_Milliseconds(), restart );
	gameVM->G_InitGame(svs.time, Com_Milliseconds(), restart);
}



/*
===================
SV_RestartGameProgs

Called on a map_restart, but not on a normal map change
===================
*/
void SV_RestartGameProgs( void ) {
	if ( !gameVM) {
		return;
	}
	//VM_Call( gvm, GAME_SHUTDOWN, qtrue );
	gameVM->G_ShutdownGame(qtrue);

	// do a restart instead of a free
	//gvm = VM_Restart( gvm );
	//if ( !gvm ) { // bk001212 - as done below
	//	Com_Error( ERR_FATAL, "VM_Restart on game failed" );
	//}

	SV_InitGameVM( qtrue, NULL );
}


/*
===============
SV_InitGameProgs

Called on a normal map change, not on a map_restart
===============
*/
void SV_InitGameProgs(const char* saveGame) {
	cvar_t	*var;
	static gameImport_t engineApi;
	gameExport_t* (*entryPoint)(int apiVersion, void* engineApi);

	// load the dll or bytecode
	//gvm = VM_Create( "qagame", SV_GameSystemCalls, Cvar_VariableValue( "vm_game" ) );
	//if ( !gvm ) {
	//	Com_Error( ERR_FATAL, "VM_Create on game failed" );
	//}
	gameDLLhandle = Sys_LoadDll("qagame", &entryPoint);
	if (!gameDLLhandle) {
		Com_Error(ERR_DROP, "VM_Create on game failed");
	}

	SV_BuildGameImport(&engineApi);

	// Grab the vm functions.
	gameVM = entryPoint(GAME_API_VERSION, &engineApi);
	if (gameVM == NULL) {
		Com_Error(ERR_FATAL, "Invalid game version\n");
		return;
	}

	SV_InitGameVM( qfalse, saveGame);
}


/*
====================
SV_GameCommand

See if the current console command is claimed by the game
====================
*/
qboolean SV_GameCommand( void ) {
	if ( sv.state != SS_GAME ) {
		return qfalse;
	}

	return gameVM->ConsoleCommand(); //VM_Call( gvm, GAME_CONSOLE_COMMAND );
}

