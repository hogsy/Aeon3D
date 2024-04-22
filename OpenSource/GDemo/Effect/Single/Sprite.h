/****************************************************************************************/
/*  Sprite.h									                                        */
/*                                                                                      */
/*  Author: Peter Siamidis                                                              */
/*  Description:    Sprite effect - Cycles through a list of sprites					*/
/*                                                                                      */
/*  The contents of this file are subject to the Genesis3D Public License               */
/*  Version 1.01 (the "License"); you may not use this file except in                   */
/*  compliance with the License. You may obtain a copy of the License at                */
/*  http://www.genesis3d.com                                                            */
/*                                                                                      */
/*  Software distributed under the License is distributed on an "AS IS"                 */
/*  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See                */
/*  the License for the specific language governing rights and limitations              */
/*  under the License.                                                                  */
/*                                                                                      */
/*  Copyright (C) 1996-1999 Eclipse Entertainment, L.L.C. All Rights Reserved           */
/*                                                                                      */
/****************************************************************************************/
#ifndef SPRITE_H
#define SPRITE_H

#include "Genesis.h"
#include "EffectI.h"

#ifdef __cplusplus
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
// Data which can be modified
////////////////////////////////////////////////////////////////////////////////////////
#define SPRITE_POS		( 1 << 0 )
#define SPRITE_SCALE	( 1 << 1 )
#define SPRITE_ROTATION	( 1 << 2 )
#define SPRITE_COLOR	( 1 << 3 )


////////////////////////////////////////////////////////////////////////////////////////
//	Cycle styles
////////////////////////////////////////////////////////////////////////////////////////
typedef enum
{
	SPRITE_CYCLE_NONE,
	SPRITE_CYCLE_RESET,
	SPRITE_CYCLE_REVERSE,
	SPRITE_CYCLE_RANDOM,
	SPRITE_CYCLE_ONCE

} Sprite_CycleStyle;


////////////////////////////////////////////////////////////////////////////////////////
// Sprite info
////////////////////////////////////////////////////////////////////////////////////////
extern	Effect_Interface	Sprite_Interface;
typedef struct
{
	int					TypeID;			// RESERVED, this MUST be the first item in the struct
	GE_LVertex			Vertex[4];		// RESERVED, vert info
	int32				Direction;		// RESERVED
	float				ElapsedTime;	// RESERVED
	int32				CurrentTexture;	// RESERVED
	int32				Leaf;			// RESERVED
	geBoolean			Paused;			// RESERVED, whether or not the sprite is paused
	geVec3d				Pos;			// location
	GE_RGBA				Color;			// color
	geBitmap			**Texture;		// list of textures to use
	int32				TotalTextures;	// total number of textures in the list
	float				TextureRate;	// every how many seconds to switch to the next texture
	float				Scale;			// how to scale the art
	float				ScaleRate;		// how much to subtract from scale each second
	float				RotationRate;	// how much to add to art rotation each second (radians)
	float				AlphaRate;		// how much to subtract from alpha each second
	Sprite_CycleStyle	Style;			// how to cycle through the images
	float				Rotation;		// art rotation amount (radians)

} Sprite;



#ifdef __cplusplus
	}
#endif

#endif
