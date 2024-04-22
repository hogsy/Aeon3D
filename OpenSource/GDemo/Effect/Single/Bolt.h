/****************************************************************************************/
/*  Bolt.h                                                                              */
/*                                                                                      */
/*  Author: Peter Siamidis                                                              */
/*  Description:    Electric Bolt effect                				                */
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
#ifndef BOLT_H
#define BOLT_H

#include "Genesis.h"
#include "EffectI.h"

#ifdef __cplusplus
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
//	Modification flags
////////////////////////////////////////////////////////////////////////////////////////
#define BOLT_START	( 1 << 0 )
#define BOLT_END	( 1 << 1 )
#define BOLT_COLOR	( 1 << 2 )


////////////////////////////////////////////////////////////////////////////////////////
//	Effect info
////////////////////////////////////////////////////////////////////////////////////////
extern	Effect_Interface	Bolt_Interface;
typedef struct Bolt
{
	int			TypeID;			// RESERVED, this MUST be the first item in the struct
	void		*ReservedData;	// RESERVED
	geBitmap	*Texture;		// art to use
	float		CompleteLife;	// life of entire bolt
	geVec3d		Start;			// starting position
	geVec3d		End;			// ending position
	int			SegmentLength;	// length of each bolt segment
	float		SegmentWidth;	// width of each bolt segment
	float		Offset;			// how much each bolt will vary off the most direct path
	int			BoltLimit;		// max number of individual bolts that can exist
	float		BoltLife;		// how low each bolt can last (in seconds)
	float		BoltCreate;		// every how many seconds to create a new bolt
	geBoolean	Loop;			// whether or not the lighting loops continously
	GE_RGBA		Color;			// color of the complete electric bolt

} Bolt;


#ifdef __cplusplus
	}
#endif

#endif
