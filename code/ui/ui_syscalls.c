/*
===========================================================================

Return to Castle Wolfenstein single player GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company. 

This file is part of the Return to Castle Wolfenstein single player GPL Source Code (RTCW SP Source Code).  

RTCW SP Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RTCW SP Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RTCW SP Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the RTCW SP Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the RTCW SP Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include "../client/client.h"

/*
====================
GetClipboardData
====================
*/
static void GetClipboardData(char* buf, int buflen) {
	char* cbd;

	cbd = Sys_GetClipboardData();

	if (!cbd) {
		*buf = 0;
		return;
	}

	Q_strncpyz(buf, cbd, buflen);

	Z_Free(cbd);
}

void trap_Print( const char *string ) {
	Com_Printf("%s", string);
}

void trap_Error( const char *string ) {
	Com_Error(ERR_DROP, "%s", string);
}

int trap_Milliseconds( void ) {
	return Sys_Milliseconds();
}

void trap_Cvar_Register( vmCvar_t *cvar, const char *var_name, const char *value, int flags ) {
	Cvar_Register( cvar, var_name, value, flags );
}

void trap_Cvar_Update( vmCvar_t *cvar ) {
	Cvar_Update( cvar );
}

void trap_Cvar_Set( const char *var_name, const char *value ) {
	Cvar_Set( var_name, value );
}

float trap_Cvar_VariableValue( const char *var_name ) {	
	return Cvar_VariableValue(var_name);
}

void trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize ) {
	Cvar_VariableStringBuffer( var_name, buffer, bufsize );
}

void trap_Cvar_SetValue( const char *var_name, float value ) {
	Cvar_SetValue( var_name, value );
}

void trap_Cvar_Reset( const char *name ) {
	Cvar_Reset( name );
}

void trap_Cvar_Create( const char *var_name, const char *var_value, int flags ) {
	Cvar_Get( var_name, var_value, flags );
}

void trap_Cvar_InfoStringBuffer( int bit, char *buffer, int bufsize ) {
	Cvar_InfoStringBuffer( bit, buffer, bufsize );
}

int trap_Argc( void ) {
	return Cmd_Argc();
}

void trap_Argv( int n, char *buffer, int bufferLength ) {
	Cmd_ArgvBuffer( n, buffer, bufferLength );
}

void trap_Cmd_ExecuteText( int exec_when, const char *text ) {
	Cbuf_ExecuteText( exec_when, text );
}

int trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode ) {
	return FS_FOpenFileByMode( qpath, f, mode );
}

void trap_FS_Read( void *buffer, int len, fileHandle_t f ) {
	FS_Read( buffer, len, f );
}

//----(SA)	added
void trap_FS_Seek( fileHandle_t f, long offset, int origin  ) {
	FS_Seek( f, offset, origin );
}
//----(SA)	end

void trap_FS_Write( const void *buffer, int len, fileHandle_t f ) {
	FS_Write( buffer, len, f );
}

void trap_FS_FCloseFile( fileHandle_t f ) {
	FS_FCloseFile( f );
}

int trap_FS_GetFileList(  const char *path, const char *extension, char *listbuf, int bufsize ) {
	return FS_GetFileList( path, extension, listbuf, bufsize );
}

int trap_FS_Delete( const char *filename ) {
	return 0; // syscall(UI_FS_DELETEFILE, filename);
}

qhandle_t trap_R_RegisterModel( const char *name ) {
	return re.RegisterModel( name );
}

qhandle_t trap_R_RegisterSkin( const char *name ) {
	return re.RegisterSkin( name );
}

void trap_R_RegisterFont( const char *fontName, int pointSize, fontInfo_t *font ) {
	re.RegisterFont( fontName, pointSize, font );
}

qhandle_t trap_R_RegisterShaderNoMip( const char *name ) {
	return re.RegisterShaderNoMip(name );
}

void trap_R_ClearScene( void ) {
	re.ClearScene();
}

void trap_R_AddRefEntityToScene( const refEntity_t *ref ) {
	re.AddRefEntityToScene( ref );
}

void trap_R_AddPolyToScene( qhandle_t hShader, int numVerts, const polyVert_t *verts ) {
	re.AddPolyToScene( hShader, numVerts, verts, 1 );
}

void trap_R_AddLightToScene( const vec3_t org, float intensity, float r, float g, float b, int overdraw ) {
	re.AddLightToScene( org, ( intensity ), ( r ), ( g ), ( b ), overdraw );
}

void trap_R_AddCoronaToScene( const vec3_t org, float r, float g, float b, float scale, int id, int flags ) {
	//syscall( UI_R_ADDCORONATOSCENE, org, ( r ), ( g ), ( b ), ( scale ), id, flags );
}

