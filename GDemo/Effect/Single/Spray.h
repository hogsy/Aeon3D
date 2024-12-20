/****************************************************************************************/
/*  Spray.h										                                        */
/*                                                                                      */
/*  Author: Peter Siamidis                                                              */
/*  Description:    Create a particle spray												*/
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
#ifndef SPRAY_H
#define SPRAY_H

#include "Genesis.h"
#include "EffectI.h"

#ifdef __cplusplus
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
// Data which can be modified
////////////////////////////////////////////////////////////////////////////////////////
#define SPRAY_SOURCE		( 1 << 0 )
#define SPRAY_DEST			( 1 << 1 )
#define SPRAY_FOLLOWTAIL	( 1 << 2 )


////////////////////////////////////////////////////////////////////////////////////////
// Effect info
////////////////////////////////////////////////////////////////////////////////////////
extern	Effect_Interface	Spray_Interface;
typedef struct
{
	int			TypeID;				// RESERVED, this MUST be the first item in the struct
	geVec3d		*ParticleGravity;	// RESERVED
	float		TimeRemaining;		// RESERVED
	float		PolyCount;			// RESERVED
	geXForm3d	Xf;					// RESERVED
	GE_LVertex	Vertex;				// RESERVED
	geBoolean	Paused;				// RESERVED, whether or not the sprite is paused
	int32		Leaf;				// RESERVED
	geBitmap	*Texture;			// texture to use
	float		SprayLife;			// life of effect
	float		Rate;				// add a new texture every "Rate" seconds
	geVec3d		*AnchorPoint;		// point to which particles are hooked to
	GE_RGBA		ColorMin;			// min values for each color
	GE_RGBA		ColorMax;			// max values for each color
	geVec3d		Gravity;			// gravity vector
	float		DistanceMax;		// distance past which the effect is not drawn
	float		DistanceMin;		// distance up to which no level of detail processing is done
	geVec3d		Source;				// source point
	int			SourceVariance;		// +/- units to vary the source point
	geVec3d		Dest;				// dest point
	int			DestVariance;		// +/- units to vary the dest point
	float		MinScale;			// min scale for the art
	float		MaxScale;			// max scale for the art
	float		MinUnitLife;		// min life of each texture
	float		MaxUnitLife;		// max life of each texture
	float		MinSpeed;			// min speed of each texture
	float		MaxSpeed;			// max speed of each texture

} Spray;


#ifdef __cplusplus
	}
#endif

#endif
