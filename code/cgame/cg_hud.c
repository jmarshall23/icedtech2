// cg_hud.c
//

#include "cg_local.h"

/*
=================
CG_GetColorForHealth
=================
*/
void CG_GetColorForHealth(int health, int armor, vec4_t hcolor) {
	int		count;
	int		max;

	// calculate the total points of damage that can
	// be sustained at the current health / armor level
	if (health <= 0) {
		VectorClear(hcolor);	// black
		hcolor[3] = 1;
		return;
	}
	count = armor;
	max = health * ARMOR_PROTECTION / (1.0 - ARMOR_PROTECTION);
	if (max < count) {
		count = max;
	}
	health += count;

	// set the color based on health
	hcolor[0] = 1.0;
	hcolor[3] = 1.0;
	if (health >= 100) {
		hcolor[2] = 1.0;
	}
	else if (health < 66) {
		hcolor[2] = 0;
	}
	else {
		hcolor[2] = (health - 66) / 33.0;
	}

	if (health > 60) {
		hcolor[1] = 1.0;
	}
	else if (health < 30) {
		hcolor[1] = 0;
	}
	else {
		hcolor[1] = (health - 30) / 30.0;
	}
}

/*
=================
CG_ColorForHealth
=================
*/
void CG_ColorForHealth(vec4_t hcolor) {

	CG_GetColorForHealth(cg.snap->ps.stats[STAT_HEALTH],
		cg.snap->ps.stats[STAT_ARMOR], hcolor);
}

/*
================
CG_AdjustFrom640

Adjusted for resolution and screen aspect ratio
================
*/
void CG_AdjustFrom640(float* x, float* y, float* w, float* h) {
	// scale for screen sizes
	* x *= cgs.screenXScale;
	*y *= cgs.screenYScale;
	*w *= cgs.screenXScale;
	*h *= cgs.screenYScale;
}

/*
===============
CG_DrawChar

Coordinates and size in 640*480 virtual screen size
===============
*/
void CG_DrawChar( int x, int y, int width, int height, int ch ) {
	int row, col;
	float frow, fcol;
	float size;
	float	ax, ay, aw, ah;

	ch &= 255;

	if ( ch == ' ' ) {
		return;
	}

	ax = x;
	ay = y;
	aw = width;
	ah = height;
	CG_AdjustFrom640( &ax, &ay, &aw, &ah );

	row = ch>>4;
	col = ch&15;

	frow = row*0.0625;
	fcol = col*0.0625;
	size = 0.0625;

	trap_R_DrawStretchPic( ax, ay, aw, ah,
					   fcol, frow, 
					   fcol + size, frow + size, 
					   cgs.media.charsetShader );
}

/*
================
CG_DrawPic

Coordinates are 640*480 virtual values
=================
*/
void CG_DrawPic(float x, float y, float width, float height, qhandle_t hShader) {
	CG_AdjustFrom640(&x, &y, &width, &height);
	trap_R_DrawStretchPic(x, y, width, height, 0, 0, 1, 1, hShader);
}

/*
==============
CG_DrawField

Draws large numbers for status bar and powerups
==============
*/
static void CG_DrawField(int x, int y, int width, int value) {
	char	num[16], * ptr;
	int		l;
	int		frame;

	if (width < 1) {
		return;
	}

	// draw number string
	if (width > 5) {
		width = 5;
	}

	switch (width) {
	case 1:
		value = value > 9 ? 9 : value;
		value = value < 0 ? 0 : value;
		break;
	case 2:
		value = value > 99 ? 99 : value;
		value = value < -9 ? -9 : value;
		break;
	case 3:
		value = value > 999 ? 999 : value;
		value = value < -99 ? -99 : value;
		break;
	case 4:
		value = value > 9999 ? 9999 : value;
		value = value < -999 ? -999 : value;
		break;
	}

	Com_sprintf(num, sizeof(num), "%i", value);
	l = strlen(num);
	if (l > width)
		l = width;
	x += 2 + CHAR_WIDTH * (width - l);

	ptr = num;
	while (*ptr && l)
	{
		if (*ptr == '-')
			frame = STAT_MINUS;
		else
			frame = *ptr - '0';

		CG_DrawPic(x, y, CHAR_WIDTH, CHAR_HEIGHT, cgs.media.numberShaders[frame]);
		x += CHAR_WIDTH;
		ptr++;
		l--;
	}
}

