////////////////////////////////////////////////////////////////////////////////////////
//	RedWispM.c
//                                                                                      
//  Author: Peter Siamidis                                                              
//  Description:    
//		A glowing red wisp
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

////////////////////////////////////////////////////////////////////////////////////////
//	-Actor:			OPTIONAL, if present effect gets locked to it
//	-User Data:		REQUIRED, all the settings are here
//	-Editor use:	SUPPORTED
//	-for effect manager use only
////////////////////////////////////////////////////////////////////////////////////////
#pragma warning ( disable : 4514 )
#include <memory.h>
#include <assert.h>
#include "Ram.h"
#include "EffectMI.h"
#include "EffectC.h"


////////////////////////////////////////////////////////////////////////////////////////
//	Custom data
////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	geActor		*Actor;

} Custom;


////////////////////////////////////////////////////////////////////////////////////////
//	Interface setup
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean	RedWispM_Create( EffectManager *EManager, EffectM_Root *Root );
static void			RedWispM_Destroy( EffectManager *EManager, EffectM_Root *Root );
static geBoolean	RedWispM_Update( EffectManager *EManager, EffectM_Root *Root );
EffectM_Interface	RedWispM_Interface =
{
	RedWispM_Create,
	RedWispM_Destroy,
	RedWispM_Update
};



////////////////////////////////////////////////////////////////////////////////////////
//
//	RedWispM_Create()
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean RedWispM_Create(
	EffectManager	*EManager,	// effect manager to add it to
	EffectM_Root	*Root )		// root data
{

	// locals
	EM_RedWisp	*Wisp;
	Glow		Gl;
	Spray		Sp;
	int			TextureNumber;
	geVec3d		Pos;
	Custom		*Cus;
	int			i;

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );

	// get user data
	Wisp = Root->UserData;
	assert( Wisp != NULL );
	assert( Root->UserDataSize == sizeof( *Wisp ) );

	// properly null stuff thats been messed up by the editor
	if ( EffectC_IsStringNull( Wisp->CenterAlphaBmp ) == GE_TRUE )
	{
		Wisp->CenterAlphaBmp = NULL;
	}
	if ( EffectC_IsStringNull( Wisp->FluffAlphaBmp ) == GE_TRUE )
	{
		Wisp->FluffAlphaBmp = NULL;
	}

	// init remaining root data
	Root->EffectCount = 3;
	Root->EffectList = geRam_Allocate( sizeof( uint32 ) * Root->EffectCount );
	if ( Root->EffectList == NULL )
	{
		return GE_FALSE;
	}
	for ( i = 0; i < Root->EffectCount; i++ )
	{
		Root->EffectList[i] = -1;
	}

	// init custom data
	Root->Custom = geRam_AllocateClear( sizeof( Custom ) );
	if ( Root->Custom == NULL )
	{
		return GE_FALSE;
	}
	Cus = Root->Custom;

	// setup source position based on actor bone...
	if ( ( ( Wisp->AStart != NULL ) && ( Wisp->AStart->Actor != NULL ) ) )
	{

		// locals
		geXForm3d	Xf;
		geBoolean	Result;

		// save actor that its locked to
		Cus->Actor = Wisp->AStart->Actor;

		// get bone location
		Result = geActor_GetBoneTransform( Cus->Actor, "EFFECT_BONE01", &Xf );
		if ( Result == GE_FALSE )
		{
			return GE_FALSE;
		}
		geVec3d_Copy( &( Xf.Translation ), &( Pos ) );
	}
	// ...or without actor bone
	{
		geVec3d_Copy( &( Wisp->Position ), &( Pos ) );
	}

	// create light
	memset( &Gl, 0, sizeof( Gl ) );
	geVec3d_Copy( &( Pos ), &( Gl.Pos ) );
	Gl.RadiusMin = 100.0f;
	Gl.RadiusMax = 150.0f;
	Gl.ColorMin.r = 128.0f;
	Gl.ColorMax.r = 255.0f;
	Gl.ColorMin.g = 0.0f;
	Gl.ColorMax.g = 0.0f;
	Gl.ColorMin.b = 0.0f;
	Gl.ColorMax.b = 0.0f;
	Gl.Intensity = 1.0f;
	if ( Effect_New( EManager->ESystem, Effect_Glow, &Gl, 0, NULL, NULL, &( Root->EffectList[0] ) ) == GE_FALSE )
	{
		return GE_FALSE;
	}

	// create fluff particles
	memset( &Sp, 0, sizeof( Sp ) );
	Sp.ColorMin.r = 252.0f;
	Sp.ColorMax.r = 252.0f;
	Sp.ColorMin.g = 132.0f;
	Sp.ColorMax.g = 170.0f;
	Sp.ColorMin.b = 4.0f;
	Sp.ColorMax.b = 80.0f;
	Sp.ColorMin.a = 255.0f;
	Sp.ColorMax.a = 255.0f;
	Sp.Rate = 0.03f;
	Sp.SourceVariance = 2;
	Sp.DestVariance = 25;
	Sp.MinScale = 0.25f;
	Sp.MaxScale = 0.55f;
	Sp.MinSpeed = 4.0f;
	Sp.MaxSpeed = 12.0f;
	Sp.MinUnitLife = 0.8f;
	Sp.MaxUnitLife = 1.2f;
	geVec3d_Copy( &( Pos ), &( Sp.Source ) );
	geVec3d_Copy( &( Pos ), &( Sp.Dest ) );
	Sp.Dest.Y += 50.0f;
	TextureNumber = TPool_Add( EManager->TPool, NULL, Wisp->FluffBmp, Wisp->FluffAlphaBmp );
	if ( TextureNumber == -1 )
	{
		return GE_FALSE;
	}
	Sp.Texture = TPool_Get( EManager->TPool, TextureNumber );
	if ( Sp.Texture == NULL )
	{
		return GE_FALSE;
	}
	if ( Effect_New( EManager->ESystem, Effect_Spray, &Sp, 0, NULL, NULL, &( Root->EffectList[1] ) ) == GE_FALSE )
	{
		return GE_FALSE;
	}

	// create center particles
	Sp.ColorMin.r = 100.0f;
	Sp.ColorMax.r = 240.0f;
	Sp.ColorMin.g = 15.0f;
	Sp.ColorMax.g = 30.0f;
	Sp.ColorMin.b = 15.0f;
	Sp.ColorMax.b = 30.0f;
	Sp.ColorMin.a = 130.0f;
	Sp.ColorMax.a = 255.0f;
	Sp.Rate = 0.03f;
	Sp.SourceVariance = 2;
	Sp.DestVariance = 25;
	Sp.MinScale = 0.15f;
	Sp.MaxScale = 0.8f;
	Sp.MinSpeed = 2.0f;
	Sp.MaxSpeed = 6.0f;
	Sp.MinUnitLife = 0.4f;
	Sp.MaxUnitLife = 0.6f;
	TextureNumber = TPool_Add( EManager->TPool, NULL, Wisp->CenterBmp, Wisp->CenterAlphaBmp );
	if ( TextureNumber == -1 )
	{
		return GE_FALSE;
	}
	Sp.Texture = TPool_Get( EManager->TPool, TextureNumber );
	if ( Sp.Texture == NULL )
	{
		return GE_FALSE;
	}
	if ( Effect_New( EManager->ESystem, Effect_Spray, &Sp, 0, NULL, NULL, &( Root->EffectList[2] ) ) == GE_FALSE )
	{
		return GE_FALSE;
	}

	// all done
	return GE_TRUE;

} // RedWispM_Create()



