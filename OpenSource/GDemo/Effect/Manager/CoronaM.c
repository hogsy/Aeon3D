////////////////////////////////////////////////////////////////////////////////////////
//  CoronaM.c									                                        
//                                                                                      
//  Author: Peter Siamidis                                                              
//  Description:    
//			A generic corona effect.
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


extern char	Light_WorldGetLTypeCurrent(geWorld *World, int32 LType);


////////////////////////////////////////////////////////////////////////////////////////
//	Custom data
////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	geActor	*Actor;
	int		LightType;
	char	LightTypeSignal;
	char	*BoneName;

} Custom;


////////////////////////////////////////////////////////////////////////////////////////
//	Interface setup
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean	CoronaM_Create( EffectManager *EManager, EffectM_Root *Root );
static void			CoronaM_Destroy( EffectManager *EManager, EffectM_Root *Root );
static geBoolean	CoronaM_Update( EffectManager *EManager, EffectM_Root *Root );
EffectM_Interface	CoronaM_Interface =
{
	CoronaM_Create,
	CoronaM_Destroy,
	CoronaM_Update
};



////////////////////////////////////////////////////////////////////////////////////////
//
//	CoronaM_Create()
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean CoronaM_Create(
	EffectManager	*EManager,	// effect manager to add it to
	EffectM_Root	*Root )		// root data
{

	// locals
	Corona		Cor;
	EM_Corona	*GCorona;
	Custom		*Cus;

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );

	// init remaining root data
	Root->EffectCount = 1;
	Root->EffectList = geRam_Allocate( sizeof( uint32 ) * Root->EffectCount );
	if ( Root->EffectList == NULL )
	{
		return GE_FALSE;
	}
	Root->EffectList[0] = -1;

	// get data
	GCorona = Root->UserData;
	assert( GCorona != NULL );
	assert( Root->UserDataSize == sizeof( *GCorona ) );

	// don't launch this effect if its trigger based
	if ( EffectC_IsStringNull( GCorona->TriggerName ) == GE_FALSE )
	{
		return GE_FALSE;
	}

	// properly null stuff thats been messed up by the editor
	if ( EffectC_IsStringNull( GCorona->AlphaName ) == GE_TRUE )
	{
		GCorona->AlphaName = NULL;
	}
	if ( EffectC_IsStringNull( GCorona->LightTypeSignal ) == GE_TRUE )
	{
		GCorona->LightTypeSignal = NULL;
	}

	// init custom data
	Root->Custom = geRam_AllocateClear( sizeof( Custom ) );
	if ( Root->Custom == NULL )
	{
		return GE_FALSE;
	}
	Cus = Root->Custom;
	Cus->LightType = GCorona->LightType;
	if ( GCorona->LightTypeSignal != NULL )
	{
		Cus->LightTypeSignal = *( GCorona->LightTypeSignal );
		if ( ( Cus->LightTypeSignal < 65 ) || ( Cus->LightTypeSignal > 90 ) )
		{
			Cus->LightTypeSignal -= 32;
		}
		assert( Cus->LightTypeSignal >= 65 );
		assert( Cus->LightTypeSignal <= 90 );
	}
	
	// reset corona struct
	memset( &Cor, 0, sizeof( Cor ) );

	// setup corona position based on actor bone...
	if ( ( GCorona->AStart != NULL ) && ( GCorona->AStart->Actor != NULL ) )
	{

		// locals
		geXForm3d	Xf;
		geBoolean	Result;

		// save actor that its locked to
		Cus->Actor = GCorona->AStart->Actor;

		// save bone name
		Cus->BoneName = geRam_Allocate( strlen( GCorona->BoneName ) + 1 );
		if ( Cus->BoneName == NULL )
		{
			return GE_FALSE;
		}
		strcpy( Cus->BoneName, GCorona->BoneName );

		// get bone location
		Result = geActor_GetBoneTransform( Cus->Actor, Cus->BoneName, &Xf );
		if ( Result == GE_FALSE )
		{
			return GE_FALSE;
		}

		// setup corona position
		Cor.Vertex.X = Xf.Translation.X;
		Cor.Vertex.Y = Xf.Translation.Y;
		Cor.Vertex.Z = Xf.Translation.Z;
	}
	// ...or based on no actor bone
	else
	{
		Cor.Vertex.X = GCorona->Position.X;
		Cor.Vertex.Y = GCorona->Position.Y;
		Cor.Vertex.Z = GCorona->Position.Z;
	}

	// create corona
	GCorona->TextureNumber = TPool_Add( EManager->TPool, NULL, GCorona->BmpName, GCorona->AlphaName );
	if ( GCorona->TextureNumber == -1 )
	{
		return -1;
	}
	Cor.Texture = TPool_Get( EManager->TPool, GCorona->TextureNumber );
	if ( Cor.Texture == NULL )
	{
		return GE_FALSE;
	}
	Cor.Vertex.r = GCorona->Color.r;
	Cor.Vertex.g = GCorona->Color.g;
	Cor.Vertex.b = GCorona->Color.b;
	Cor.Vertex.a = 255.0f;
	assert( Cor.Vertex.r >= 0.0f );
	assert( Cor.Vertex.r <= 255.0f );
	assert( Cor.Vertex.g >= 0.0f );
	assert( Cor.Vertex.g <= 255.0f );
	assert( Cor.Vertex.b >= 0.0f );
	assert( Cor.Vertex.b <= 255.0f );
	Cor.FadeTime = GCorona->FadeTime;
	assert( Cor.FadeTime >= 0.0f );
	Cor.MinRadius = GCorona->MinRadius;
	assert( Cor.MinRadius >= 0.0f );
	Cor.MaxRadius = GCorona->MaxRadius;
	assert( Cor.MaxRadius >= 0.0f );
	Cor.MinRadiusDistance = GCorona->MinRadiusDistance;
	assert( Cor.MinRadiusDistance >= 0.0f );
	Cor.MaxRadiusDistance = GCorona->MaxRadiusDistance;
	assert( Cor.MaxRadiusDistance >= 0.0f );
	Cor.MaxVisibleDistance = GCorona->MaxVisibleDistance;
	assert( Cor.MaxVisibleDistance >= 0.0f );
	if ( Effect_New( EManager->ESystem, Effect_Corona, &Cor, 0, NULL, NULL, &( Root->EffectList[0] ) ) == GE_FALSE )
	{
		return GE_FALSE;
	}

	// all done
	return GE_TRUE;

} // CoronaM_Create()



