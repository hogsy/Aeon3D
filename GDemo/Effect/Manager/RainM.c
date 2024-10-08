////////////////////////////////////////////////////////////////////////////////////////
//	RainM.c
//                                                                                      
//  Author: Peter Siamidis                                                              
//  Description:    
//		A rain storm effect.
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
//	Hardwired stuff
////////////////////////////////////////////////////////////////////////////////////////
#define RAINM_RADIUSTOEFFECTRATIO	50


////////////////////////////////////////////////////////////////////////////////////////
//	Custom data
////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	geActor	*Actor;

} Custom;


////////////////////////////////////////////////////////////////////////////////////////
//	Interface setup
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean	RainM_Create( EffectManager *EManager, EffectM_Root *Root );
static void			RainM_Destroy( EffectManager *EManager, EffectM_Root *Root );
static geBoolean	RainM_Update( EffectManager *EManager, EffectM_Root *Root );
EffectM_Interface	RainM_Interface =
{
	RainM_Create,
	RainM_Destroy,
	RainM_Update
};



////////////////////////////////////////////////////////////////////////////////////////
//
//	RainM_Create()
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean RainM_Create(
	EffectManager	*EManager,	// effect manager to add it to
	EffectM_Root	*Root )		// root data
{

	// locals
	EM_Rain	*Rain;
	Spray	Sp;
	int		i;
	Custom	*Cus;

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );

	// get data
	Rain = Root->UserData;
	assert( Rain != NULL );
	assert( Root->UserDataSize == sizeof( *Rain ) );
	
	// don't launch this effect if its trigger based
	if ( EffectC_IsStringNull( Rain->TriggerName ) == GE_FALSE )
	{
		return GE_FALSE;
	}

	// properly null stuff thats been messed up by the editor
	if ( EffectC_IsStringNull( Rain->AlphaName ) == GE_TRUE )
	{
		Rain->AlphaName = NULL;
	}

	// init remaining root data
	assert( Rain->Radius > 0.0f );
	Root->EffectCount = (int)Rain->Radius / RAINM_RADIUSTOEFFECTRATIO;
	assert( Root->EffectCount > 0 );
	Root->EffectList = geRam_Allocate( sizeof( int ) * Root->EffectCount );
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

	// setup default spray data
	memset( &Sp, 0, sizeof( Sp ) );
	Rain->TextureNumber = TPool_Add( EManager->TPool, NULL, Rain->BmpName, Rain->AlphaName );
	if ( Rain->TextureNumber == -1 )
	{
		return -1;
	}
	Sp.Texture = TPool_Get( EManager->TPool, Rain->TextureNumber );
	if ( Sp.Texture == NULL )
	{
		return GE_FALSE;
	}
	Sp.MinScale = 0.5f;
	Sp.MaxScale = 1.5f;
	assert( Rain->Severity >= 0.0f );
	assert( Rain->Severity <= 1.0f );
	Sp.Rate = ( 1.1f - Rain->Severity ) * 0.1f;
	assert( Sp.Rate > 0.0f );
	geVec3d_Copy( &( Rain->Gravity ), &( Sp.Gravity ) );
	geVec3d_Copy( &( Rain->Position ), &( Sp.Source ) );
	Sp.SourceVariance = (int)( Rain->Radius / 2.0f );
	assert( Rain->DropLife > 0.0f );
	Sp.MinUnitLife = Rain->DropLife;
	Sp.MaxUnitLife = Rain->DropLife;
	geVec3d_AddScaled( &( Sp.Source ), &( Sp.Gravity ), Sp.MinUnitLife, &( Sp.Dest ) );
	Sp.DestVariance = (int)( Rain->Radius / 2.0f );
	memcpy( &( Sp.ColorMin ), &( Rain->ColorMin ), sizeof( Sp.ColorMin ) );
	memcpy( &( Sp.ColorMax ), &( Rain->ColorMax ), sizeof( Sp.ColorMax ) );
	Sp.ColorMin.a = Sp.ColorMax.a = 255.0f;

	// setup position based on actor bone
	if ( ( Rain->AStart != NULL ) && ( Rain->AStart->Actor != NULL ) )
	{

		// locals
		geXForm3d	Xf;
		geBoolean	Result;

		// save actor that its locked to
		Cus->Actor = Rain->AStart->Actor;

		// get bone location
		Result = geActor_GetBoneTransform( Cus->Actor, "EFFECT_BONE01", &Xf );
		if ( Result == GE_FALSE )
		{
			return GE_FALSE;
		}
		geVec3d_Copy( &( Xf.Translation ), &( Sp.Source ) );
		geVec3d_AddScaled( &( Sp.Source ), &( Sp.Gravity ), Sp.MinUnitLife, &( Sp.Dest ) );
	}

	// add drop effects
	for ( i = 0; i < Root->EffectCount; i++ )
	{
		if ( Effect_New( EManager->ESystem, Effect_Spray, &Sp, 0, NULL, NULL, &( Root->EffectList[i] ) ) == GE_FALSE )
		{
			return GE_FALSE;
		}
	}

	// all done
	return GE_TRUE;

} // RainM_Create()



////////////////////////////////////////////////////////////////////////////////////////
//
//	RainM_Destroy()
//
////////////////////////////////////////////////////////////////////////////////////////
static void RainM_Destroy(
	EffectManager	*EManager,	// effect manager in which it exists
	EffectM_Root	*Root )		// root data
{

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );

	// all done
	return;

	// get rid of warnings
	EManager;
	Root;

} // RainM_Destroy()



////////////////////////////////////////////////////////////////////////////////////////
//
//	RainM_Update()
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean RainM_Update(
	EffectManager	*EManager,	// effect manager to which it belongs
	EffectM_Root	*Root )		// root data
{

	// locals
	Custom	*Cus;

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );

	// get custom data
	Cus = Root->Custom;
	assert( Cus != NULL );

	// adjust position if its locked to an actor
	if ( Cus->Actor != NULL )
	{

		// locals
		geXForm3d	Xf;
		geBoolean	Result;
		Spray		Sp;
		int			i;

		// get bone location
		Result = geActor_GetBoneTransform( Cus->Actor, "EFFECT_BONE01", &Xf );
		if ( Result == GE_FALSE ) 
		{
			return GE_FALSE;
		}
		geVec3d_Copy( &( Xf.Translation ), &( Sp.Source ) );
		geVec3d_AddScaled( &( Sp.Source ), &( Sp.Gravity ), Sp.MinUnitLife, &( Sp.Dest ) );

		// adjust position
		for ( i = 0; i < Root->EffectCount; i++ )
		{
			Result = Effect_Modify(	EManager->ESystem,
									Effect_Spray,
									&Sp,
									Root->EffectList[i],
									SPRAY_SOURCE | SPRAY_DEST );
			assert( Result == GE_TRUE );
		}
	}

	// all done
	return GE_TRUE;

} // RainM_Update()