/*
===============================================================================

CENTER PRINTING

===============================================================================
*/

/*
================
CG_FadeColor
================
*/
float* CG_FadeColor(int startMsec, int totalMsec) {
	static vec4_t		color;
	int			t;

	if (startMsec == 0) {
		return NULL;
	}

	t = cg.time - startMsec;

	if (t >= totalMsec) {
		return NULL;
	}

	// fade out
	if (totalMsec - t < FADE_TIME) {
		color[3] = (totalMsec - t) * 1.0 / FADE_TIME;
	}
	else {
		color[3] = 1.0;
	}
	color[0] = color[1] = color[2] = 1;

	return color;
}

/*
=================
CG_DrawStrlen

Returns character count, skiping color escape codes
=================
*/
int CG_DrawStrlen(const char* str) {
	const char* s = str;
	int count = 0;

	while (*s) {
		if (Q_IsColorString(s)) {
			s += 2;
		}
		else {
			count++;
			s++;
		}
	}

	return count;
}


/*
==============
CG_CenterPrint

Called for important messages that should stay in the center of the screen
for a few moments
==============
*/
void CG_CenterPrint(const char* str, int y, int charWidth) {
	char* s;

	Q_strncpyz(cg.centerPrint, str, sizeof(cg.centerPrint));

	cg.centerPrintTime = cg.time;
	cg.centerPrintY = y;
	cg.centerPrintCharWidth = charWidth;

	// count the number of lines for centering
	cg.centerPrintLines = 1;
	s = cg.centerPrint;
	while (*s) {
		if (*s == '\n')
			cg.centerPrintLines++;
		s++;
	}
}


/*
===================
CG_DrawCenterString
===================
*/
static void CG_DrawCenterString(void) {
	char* start;
	int		l;
	int		x, y, w;
#ifdef MISSIONPACK // bk010221 - unused else
	int h;
#endif
	float* color;

	if (!cg.centerPrintTime) {
		return;
	}

	color = CG_FadeColor(cg.centerPrintTime, 1000 * cg_centertime.value);
	if (!color) {
		return;
	}

	trap_R_SetColor(color);

	start = cg.centerPrint;

	y = cg.centerPrintY - cg.centerPrintLines * BIGCHAR_HEIGHT / 2;

	while (1) {
		char linebuffer[1024];

		for (l = 0; l < 50; l++) {
			if (!start[l] || start[l] == '\n') {
				break;
			}
			linebuffer[l] = start[l];
		}
		linebuffer[l] = 0;

#ifdef MISSIONPACK
		w = CG_Text_Width(linebuffer, 0.5, 0);
		h = CG_Text_Height(linebuffer, 0.5, 0);
		x = (SCREEN_WIDTH - w) / 2;
		CG_Text_Paint(x, y + h, 0.5, color, linebuffer, 0, 0, ITEM_TEXTSTYLE_SHADOWEDMORE);
		y += h + 6;
#else
		w = cg.centerPrintCharWidth * CG_DrawStrlen(linebuffer);

		x = (SCREEN_WIDTH - w) / 2;

		trap_DrawBigString(x, y, linebuffer, 1.0f);

		//CG_DrawStringExt(x, y, linebuffer, color, qfalse, qtrue,
		//	cg.centerPrintCharWidth, (int)(cg.centerPrintCharWidth * 1.5), 0);

		y += cg.centerPrintCharWidth * 1.5;
#endif
		while (*start && (*start != '\n')) {
			start++;
		}
		if (!*start) {
			break;
		}
		start++;
	}

	trap_R_SetColor(NULL);
}


/*
==================
CG_DrawSnapshot
==================
*/
static float CG_DrawSnapshot( float y ) {
	char		*s;
	int			w;

	s = va( "time:%i snap:%i cmd:%i", cg.snap->serverTime, 
		cg.latestSnapshotNum, cgs.serverCommandSequence );
	w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;

	CG_DrawBigString( 635 - w, y + 2, s, 1.0F);

	return y + BIGCHAR_HEIGHT + 4;
}


