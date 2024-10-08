/****************************************************************************************/
/*  Corona.h                                                                            */
/*                                                                                      */
/*  Author: Peter Siamidis                                                              */
/*  Description:    Displays a bitmap which fades when out of view.						*/
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
#ifndef CORONA_H
#define CORONA_H

#include "Genesis.h"
#include "EffectI.h"

#ifdef __cplusplus
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
// Data which can be modified
////////////////////////////////////////////////////////////////////////////////////////
#define CORONA_POS	( 1 << 0 )


////////////////////////////////////////////////////////////////////////////////////////
// Corona info
////////////////////////////////////////////////////////////////////////////////////////
extern	Effect_Interface	Corona_Interface;
typedef struct
{
	int			TypeID;				// RESERVED, this MUST be the first item in the struct
	float		LastVisibleRadius;	// RESERVED, last visible radius
	int32		Leaf;				// RESERVED, leaf it resides in
	geBoolean	Paused;				// RESERVED, whether or not the sprite is paused
	geBitmap	*Texture;			// texture to use
	GE_LVertex	Vertex;				// location and color info
	geFloat		FadeTime;			// how many seconds to spend fading away the corona
    float		MinRadius;			// mix corona radius
    float		MaxRadius;			// max corona radius
	float		MaxRadiusDistance;	// above this distance, corona is capped at MaxRadius
	float		MinRadiusDistance;	// below this distance, corona is capped at MinRadius
	float		MaxVisibleDistance;	// beyond this distance the corona is not visible

} Corona;


#ifdef __cplusplus
	}
#endif

#endif
