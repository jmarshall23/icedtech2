Darklight 2.0
https://www.quakertx.com/

Installation:
	1) You need visual studio 2019 to compile(only if compiling source). 
	2) Copy the retail .pak files from Quake 1 into baseq3 folder. 

Introduction:
	Darklight 2.0 is a idTech 3 project that is designed to create
a simple and easy to use platform for people to create new singleplayer 
games on.

Long story:
	The Quake 3 engine was a difficult engine to make a new game on.
The ai code for Quake 3 was heavily complex, the state machines were
difficult to work with, lots of singleplayer functionality was missing,
(like keys) etc. Return to Castle Wolfenstein GPL release, didn't really
help the situation either. That platform was built upon the same complex,
AI system that made Quake 3 hard to work with.

	I loved the Quake 3 engine, the engine code was pretty nice and
straight forward to work with. However if you can't make a game on it,
it doesn't matter if the engine was awesome to work with; and that's 
what this project is designed to fix. 

	I ported the Quake 2 native AI to Quake 3, I created a pipeline to 
import monster .qc files from Quake 1 into Quake 3 via a straight forward 
conversion pipeline called "superscript". There are some unique things
to know about qc in superscript.

Superscript:
	Superscript is consists of qc files, a C# executable and a C++ static library.
The C# executable takes all the .qc files in the superscript folder and converts them
to C++(technically C, but I do use splines for vector/matrix math). Some things about
super script:

	1) All animation types for $frame need to be on the same line. E.g. all the walk
	   frames need to be on the same line. This is so the cgame module can do client
	   side animation(I assume all frames on the same line are part of the same animation).
	   So:
		$frame walk1 walk2 walk3
	   NOT: 
		$frame walk1
		$frame walk2 walk3
	2) Vectors in QuakeC used to be defined as '1 1 1'(for example), in SuperScript vectors
	   are defined as idVec3_t(1, 1, 1).
	3) SuperScript is mostly designed to let you design a "ai/animation blueprint", more complex
	   functionality should be created in the .c files. 

Item Code:
	The entire item system in Quake 3 has been reworked(no more bg_misc.c), as a result please take a look at the new code.

Quake Shareware Content:
	Darklight 2.0 as a "example" project uses converted Quake 1 shareware content. This is copyrighted
material. Please take a look at assets_readme.txt and quake_copyright_assets.pk3. As a result, you will need 
the .pak files from the retail version of the game. You will need to purchase Quake 1 and put those files 
inside of the baseq3 folder. The example project will only support the shareware version of the game. 

Quake GPL maps:
	This project uses the Quake 1 GPL map files, and are supplied in quake_gpl_maps.pk3. Some maps have been
modified to support area lights(sky/lava). 

Raytracing:
	Darklight 2.0 only uses rasterization to render UI elements. All of the graphics rendering pipeline
uses raytracing. You will need a raytracing compatible GPU to run Darklight 2.0. We support real time area lights,
with shadows, point lights with shadows and reflections. 

Making new maps:
	We use q3map2 to compile the .map files to .bsp. We only use the bsp and vis pass. Since all of the raytracing,
is real time, do not use the lightmap pass. 

Other projects:
	This projected started with the vanilla version of Quake 3 arena. However we do use some code from iortcw,
like the audio engine. 

	UI code is integrated into the engine(no longer as a DLL), and the code is from RTCW.

State:
	All shareware maps are converted and included. However you can only play without cheats to e1m3. Shambler,
Wizard and Boss are missing right now.