/*
==================
CG_DrawStringExt

Draws a multi-colored string with a drop shadow, optionally forcing
to a fixed color.

Coordinates are at 640 by 480 virtual resolution
==================
*/
void CG_DrawStringExt( int x, int y, const char *string, const float *setColor, 
		qboolean forceColor, qboolean shadow, int charWidth, int charHeight, int maxChars ) {
	vec4_t		color;
	const char	*s;
	int			xx;
	int			cnt;

	if (maxChars <= 0)
		maxChars = 32767; // do them all!

	// draw the drop shadow
	if (shadow) {
		color[0] = color[1] = color[2] = 0;
		color[3] = setColor[3];
		trap_R_SetColor( color );
		s = string;
		xx = x;
		cnt = 0;
		while ( *s && cnt < maxChars) {
			if ( Q_IsColorString( s ) ) {
				s += 2;
				continue;
			}
			CG_DrawChar( xx + 2, y + 2, charWidth, charHeight, *s );
			cnt++;
			xx += charWidth;
			s++;
		}
	}

	// draw the colored text
	s = string;
	xx = x;
	cnt = 0;
	trap_R_SetColor( setColor );
	while ( *s && cnt < maxChars) {
		if ( Q_IsColorString( s ) ) {
			if ( !forceColor ) {
				memcpy( color, g_color_table[ColorIndex(*(s+1))], sizeof( color ) );
				color[3] = setColor[3];
				trap_R_SetColor( color );
			}
			s += 2;
			continue;
		}
		CG_DrawChar( xx, y, charWidth, charHeight, *s );
		xx += charWidth;
		cnt++;
		s++;
	}
	trap_R_SetColor( NULL );
}

void CG_DrawBigString( int x, int y, const char *s, float alpha ) {
	float	color[4];

	color[0] = color[1] = color[2] = 1.0;
	color[3] = alpha;
	CG_DrawStringExt( x, y, s, color, qfalse, qtrue, BIGCHAR_WIDTH, BIGCHAR_HEIGHT, 0 );
}

/*
==================
CG_DrawFPS
==================
*/
#define	FPS_FRAMES	4
static float CG_DrawFPS( float y ) {
	char		*s;
	int			w;
	static int	previousTimes[FPS_FRAMES];
	static int	index;
	int		i, total;
	int		fps;
	static	int	previous;
	int		t, frameTime;

	// don't use serverTime, because that will be drifting to
	// correct for internet lag changes, timescales, timedemos, etc
	t = trap_Milliseconds();
	frameTime = t - previous;
	previous = t;

	previousTimes[index % FPS_FRAMES] = frameTime;
	index++;
	if ( index > FPS_FRAMES ) {
		// average multiple frames together to smooth changes out a bit
		total = 0;
		for ( i = 0 ; i < FPS_FRAMES ; i++ ) {
			total += previousTimes[i];
		}
		if ( !total ) {
			total = 1;
		}
		fps = 1000 * FPS_FRAMES / total;

		s = va( "%ifps", fps );
		w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;

		CG_DrawBigString( 635 - w, y + 2, s, 1.0F);
	}

	return y + BIGCHAR_HEIGHT + 4;
}

/*
=====================
CG_DrawUpperRight
=====================
*/
static void CG_DrawUpperRight( void ) {
	float	y;

	y = 0;

	if ( cg_drawSnapshot.integer ) {
		y = CG_DrawSnapshot( y );
	}
	if ( cg_drawFPS.integer ) {
		y = CG_DrawFPS( y );
	}
}

