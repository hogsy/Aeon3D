////////////////////////////////////////////////////////////////////////////////////////
//  EffectC.c									                                        
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
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "Genesis.h"
#include "EffectC.h"



////////////////////////////////////////////////////////////////////////////////////////
//
//	EffectC_Frand()
//
//	Picks a random float within the supplied range.
//
////////////////////////////////////////////////////////////////////////////////////////
float EffectC_Frand(
	float Low,		// minimum value
	float High )	// maximum value
{

	// locals
	float	Range;

	// ensure valid data
	assert( High >= Low );

	// if they are the same then just return one of them
	if ( High == Low )
	{
		return Low;
	}

	// pick a random float from whithin the range
	Range = High - Low;
	return ( (float)( ( ( rand() % 1000 ) + 1 ) ) ) / 1000.0f * Range + Low;

} // EffectC_Frand()



////////////////////////////////////////////////////////////////////////////////////////
//
//	EffectC_XFormFromVector()
//
//	Create a transform from two vectors.
//
////////////////////////////////////////////////////////////////////////////////////////
void EffectC_XFormFromVector(
	geVec3d		*Source,	// source point
	geVec3d		*Target,	// where we are looking at
	geXForm3d	*Out )		// where to store resultant transformation
{

	// locals
	geVec3d	Temp, Vertical, Vect;

	// ensure valid data
	assert( Source != NULL );
	assert( Target != NULL );
	assert( Out != NULL );
	
	// create a straight up vector
	Vertical.X = 0.0f;
	Vertical.Y = 1.0f;
	Vertical.Z = 0.0f;

	// create the source vector, fudging it if its coplanar to the comparison vector
	geVec3d_Subtract( Source, Target, &Vect );
	//assert( geVec3d_Compare( Source, Target, 0.05f ) == GE_FALSE );
	if ( ( Vertical.X == Vect.X ) && ( Vertical.Z == Vect.Z ) )
	{
		Vertical.X += 1.0f;
	}

	// set the IN vector
	geXForm3d_SetIdentity( Out );
	geVec3d_Normalize( &Vect );
	Out->AZ = Vect.X;
	Out->BZ = Vect.Y;
	Out->CZ = Vect.Z;

	// use it with the in vector to get the RIGHT vector
	geVec3d_CrossProduct( &Vertical, &Vect, &Temp );
	geVec3d_Normalize( &Temp );

	// put the RIGHT vector in the matrix
	Out->AX = Temp.X;
	Out->BX = Temp.Y;
	Out->CX = Temp.Z;

	// use the RIGHT vector with the IN vector to get the real UP vector
	geVec3d_CrossProduct( &Vect, &Temp, &Vertical );
	geVec3d_Normalize( &Vertical );

	// put the UP vector in the matrix
	Out->AY = Vertical.X;
	Out->BY = Vertical.Y;
	Out->CY = Vertical.Z;
	
	// put the translation in
	Out->Translation = *Source;

} // EffectC_XFormFromVector()



////////////////////////////////////////////////////////////////////////////////////////
//
//	EffectC_IsPointVisible()
//
//	Returns true if point is visible, false if it isn't.
//
////////////////////////////////////////////////////////////////////////////////////////
geBoolean EffectC_IsPointVisible(
	geWorld		*World,			// world in which to party
	geCamera	*Camera,		// camera location data
	geVec3d		*Target,		// target point
	int32		Leaf,			// target leaf
	uint32		ClipStyle )		// clipping styles to use
{

	// ensure valid data
	assert( World != NULL );
	assert( Camera != NULL );
	assert( Target != NULL );

	// leaf check
	if ( ClipStyle & EFFECTC_CLIP_LEAF )
	{
		if ( geWorld_MightSeeLeaf( World, Leaf ) == GE_FALSE )
		{
			return GE_FALSE;
		}
	}

	// semi circle check
	if ( ClipStyle & EFFECTC_CLIP_SEMICIRCLE )
	{

		// locals
		const geXForm3d	*CameraXf;
		geVec3d			In;
		geVec3d			Delta;
		float			Dot;

		// get camera xform
		CameraXf = geCamera_GetWorldSpaceXForm( Camera );

		// get angle between camera in vector and vector to target
		geVec3d_Subtract( Target, &( CameraXf->Translation ), &Delta );
		geVec3d_Normalize( &Delta );
		geXForm3d_GetIn( CameraXf, &In );
		Dot = geVec3d_DotProduct( &In, &Delta );

		// check if its visible
		if ( Dot < 0.0f )
		{
			return GE_FALSE;
		}
	}

	// line of sight check
	if ( ClipStyle & EFFECTC_CLIP_LINEOFSIGHT )
	{

		// locals
		GE_Collision	Collision;
		const geXForm3d	*CameraXf;

		// get camera xform
		CameraXf = geCamera_GetWorldSpaceXForm( Camera );

		// check if its visible
		if ( geWorld_Collision( World, NULL, NULL, &( CameraXf->Translation ), Target, GE_CONTENTS_SOLID, GE_COLLIDE_MODELS, 0, NULL, NULL, &Collision ) == GE_TRUE )
		{
			return GE_FALSE;
		}
	}

	// if we got to here then its visible
	return GE_TRUE;

} // EffectC_IsPointVisible()



