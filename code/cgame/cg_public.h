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

#define	CMD_BACKUP			64	
#define	CMD_MASK			(CMD_BACKUP - 1)
// allow a lot of command backups for very fast systems
// multiple commands may be combined into a single packet, so this
// needs to be larger than PACKET_BACKUP


#define	MAX_ENTITIES_IN_SNAPSHOT	256

// snapshots are a view of the server at a given time

// Snapshots are generated at regular time intervals by the server,
// but they may not be sent if a client's rate level is exceeded, or
// they may be dropped by the network.
typedef struct {
	int				snapFlags;			// SNAPFLAG_RATE_DELAYED, etc
	int				ping;

	int				serverTime;		// server time the message is valid for (in msec)

	byte			areamask[MAX_MAP_AREA_BYTES];		// portalarea visibility bits

	playerState_t	ps;						// complete information about the current player at this time

	int				numEntities;			// all of the entities that need to be presented
	entityState_t	entities[MAX_ENTITIES_IN_SNAPSHOT];	// at the time of this snapshot

	int				numServerCommands;		// text based server commands to execute when this
	int				serverCommandSequence;	// snapshot becomes current
} snapshot_t;

enum {
  CGAME_EVENT_NONE,
  CGAME_EVENT_TEAMMENU,
  CGAME_EVENT_SCOREBOARD,
  CGAME_EVENT_EDITHUD
};


/*
==================================================================

functions imported from the main executable

==================================================================
*/

#define	CGAME_IMPORT_API_VERSION	2001

