////////////////////////////////////////////////////////////////////////////////////////
//	SpoutM.c
//                                                                                      
//  Author: Peter Siamidis                                                              
//  Description:    
//		A spout that shoots particles.
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
#include <string.h>
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
	geXForm3d	Xf;
	int			TextureNumber;
	float		TimeLeft;
	float		MinPauseTime;
	float		MaxPauseTime;
	float		PauseTime;
	geBoolean	PauseState;
	char		*BoneName;

} Custom;


////////////////////////////////////////////////////////////////////////////////////////
//	Interface setup
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean	SpoutM_Create( EffectManager *EManager, EffectM_Root *Root );
static void			SpoutM_Destroy( EffectManager *EManager, EffectM_Root *Root );
static geBoolean	SpoutM_Update( EffectManager *EManager, EffectM_Root *Root );
EffectM_Interface	SpoutM_Interface =
{
	SpoutM_Create,
	SpoutM_Destroy,
	SpoutM_Update
};



////////////////////////////////////////////////////////////////////////////////////////
//
//	SpoutM_Create()
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean SpoutM_Create(
	EffectManager	*EManager,	// effect manager to add it to
	EffectM_Root	*Root )		// root data
{

	// locals
	EM_Spout	*Fl;
	Spray		Sp;
	Custom		*Cus;
	geVec3d		In;

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );

	// get user data
	Fl = Root->UserData;
	assert( Fl != NULL );
	assert( Root->UserDataSize == sizeof( *Fl ) );

	// don't launch this effect if its trigger based
	if ( EffectC_IsStringNull( Fl->TriggerName ) == GE_FALSE )
	{
		return GE_FALSE;
	}

	// properly null stuff thats been messed up by the editor
	if ( EffectC_IsStringNull( Fl->AlphaName ) == GE_TRUE )
	{
		Fl->AlphaName = NULL;
	}

	// init remaining root data
	Root->EffectCount = 1;
	Root->EffectList = geRam_Allocate( sizeof( uint32 ) * Root->EffectCount );
	if ( Root->EffectList == NULL )
	{
		return GE_FALSE;
	}
	Root->EffectList[0] = -1;

	// init custom data
	Root->Custom = geRam_AllocateClear( sizeof( Custom ) );
	if ( Root->Custom == NULL )
	{
		return GE_FALSE;
	}
	Cus = Root->Custom;
	Cus->TimeLeft = Fl->TotalLife;
	Cus->MinPauseTime = Fl->MinPauseTime;
	Cus->MaxPauseTime = Fl->MaxPauseTime;
	assert( Cus->MinPauseTime >= 0.0f );
	assert( Cus->MaxPauseTime >= Cus->MinPauseTime );
	if ( Cus->MaxPauseTime > 0.0f )
	{
		Cus->PauseTime = EffectC_Frand( Cus->MinPauseTime, Cus->MaxPauseTime );
	}

	// setup default data
	memset( &Sp, 0, sizeof( Sp ) );
	Cus->TextureNumber = TPool_Add( EManager->TPool, NULL, Fl->BmpName, Fl->AlphaName );
	if ( Cus->TextureNumber == -1 )
	{
		return GE_FALSE;
	}
	Sp.Texture = TPool_Get( EManager->TPool, Cus->TextureNumber );
	if ( Sp.Texture == NULL )
	{
		return GE_FALSE;
	}
	Sp.SourceVariance = Fl->SourceVariance;
	assert( Sp.SourceVariance >= 0 );
	Sp.DestVariance = Fl->DestVariance;
	assert( Sp.DestVariance >= 0 );
	Sp.MinScale = Fl->MinScale;
	Sp.MaxScale = Fl->MaxScale;
	assert( Sp.MinScale > 0.0f );
	assert( Sp.MaxScale >= Sp.MinScale );
	Sp.MinSpeed = Fl->MinSpeed;
	Sp.MaxSpeed = Fl->MaxSpeed;
	assert( Sp.MinSpeed > 0.0f );
	assert( Sp.MaxSpeed >= Sp.MinSpeed );
	Sp.MinUnitLife = Fl->MinUnitLife;
	Sp.MaxUnitLife = Fl->MaxUnitLife;
	assert( Sp.MinUnitLife > 0.0f );
	assert( Sp.MaxUnitLife >= Sp.MinUnitLife );
	Sp.ColorMin.r = Fl->ColorMin.r;
	assert( ( Sp.ColorMin.r >= 0.0f ) && ( Sp.ColorMin.r <= 255.0f ) );
	Sp.ColorMax.r = Fl->ColorMax.r;
	assert( ( Sp.ColorMax.r >= Sp.ColorMin.r ) && ( Sp.ColorMin.r <= 255.0f ) );
	Sp.ColorMin.g = Fl->ColorMin.g;
	assert( ( Sp.ColorMin.g >= 0.0f ) && ( Sp.ColorMin.g <= 255.0f ) );
	Sp.ColorMax.g = Fl->ColorMax.g;
	assert( ( Sp.ColorMax.g >= Sp.ColorMin.g ) && ( Sp.ColorMin.g <= 255.0f ) );
	Sp.ColorMin.b = Fl->ColorMin.b;
	assert( ( Sp.ColorMin.b >= 0.0f ) && ( Sp.ColorMin.b <= 255.0f ) );
	Sp.ColorMax.b = Fl->ColorMax.b;
	assert( ( Sp.ColorMax.b >= Sp.ColorMin.b ) && ( Sp.ColorMin.b <= 255.0f ) );
	Sp.ColorMin.a = 255.0f;
	Sp.ColorMax.a = 255.0f;
	Sp.Rate = Fl->ParticleCreateRate;
	assert( Sp.Rate > 0.0f );
	geVec3d_Copy( &( Fl->Gravity ), &( Sp.Gravity ) );

	// setup orientation
	geXForm3d_SetIdentity( &( Cus->Xf ) );
	geXForm3d_RotateX( &( Cus->Xf ), Fl->Angles.X / 57.3f );  
	geXForm3d_RotateY( &( Cus->Xf ), ( Fl->Angles.Y - 90.0f ) / 57.3f );  
	geXForm3d_RotateZ( &( Cus->Xf ), Fl->Angles.Z / 57.3f );  

	// setup source position based on actor bone...
	if ( ( ( Fl->AStart != NULL ) && ( Fl->AStart->Actor != NULL ) ) || ( Fl->Actor != NULL ) )
	{

		// locals
		geBoolean	Result;

		// save actor that its locked to
		if ( ( Fl->AStart != NULL ) && ( Fl->AStart->Actor != NULL ) )
		{
			Cus->Actor = Fl->AStart->Actor;
		}
		else
		{
			Cus->Actor = Fl->Actor;
		}

		// save bone name
		Cus->BoneName = geRam_Allocate( strlen( Fl->BoneName ) + 1 );
		if ( Cus->BoneName == NULL )
		{
			return GE_FALSE;
		}
		strcpy( Cus->BoneName, Fl->BoneName );

		// get bone location
		Result = geActor_GetBoneTransform( Cus->Actor, Cus->BoneName, &Cus->Xf );
		if ( Result == GE_FALSE )
		{
			return GE_FALSE;
		}
		geVec3d_Copy( &( Cus->Xf.Translation ), &( Sp.Source ) );
	}
	else
	// ...or without actor bone
	{
		geVec3d_Copy( &( Fl->Position ), &( Sp.Source ) );
	}

	// setup dest position
	geXForm3d_GetIn( &( Cus->Xf ), &In );
	geVec3d_Inverse( &In );
	geVec3d_AddScaled( &( Sp.Source ), &In, 50.0f, &( Sp.Dest ) );

	// add effect
	if ( Effect_New( EManager->ESystem, Effect_Spray, &Sp, 0, NULL, NULL, &( Root->EffectList[0] ) ) == GE_FALSE )
	{
		return GE_FALSE;
	}

	// all done
	return GE_TRUE;

} // SpoutM_Create()



