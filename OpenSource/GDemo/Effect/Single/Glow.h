/****************************************************************************************/
/*  Glow.h	                                                                            */
/*                                                                                      */
/*  Author: Peter Siamidis                                                              */
/*  Description:    Creates a light which follows a passed vector.						*/
/*					Does not support attachments										*/
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
#ifndef GLOW_H
#define GLOW_H

#include "Genesis.h"
#include "EffectI.h"

#ifdef __cplusplus
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
// Data which can be modified
////////////////////////////////////////////////////////////////////////////////////////
#define GLOW_POS		( 1 << 0 )
#define GLOW_RADIUSMIN	( 1 << 1 )
#define GLOW_RADIUSMAX	( 1 << 2 )
#define GLOW_INTENSITY	( 1 << 3 )


////////////////////////////////////////////////////////////////////////////////////////
// Effect info
////////////////////////////////////////////////////////////////////////////////////////
extern	Effect_Interface	Glow_Interface;
typedef struct
{
	int			TypeID;			// RESERVED, this MUST be the first item in the struct
	geLight		*Light;			// RESERVED
	int32		Leaf;			// RESERVED
	geVec3d		Pos;			// vector which light will follow
	float		RadiusMin;		// min light radius
	float		RadiusMax;		// max light radius
	GE_RGBA		ColorMin;		// min color info
	GE_RGBA		ColorMax;		// max color info
	float		Intensity;		// light intensity
	geBoolean	DoNotClip;		// whether or not clipping should be ignored
	geBoolean	CastShadows;	// whether or not the light should cast shadows

} Glow;


#ifdef __cplusplus
	}
#endif

#endif