typedef struct {
	// Client specific API
	void	(*CL_GetGlconfig)(glconfig_t* config);
	void	(*CL_GetGameState)(gameState_t* gs);
	void	(*CL_GetCurrentSnapshotNumber)(int* snapshotNumber, int* serverTime);
	qboolean (*CL_GetSnapshot)(int snapshotNumber, snapshot_t* snapshot);
	qboolean (*CL_GetServerCommand)(int serverCommandNumber);
	int		(*CL_GetCurrentCmdNumber)(void);
	qboolean (*CL_GetUserCmd)(int cmdNumber, usercmd_t* ucmd);
	void (*CL_SetUserCmdValue)(int userCmdValue, float sensitivityScale);
	int	(*Hunk_MemoryRemaining)(void);

	// Common
	void	(*Com_Error)(int level, const char* error, ...);
	void	(*Com_Printf)(const char* msg, ...);
	int		(*Com_RealTime)(qtime_t* qtime);
	int		(*Sys_Milliseconds)(void);
	void	(*SCR_UpdateScreen)(void);

	// Key input
	qboolean(*Key_IsDown)(int keynum);
	int (*Key_GetCatcher)(void);
	void (*Key_SetCatcher)(int catcher);
	int (*Key_GetKey)(const char* binding);

	// Cvars
	void	(*Cvar_Register)(vmCvar_t* vmCvar, const char* varName, const char* defaultValue, int flags);
	void	(*Cvar_Update)(vmCvar_t* vmCvar);
	void 	(*Cvar_Set)(const char* var_name, const char* value);
	void	(*Cvar_VariableStringBuffer)(const char* var_name, char* buffer, int bufsize);
	int		(*Cmd_Argc)(void);
	void	(*Cmd_ArgvBuffer)(int arg, char* buffer, int bufferLength);
	void	(*Cmd_ArgsBuffer)(char* buffer, int bufferLength);

	// FileSystem
	int		(*FS_FOpenFileByMode)(const char* qpath, fileHandle_t* f, fsMode_t mode);
	int		(*FS_Read2)(void* buffer, int len, fileHandle_t f);
	int		(*FS_Write)(const void* buffer, int len, fileHandle_t f);
	int		(*FS_Seek)(fileHandle_t f, long offset, int origin);
	void	(*FS_FCloseFile)(fileHandle_t f);

	// Command System
	void	(*Cbuf_AddText)(const char* text);
	void	(*CL_AddCgameCommand)(const char* cmdName);
	void	(*Cmd_RemoveCommand)(const char* cmd_name);
	void	(*CL_AddReliableCommand)(const char* cmd);

	// Renderer
	refexport_t* renderer;

	// User Interface
	void	(*UI_Text_Paint)(float x, float y, float scale, vec4_t color, const char* text);
	void	(*UI_ForceActiveCustomMenu)(const char* name);

	// Collision Manager
	void	(*CL_CM_LoadMap)(const char* mapname);
	int		(*CM_NumInlineModels)(void);
	clipHandle_t(*CM_InlineModel)(int index);
	clipHandle_t(*CM_TempBoxModel)(const vec3_t mins, const vec3_t maxs, int capsule);
	int			(*CM_PointContents)(const vec3_t p, clipHandle_t model);
	int			(*CM_TransformedPointContents)(const vec3_t p, clipHandle_t model, const vec3_t origin, const vec3_t angles);
	void		(*CM_BoxTrace)(trace_t* results, const vec3_t start, const vec3_t end, vec3_t mins, vec3_t maxs, clipHandle_t model, int brushmask, int capsule);
	void		(*CM_TransformedBoxTrace)(trace_t* results, const vec3_t start, const vec3_t end, vec3_t mins, vec3_t maxs, clipHandle_t model, int brushmask, const vec3_t origin, const vec3_t angles, int capsule);

	// Sound System
	void		(*S_StartSound)(vec3_t origin, int entnum, int entchannel, sfxHandle_t sfx);
	void		(*S_StartSoundEx)(vec3_t origin, int entnum, int entchannel, sfxHandle_t sfx, int flags);
	void		(*S_StartLocalSound)(sfxHandle_t sfx, int channelNum);
	void		(*S_StopEntStreamingSound)(int entNum);
	void		(*S_UpdateEntityPosition)(int entityNum, const vec3_t origin);
	int			(*S_GetVoiceAmplitude)(int entityNum);
	void		(*S_Respatialize)(int entityNum, const vec3_t origin, vec3_t axis[3], int inwater);
	sfxHandle_t	(*S_RegisterSound)(const char* sample);
	void		(*S_StartBackgroundTrack)(const char* intro, const char* loop);
	void		(*S_FadeStreamingSound)(float targetvol, int time, int ssNum);
	void		(*S_StartStreamingSound)(const char* intro, const char* loop, int entityNum, int channel, int attenuation);
	void		(*S_FadeAllSounds)(float targetvol, int time);
	qboolean	(*S_IsSoundPlaying)(sfxHandle_t sfx);
	void		(*S_StopBackgroundTrack)(void);
	void		(*S_AddLoopingSound)(int entityNum, const vec3_t origin, const vec3_t velocity, const int range, sfxHandle_t sfx, int volume);
	void		(*S_ClearLoopingSounds)(qboolean killall);

	// System
	void		(*Sys_SnapVector)(float* v);

} cgameImport_t;


/*
==================================================================

functions exported to the main executable

==================================================================
*/

typedef struct {
	void (*CG_Init)(int serverMessageNum, int serverCommandSequence, int clientNum);
	// called when the level loads or when the renderer is restarted
	// all media should be registered at this time
	// cgame will display loading status by calling SCR_Update, which
	// will call CG_DrawInformation during the loading process
	// reliableCommandSequence will be 0 on fresh loads, but higher for
	// demos, tourney restarts, or vid_restarts

	void (*CG_Shutdown)( void );
	// oportunity to flush and close any open files

	qboolean (*CG_ConsoleCommand)( void );

	// a console command has been issued locally that is not recognized by the
	// main game system.
	// use Cmd_Argc() / Cmd_Argv() to read the command, return qfalse if the
	// command is not known to the game
	void (*CG_DrawActiveFrame)( int serverTime, stereoFrame_t stereoView, qboolean demoPlayback );
	// Generates and draws a game scene and status information at the given time.
	// If demoPlayback is set, local movement prediction will not be enabled
	
	int  (*CG_CrosshairPlayer)( void );
	int  (*CG_LastAttacker)( void );
	void (*CG_KeyEvent)( int key, qboolean down );
	void (*CG_MouseEvent)( int dx, int dy );
	void (*CG_EventHandling)(int type);
} cgameExport_t;

//----------------------------------------------