////////////////////////////////////////////////////////////////////////////////////////
//
//	SpoutM_Destroy()
//
////////////////////////////////////////////////////////////////////////////////////////
static void SpoutM_Destroy(
	EffectManager	*EManager,	// effect manager in which it exists
	EffectM_Root	*Root )		// root data
{

	// locals
	Custom		*Cus;

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );

	// free any custom data
	Cus = Root->Custom;
	if ( Cus != NULL )
	{

		// free bone name
		if ( Cus->BoneName != NULL )
		{
			geRam_Free( Cus->BoneName );
		}
	}

	// all done
	return;

	// get rid of warnings
	EManager;

} // SpoutM_Destroy()



////////////////////////////////////////////////////////////////////////////////////////
//
//	SpoutM_Update()
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean SpoutM_Update(
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

	// kill the effect if its time has run out
	if ( Cus->TimeLeft > 0.0f )
	{
		Cus->TimeLeft -= Root->TimeDelta;
		if ( Cus->TimeLeft <= 0.0f )
		{
			Cus->TimeLeft = 0.0f;
			return GE_FALSE;
		}
	}

	// adjust pause time
	if ( Cus->PauseTime > 0.0f )
	{
		Cus->PauseTime -= Root->TimeDelta;
		if ( Cus->PauseTime <= 0.0f )
		{
			Cus->PauseTime = EffectC_Frand( Cus->MinPauseTime, Cus->MaxPauseTime );
			Cus->PauseState = !Cus->PauseState;
			Effect_SetPause( EManager->ESystem, Root->EffectList[0], Cus->PauseState );
		}
	}

	// adjust position if its hooked to an actor
	if ( Cus->Actor != NULL )
	{

		// locals
		geVec3d		In;
		geBoolean	Result;
		Spray		Sp;

		// get bone location
		Result = geActor_GetBoneTransform( Cus->Actor, Cus->BoneName, &( Cus->Xf ) );
		if ( Result == GE_FALSE )
		{
			return GE_FALSE;
		}
		geVec3d_Copy( &( Cus->Xf.Translation ), &( Sp.Source ) );
		geXForm3d_GetIn( &( Cus->Xf ), &In );
		geVec3d_Inverse( &In );
		geVec3d_AddScaled( &( Sp.Source ), &In, 50.0f, &( Sp.Dest ) );

		// adjust flame
		Result = Effect_Modify(	EManager->ESystem,
								Effect_Spray,
								&Sp,
								Root->EffectList[0],
								SPRAY_SOURCE | SPRAY_DEST );
		assert( Result == GE_TRUE );
	}

	// all done
	return GE_TRUE;

} // SpoutM_Update()