void trap_R_RenderScene( const refdef_t *fd ) {
	re.RenderScene( fd );
}

void trap_R_SetColor( const float *rgba ) {
	re.SetColor( rgba );
}

void trap_R_DrawStretchPic( float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader ) {
	re.DrawStretchPic( ( x ), ( y ), ( w ), ( h ), ( s1 ), ( 1.0f - t1 ), ( s2 ), ( 1.0f - t2 ), hShader );
}

void    trap_R_ModelBounds( clipHandle_t model, vec3_t mins, vec3_t maxs ) {
	re.ModelBounds( model, mins, maxs );
}

void trap_UpdateScreen( void ) {
	SCR_UpdateScreen();
}

int trap_CM_LerpTag( orientation_t *tag, const refEntity_t *refent, const char *tagName, int startIndex ) {
	return 0; // syscall(UI_CM_LERPTAG, tag, refent, tagName, 0);           // NEFVE - SMF - fixed
}

void trap_S_StartLocalSound( sfxHandle_t sfx, int channelNum ) {
	S_StartLocalSound( sfx, channelNum );
}

sfxHandle_t trap_S_RegisterSound( const char *sample ) {
	return S_RegisterSound( sample, qfalse );
}

//----(SA)	added (already in cg)
void    trap_S_FadeBackgroundTrack( float targetvol, int time, int num ) {   // yes, i know.  fadebackground coming in, fadestreaming going out.  will have to see where functionality leads...
	//syscall( UI_S_FADESTREAMINGSOUND, ( targetvol ), time, num ); // 'num' is '0' if it's music, '1' if it's "all streaming sounds"
}

void    trap_S_FadeAllSound( float targetvol, int time ) {
	//syscall( UI_S_FADEALLSOUNDS, ( targetvol ), time );
}
//----(SA)	end


void trap_Key_KeynumToStringBuf( int keynum, char *buf, int buflen ) {
	Key_KeynumToStringBuf( keynum, buf, buflen );
}

void trap_Key_GetBindingBuf( int keynum, char *buf, int buflen ) {
	Key_GetBindingBuf( keynum, buf, buflen );
}

void trap_Key_SetBinding( int keynum, const char *binding ) {
	Key_SetBinding( keynum, binding );
}

qboolean trap_Key_IsDown( int keynum ) {
	return Key_IsDown( keynum );
}

qboolean trap_Key_GetOverstrikeMode( void ) {
	return Key_GetOverstrikeMode();
}

void trap_Key_SetOverstrikeMode( qboolean state ) {
	Key_SetOverstrikeMode( state );
}

void trap_Key_ClearStates( void ) {
	Key_ClearStates();
}

int trap_Key_GetCatcher( void ) {
	return Key_GetCatcher();
}

void trap_Key_SetCatcher( int catcher ) {
	Key_SetCatcher( catcher );
}

void trap_GetClipboardData( char *buf, int bufsize ) {
	GetClipboardData( buf, bufsize );
}

void trap_GetClientState( uiClientState_t *state ) {
	GetClientState( state );
}

void trap_GetGlconfig( glconfig_t *glconfig ) {
	CL_GetGlconfig( glconfig );
}

int trap_GetConfigString( int index, char* buff, int buffsize ) {
	return GetConfigString( index, buff, buffsize );
}

int trap_LAN_GetLocalServerCount( void ) {
	return 0; // syscall(UI_LAN_GETLOCALSERVERCOUNT);
}

void trap_LAN_GetLocalServerAddressString( int n, char *buf, int buflen ) {
	//syscall( UI_LAN_GETLOCALSERVERADDRESSSTRING, n, buf, buflen );
}

int trap_LAN_GetGlobalServerCount( void ) {
	return 0; // syscall(UI_LAN_GETGLOBALSERVERCOUNT);
}

void trap_LAN_GetGlobalServerAddressString( int n, char *buf, int buflen ) {
	//syscall( UI_LAN_GETGLOBALSERVERADDRESSSTRING, n, buf, buflen );
}

int trap_LAN_GetPingQueueCount( void ) {
	return 0; // syscall(UI_LAN_GETPINGQUEUECOUNT);
}

void trap_LAN_ClearPing( int n ) {
	//syscall( UI_LAN_CLEARPING, n );
}

void trap_LAN_GetPing( int n, char *buf, int buflen, int *pingtime ) {
	//syscall( UI_LAN_GETPING, n, buf, buflen, pingtime );
}

void trap_LAN_GetPingInfo( int n, char *buf, int buflen ) {
	//syscall( UI_LAN_GETPINGINFO, n, buf, buflen );
}

// NERVE - SMF
qboolean trap_LAN_UpdateVisiblePings( int source ) {
	return 0; // syscall(UI_LAN_UPDATEVISIBLEPINGS, source);
}

