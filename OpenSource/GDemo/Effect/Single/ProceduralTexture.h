/****************************************************************************************/
/*  ProceduralTexture.h                                                                 */
/*                                                                                      */
/*  Author: Peter Siamidis                                                              */
/*  Description:    Procedural texture													*/
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
#ifndef PROCEDURALTEXTURE_H
#define PROCEDURALTEXTURE_H

#include "Genesis.h"
#include "EffectI.h"

#ifdef __cplusplus
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
// Data which can be modified
////////////////////////////////////////////////////////////////////////////////////////
#define PROCEDURALTEXTURE_POS	( 1 << 0 )
#define PROCEDURALTEXTURE_COLOR	( 1 << 1 )


////////////////////////////////////////////////////////////////////////////////////////
// Effect info
////////////////////////////////////////////////////////////////////////////////////////
extern	Effect_Interface	ProceduralTexture_Interface;
typedef struct
{
	int				TypeID;			// RESERVED, this MUST be the first item in the struct
	gePoly			*Poly;			// RESERVED, poly pointer
	int32			VertCount;		// RESERVED, how many verts
	GE_LVertex		Vertex[4];		// location and color info
	gePoly_Type		PolyType;		// either GE_TEXTURED_POINT or GE_TEXTURED_POLY
	geBoolean		UseWorldPoly;	// specifys using a world poly instead of creating a new one
	uint32			RenderFlags;	// poly render flags
	geBitmap		*Texture;		// texture to use
	float			Scale;			// texture scale
	char			*Type;			// name of procedural to use
	char			*Parms;			// parameters to use

} ProceduralTexture;


#ifdef __cplusplus
	}
#endif

#endif