////////////////////////////////////////////////////////////////////////////////////////
//
//	EffectC_GetBoneLoc()
//
//	Get a bones correct transform.
//
////////////////////////////////////////////////////////////////////////////////////////
geBoolean EffectC_GetBoneLoc(
	geActor		*pActor,	// actor in which bone exists
	const char	*BoneName,	// name of bone
	geXForm3d	*Kw )		// where to store transform
{

	// locals
	geXForm3d Ar, ArI, L;
	geVec3d	tempTranslation; 

	// ensure valid data
	assert( pActor );
	assert( BoneName );
	assert( Kw );

	// get bone transform
	if( !geActor_GetBoneTransform( pActor, BoneName, &L ) )
	{
		return GE_FALSE;
	}

	// Locator is a child of of BIP01 I must pull out the BIP01's attchment
	tempTranslation = L.Translation;
	geVec3d_Set( &L.Translation, 0.0f, 0.0f, 0.0f );
	if( !geActor_GetBoneAttachment( pActor, "BIP01", &Ar ) )
	{
		return GE_FALSE;
	}
	geVec3d_Set( &Ar.Translation, 0.0f, 0.0f, 0.0f );
	geXForm3d_GetTranspose( &Ar, &ArI );

	geXForm3d_Multiply( &L, &ArI, Kw );

	Kw->AY = 0.0f;

	Kw->BX = 0.0f;
	Kw->BY = 1.0f;
	Kw->BZ = 0.0f;

	Kw->CY = 0.0f;

	geXForm3d_Orthonormalize( Kw );
	Kw->Translation = tempTranslation;
	return GE_TRUE;
	
} // EffectC_GetBoneLoc()



////////////////////////////////////////////////////////////////////////////////////////
//
//	EffectC_XfRotateX()
//
//	Rotate a transform.
//
////////////////////////////////////////////////////////////////////////////////////////
void EffectC_XfRotateX(
	geXForm3d	*XfIn,		// xf to rotatte
	float		Radians,	// how much ro rotate it
	geXForm3d	*XfOut )	// where to store new xf
{

	// locals
	geXForm3d	XfRot;
	geVec3d		Pos;

	// ensure valid data
	assert( XfIn != NULL );
	assert( XfOut != NULL );

	// apply rotation
	geVec3d_Copy( &( XfIn->Translation ), &Pos );
	geXForm3d_SetXRotation( &XfRot, Radians );
	geXForm3d_Multiply( XfIn, &XfRot, XfOut );
	geVec3d_Copy( &Pos, &( XfOut->Translation ) );

} // EffectC_XfRotateX()



////////////////////////////////////////////////////////////////////////////////////////
//
//	EffectC_IsStringNull()
//
//	Determines if a string is NULL, accounting for additional editor posibilities.
//
////////////////////////////////////////////////////////////////////////////////////////
geBoolean EffectC_IsStringNull(
	char	*String )	// string to check
{

	// first way
	if ( String == NULL )
	{
		return GE_TRUE;
	}

	// second way
	if ( strlen( String ) < 1 )
	{
		return GE_TRUE;
	}

	// third way
	if ( strnicmp( String, "<null>", 6 ) == 0 )
	{
		return GE_TRUE;
	}

	// fourth way
	if ( strnicmp( String, "NULL", 4 ) == 0 )
	{
		return GE_TRUE;
	}

	// if we got to here then the string is not null
	return GE_FALSE;

} // EffectC_IsStringNull()



////////////////////////////////////////////////////////////////////////////////////////
//
//	EffectC_IsColorGood()
//
//	Checks if a color struct contains valid data.
//
////////////////////////////////////////////////////////////////////////////////////////
geBoolean EffectC_IsColorGood(
	GE_RGBA	*Color )	// color to check
{

	// ensure valid data
	assert( Color != NULL );
	
	// fail if any color values are out of range
	if ( ( Color->a < 0.0f ) || ( Color->a > 255.0f ) )
	{
		return GE_FALSE;
	}
	if ( ( Color->r < 0.0f ) || ( Color->r > 255.0f ) )
	{
		return GE_FALSE;
	}
	if ( ( Color->g < 0.0f ) || ( Color->g > 255.0f ) )
	{
		return GE_FALSE;
	}
	if ( ( Color->b < 0.0f ) || ( Color->b > 255.0f ) )
	{
		return GE_FALSE;
	}

	// if we got to here then the color is good
	return GE_TRUE;

} // EffectC_IsColorGood()