////////////////////////////////////////////////////////////////////////////////////////
//
//	RedWispM_Destroy()
//
////////////////////////////////////////////////////////////////////////////////////////
static void RedWispM_Destroy(
	EffectManager	*EManager,	// effect manager in which it exists
	EffectM_Root	*Root )		// root data
{

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );

	// all done
	return;

} // RedWispM_Destroy()



////////////////////////////////////////////////////////////////////////////////////////
//
//	RedWispM_Update()
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean RedWispM_Update(
	EffectManager	*EManager,	// effect manager to add it to
	EffectM_Root	*Root )		// root data
{

	// locals
	Custom		*Cus;

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );

	// get custom data
	Cus = Root->Custom;
	assert( Cus != NULL );

	// setup source position based on actor bone...
	if ( Cus->Actor != NULL )
	{

		// locals
		geXForm3d	Xf;
		geBoolean	Result;
		Glow		Gl;
		Spray		Sp;

		// get bone location
		Result = geActor_GetBoneTransform( Cus->Actor, "EFFECT_BONE01", &Xf );
		if ( Result == GE_FALSE )
		{
			return GE_FALSE;
		}

		// adjust light
		geVec3d_Copy( &( Xf.Translation ), &( Gl.Pos ) );
		Result = Effect_Modify(	EManager->ESystem,
								Effect_Glow,
								&Gl,
								Root->EffectList[0],
								GLOW_POS );
		assert( Result == GE_TRUE );

		// adjust all particles
		geVec3d_Copy( &( Xf.Translation ), &( Sp.Source ) );
		geVec3d_Copy( &( Xf.Translation ), &( Sp.Dest ) );
		Sp.Dest.Y += 50.0f;
		Result = Effect_Modify(	EManager->ESystem,
								Effect_Spray,
								&Sp,
								Root->EffectList[1],
								SPRAY_SOURCE | SPRAY_DEST );
		assert( Result == GE_TRUE );
		Result = Effect_Modify(	EManager->ESystem,
								Effect_Spray,
								&Sp,
								Root->EffectList[2],
								SPRAY_SOURCE | SPRAY_DEST );
		assert( Result == GE_TRUE );
	}

	// all done
	return GE_TRUE;

} // RedWispM_Update()