////////////////////////////////////////////////////////////////////////////////////////
//
//	CoronaM_Destroy()
//
////////////////////////////////////////////////////////////////////////////////////////
static void CoronaM_Destroy(
	EffectManager	*EManager,	// effect manager in which it exists
	EffectM_Root	*Root )		// root data
{

	// locals
	Custom	*Cus;

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

} // CoronaM_Destroy()



////////////////////////////////////////////////////////////////////////////////////////
//
//	CoronaM_Update()
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean CoronaM_Update(
	EffectManager	*EManager,	// effect manager to add it to
	EffectM_Root	*Root )		// root data
{

	// locals
	Custom	*Cus;

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );
	assert( Root->EffectList[0] >= 0 );

	// get custom data
	Cus = Root->Custom;
	assert( Cus != NULL );

	// if this corona is hooked to a light type then check if the corona needs to be turned off
	if ( Cus->LightType != -1 )
	{

		// locals
		char	Signal;

		// lights current state
		assert( Cus->LightType >= 0 );
		Signal = Light_WorldGetLTypeCurrent( Effect_GetWorld( EManager->ESystem ), Cus->LightType );
		if ( ( Signal < 65 ) || ( Signal > 90 ) )
		{
			Signal -= 32;
		}
		assert( Signal >= 65 );
		assert( Signal <= 90 );

		// unpause the corona...
		if ( Signal >= Cus->LightTypeSignal )
		{
			Effect_SetPause( EManager->ESystem, Root->EffectList[0], GE_FALSE );
		}
		// ...or pause it
		else
		{
			Effect_SetPause( EManager->ESystem, Root->EffectList[0], GE_TRUE );
		}
	}

	// adjust position if its locked to an actor
	if ( Cus->Actor != NULL )
	{

		// locals
		geXForm3d	Xf;
		geBoolean	Result;
		Corona		Cor;

		// get bone location
		Result = geActor_GetBoneTransform( Cus->Actor, Cus->BoneName, &Xf );
		if ( Result == GE_FALSE )
		{
			return GE_FALSE;
		}

		// setup corona position
		Cor.Vertex.X = Xf.Translation.X;
		Cor.Vertex.Y = Xf.Translation.Y;
		Cor.Vertex.Z = Xf.Translation.Z;
		Result = Effect_Modify(	EManager->ESystem,
								Effect_Corona,
								&Cor,
								Root->EffectList[0],
								CORONA_POS );
		assert( Result == GE_TRUE );
	}

	// all done
	return GE_TRUE;

} // CoronaM_Update()