void CG_DrawStatusBar(void) {
	int			color;
	centity_t* cent;
	playerState_t* ps;
	int			value;
	vec4_t		hcolor;
	vec3_t		angles;
	vec3_t		origin;

	static float colors[4][4] = {
		//		{ 0.2, 1.0, 0.2, 1.0 } , { 1.0, 0.2, 0.2, 1.0 }, {0.5, 0.5, 0.5, 1} };
				{ 1.0f, 0.69f, 0.0f, 1.0f },    // normal
				{ 1.0f, 0.2f, 0.2f, 1.0f },     // low health
				{ 0.5f, 0.5f, 0.5f, 1.0f },     // weapon firing
				{ 1.0f, 1.0f, 1.0f, 1.0f } };   // health > 100

	if (cg_drawStatus.integer == 0) {
		return;
	}

	// draw the team background
	//CG_DrawTeamBackground(0, 420, 640, 60, 0.33f, cg.snap->ps.persistant[PERS_TEAM]);

	cent = &cg_entities[cg.snap->ps.clientNum];
	ps = &cg.snap->ps;

	VectorClear(angles);

	// draw any 3D icons first, so the changes back to 2D are minimized
	if (cent->currentState.weapon && cg_weapons[cent->currentState.weapon].ammoModel) {
		origin[0] = 70;
		origin[1] = 0;
		origin[2] = 0;
		angles[YAW] = 90 + 20 * sin(cg.time / 1000.0);
		//CG_Draw3DModel(CHAR_WIDTH * 3 + TEXT_ICON_SPACE, 432, ICON_SIZE, ICON_SIZE,
		//	cg_weapons[cent->currentState.weapon].ammoModel, 0, origin, angles);
	}

	if (ps->stats[STAT_ARMOR]) {
		origin[0] = 90;
		origin[1] = 0;
		origin[2] = -10;
		angles[YAW] = (cg.time & 2047) * 360 / 2048.0;
		//CG_Draw3DModel(370 + CHAR_WIDTH * 3 + TEXT_ICON_SPACE, 432, ICON_SIZE, ICON_SIZE,
		//	cgs.media.armorModel, 0, origin, angles);
	}

	//
	// ammo
	//
	if (cent->currentState.weapon) {
		value = ps->ammo[cent->currentState.weapon];
		if (value > -1) {
			if (cg.predictedPlayerState.weaponstate == WEAPON_FIRING
				&& cg.predictedPlayerState.weaponTime > 100) {
				// draw as dark grey when reloading
				color = 2;	// dark grey
			}
			else {
				if (value >= 0) {
					color = 0;	// green
				}
				else {
					color = 1;	// red
				}
			}
			trap_R_SetColor(colors[color]);

			CG_DrawField(0, 432, 3, value);
			trap_R_SetColor(NULL);

			// if we didn't draw a 3D icon, draw a 2D icon for ammo
			if (!cg_draw3dIcons.integer && cg_drawIcons.integer) {
				qhandle_t	icon;

				icon = cg_weapons[cg.predictedPlayerState.weapon].ammoIcon;
				if (icon) {
					CG_DrawPic(CHAR_WIDTH * 3 + TEXT_ICON_SPACE, 432, ICON_SIZE, ICON_SIZE, icon);
				}
			}
		}
	}

	//
	// health
	//
	value = ps->stats[STAT_HEALTH];
	int faceIcon = 0;
	if (value > 80)
		faceIcon = 0;
	else if (value > 60)
		faceIcon = 1;
	else if (value > 30)
		faceIcon = 2;
	else if (value <= 20 && value > 0)
		faceIcon = 3;
	else
		faceIcon = 4;

	CG_DrawPic(185 + 125 - 50, 432, 50, 50, cgs.media.faceIcon[faceIcon]);

	// stretch the health up when taking damage
	CG_DrawField(185 + 125, 432, 3, value);
	CG_ColorForHealth(hcolor);
	trap_R_SetColor(hcolor);


	//
	// armor
	//
	value = ps->stats[STAT_ARMOR];
	trap_R_SetColor(colors[0]);
	CG_DrawField(500, 432, 3, value);
	trap_R_SetColor(NULL);
	// if we didn't draw a 3D icon, draw a 2D icon for armor
	if (!cg_draw3dIcons.integer && cg_drawIcons.integer) {
		CG_DrawPic(370 + CHAR_WIDTH * 3 + TEXT_ICON_SPACE, 432, ICON_SIZE, ICON_SIZE, cgs.media.armorIcon);
	}

	CG_DrawCenterString();

	CG_DrawUpperRight();
}