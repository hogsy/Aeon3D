////////////////////////////////////////////////////////////////////////////////////////
//  EffectC.h									                                        
//                                                                                      
//  Author: Peter Siamidis                                                              
//  Description:    
//				Common functions used by all effect stuff.
//                                                                                      
//  The contents of this file are subject to the Genesis3D Public License               
//  Version 1.01 (the "License"); you may not use this file except in                   
//  compliance with the License. You may obtain a copy of the License at                
//  http://www.genesis3d.com                                                            
//                                                                                      
//  Software distributed under the License is distributed on an "AS IS"                 
//  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See                
//  the License for the specific language governing rights and limitations              
//  under the License.                                                                  
//                                                                                      
//  Copyright (C) 1996-1999 Eclipse Entertainment, L.L.C. All Rights Reserved           
//                                                                                      
////////////////////////////////////////////////////////////////////////////////////////
#ifndef EFFECTC_H
#define EFFECTC_H

#include "Genesis.h"

#ifdef __cplusplus
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
//	Clip styles
////////////////////////////////////////////////////////////////////////////////////////
#define EFFECTC_CLIP_LEAF			( 1 << 0 )
#define EFFECTC_CLIP_LINEOFSIGHT	( 1 << 1 )
#define EFFECTC_CLIP_SEMICIRCLE		( 1 << 2 )


////////////////////////////////////////////////////////////////////////////////////////
//	Prototypes
////////////////////////////////////////////////////////////////////////////////////////

//	Picks a random float within the supplied range.
//
////////////////////////////////////////////////////////////////////////////////////////
extern float EffectC_Frand(
	float Low,		// minimum value
	float High );	// maximum value

//	Create a transform from two vectors.
//
////////////////////////////////////////////////////////////////////////////////////////
extern void EffectC_XFormFromVector(
	geVec3d		*Source,	// source point
	geVec3d		*Target,	// where we are looking at
	geXForm3d	*Out );		// resultant transformation

//	Returns true if point is visible, false if it isn't.
//
////////////////////////////////////////////////////////////////////////////////////////
extern geBoolean EffectC_IsPointVisible(
	geWorld		*World,			// world in which to party
	geCamera	*Camera,		// camera location data
	geVec3d		*Target,		// target point
	int32		Leaf,			// target leaf
	uint32		ClipStyle );	// clipping styles to use

//	Get a bones correct transform.
//
////////////////////////////////////////////////////////////////////////////////////////
extern geBoolean EffectC_GetBoneLoc(
	geActor		*pActor,	// actor in which bone exists
	const char	*BoneName,	// name of bone
	geXForm3d	*Kw );		// where to store transform

//	Rotate a transform.
//
////////////////////////////////////////////////////////////////////////////////////////
void EffectC_XfRotateX(
	geXForm3d	*XfIn,		// xf to rotatte
	float		Radians,	// how much ro rotate it
	geXForm3d	*XfOut );	// where to store new xf

//	Determines if a string is NULL, accounting for additional genesis editor posibilities.
//
////////////////////////////////////////////////////////////////////////////////////////
geBoolean EffectC_IsStringNull(
	char	*String );	// string to check

//	Checks if a color struct contains valid data.
//
////////////////////////////////////////////////////////////////////////////////////////
geBoolean EffectC_IsColorGood(
	GE_RGBA	*Color );	// color to check


#ifdef __cplusplus
	}
#endif

#endif