int trap_LAN_GetServerCount( int source ) {
	return 0; // syscall(UI_LAN_GETSERVERCOUNT, source);
}

int trap_LAN_CompareServers( int source, int sortKey, int sortDir, int s1, int s2 ) {
	return 0; // syscall(UI_LAN_COMPARESERVERS, source, sortKey, sortDir, s1, s2);
}

void trap_LAN_GetServerAddressString( int source, int n, char *buf, int buflen ) {
	//syscall( UI_LAN_GETSERVERADDRESSSTRING, source, n, buf, buflen );
}

void trap_LAN_GetServerInfo( int source, int n, char *buf, int buflen ) {
	//syscall( UI_LAN_GETSERVERINFO, source, n, buf, buflen );
}

int trap_LAN_AddServer( int source, const char *name, const char *addr ) {
	return 0; // syscall(UI_LAN_ADDSERVER, source, name, addr);
}

void trap_LAN_RemoveServer( int source, const char *addr ) {
	//syscall( UI_LAN_REMOVESERVER, source, addr );
}

int trap_LAN_GetServerPing( int source, int n ) {
	return 0;
}

int trap_LAN_ServerIsVisible( int source, int n ) {
	return 0;
}

int trap_LAN_ServerStatus( const char *serverAddress, char *serverStatus, int maxLen ) {
	return 0;
}

void trap_LAN_SaveCachedServers() {
	
}

void trap_LAN_LoadCachedServers() {
	
}

void trap_LAN_MarkServerVisible( int source, int n, qboolean visible ) {
	
}

void trap_LAN_ResetPings( int n ) {
	
}
// -NERVE - SMF

int trap_MemoryRemaining( void ) {
	return Hunk_MemoryRemaining();
}


void trap_GetCDKey( char *buf, int buflen ) {
	//syscall( UI_GET_CDKEY, buf, buflen );
}

void trap_SetCDKey( char *buf ) {
	//syscall( UI_SET_CDKEY, buf );
}

int trap_PC_AddGlobalDefine( char *define ) {
	return PC_AddGlobalDefine( define );
}

int trap_PC_LoadSource( const char *filename ) {
	return PC_LoadSourceHandle( filename );
}

int trap_PC_FreeSource( int handle ) {
	return PC_FreeSourceHandle( handle );
}

int trap_PC_ReadToken( int handle, pc_token_t *pc_token ) {
	return PC_ReadTokenHandle( handle, pc_token );
}

int trap_PC_SourceFileAndLine( int handle, char *filename, int *line ) {
	return PC_SourceFileAndLine( handle, filename, line );
}

void trap_S_StopBackgroundTrack( void ) {
	S_StopBackgroundTrack();
}

void trap_S_StartBackgroundTrack( const char *intro, const char *loop, int fadeupTime ) {
	S_StartBackgroundTrack( intro, loop, fadeupTime );
}

int trap_RealTime( qtime_t *qtime ) {
	return Com_RealTime( qtime );
}

// this returns a handle.  arg0 is the name in the format "idlogo.roq", set arg1 to NULL, alteredstates to qfalse (do not alter gamestate)
int trap_CIN_PlayCinematic( const char *arg0, int xpos, int ypos, int width, int height, int bits ) {
	return CIN_PlayCinematic( arg0, xpos, ypos, width, height, bits );
}

// stops playing the cinematic and ends it.  should always return FMV_EOF
// cinematics must be stopped in reverse order of when they are started
e_status trap_CIN_StopCinematic( int handle ) {
	return CIN_StopCinematic( handle );
}


// will run a frame of the cinematic but will not draw it.  Will return FMV_EOF if the end of the cinematic has been reached.
e_status trap_CIN_RunCinematic( int handle ) {
	return CIN_RunCinematic( handle );
}


// draws the current frame
void trap_CIN_DrawCinematic( int handle ) {
	CIN_DrawCinematic( handle );
}


// allows you to resize the animation dynamically
void trap_CIN_SetExtents( int handle, int x, int y, int w, int h ) {
	CIN_SetExtents( handle, x, y, w, h );
}


void    trap_R_RemapShader( const char *oldShader, const char *newShader, const char *timeOffset ) {
	re.RemapShader( oldShader, newShader, timeOffset );
}

qboolean trap_VerifyCDKey( const char *key, const char *chksum ) {
	return qtrue; // yscall(UI_VERIFY_CDKEY, key, chksum);
}

// NERVE - SMF
qboolean trap_GetLimboString( int index, char *buf ) {
	return 0; // syscall(UI_CL_GETLIMBOSTRING, index, buf);
}
// -NERVE - SMF
