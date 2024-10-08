////////////////////////////////////////////////////////////////////////////////////////
//  AmbientSoundM.c									                                        
//                                                                                      
//  Author: Peter Siamidis                                                              
//  Description:    
//			An ambient sound effect.
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
	geActor	*Actor;

} Custom;


////////////////////////////////////////////////////////////////////////////////////////
//	Interface setup
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean	AmbientSoundM_Create( EffectManager *EManager, EffectM_Root *Root );
static void			AmbientSoundM_Destroy( EffectManager *EManager, EffectM_Root *Root );
static geBoolean	AmbientSoundM_Update( EffectManager *EManager, EffectM_Root *Root );
EffectM_Interface	AmbientSoundM_Interface =
{
	AmbientSoundM_Create,
	AmbientSoundM_Destroy,
	AmbientSoundM_Update
};



////////////////////////////////////////////////////////////////////////////////////////
//
//	AmbientSoundM_Death()
//
////////////////////////////////////////////////////////////////////////////////////////
void AmbientSoundM_Death(
	int				ID,			// effect id
	EffectM_Root	*Root )		// root data
{

	// ensure valid data
	assert( ID != -1 );
	assert( Root != NULL );

	// don't need to kill its only effect any more
	assert( Root->EffectList[0] == ID );
	Root->EffectList[0] = -1;

	// mark termination flag
	assert( Root->Terminate == GE_FALSE );
	Root->Terminate = GE_TRUE;

} // AmbientSoundM_Death()



////////////////////////////////////////////////////////////////////////////////////////
//
//	AmbientSoundM_Create()
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean AmbientSoundM_Create(
	EffectManager	*EManager,	// effect manager to add it to
	EffectM_Root	*Root )		// root data
{

	// locals
	EM_AmbientSound	*ASound;
	Snd				Sound;
	Custom			*Cus;

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );

	// do nothing if there is no sound support
	if ( EManager->SPool == NULL )
	{
		return GE_FALSE;
	}

	// get data
	ASound = Root->UserData;
	assert( ASound != NULL );
	assert( Root->UserDataSize == sizeof( *ASound ) );
	
	// don't launch this effect if its trigger based
	if ( EffectC_IsStringNull( ASound->TriggerName ) == GE_FALSE )
	{
		return GE_FALSE;
	}

	// init remaining root data
	Root->EffectCount = 1;
	Root->EffectList = geRam_Allocate( sizeof( int ) * Root->EffectCount );
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

	// setup sound struct
	memset( &Sound, 0, sizeof( Sound ) );
	geVec3d_Copy( &( ASound->Position ), &( Sound.Pos ) );
	Sound.Loop = ASound->LoopSound;
	Sound.Min = ASound->MinDistance;
	Sound.IgnoreObstructions = ASound->IgnoreObstructions;
	assert( Sound.Min > 0.0f );
	ASound->SoundNumber = SPool_Add( EManager->SPool, NULL, ASound->SoundName );
	if ( ASound->SoundNumber == -1 )
	{
		return GE_FALSE;
	}
	Sound.SoundDef = SPool_Get( EManager->SPool, ASound->SoundNumber );
	assert( Sound.SoundDef != NULL );

	// setup position based on actor bone
	if ( ( ASound->AStart != NULL ) && ( ASound->AStart->Actor != NULL ) )
	{

		// locals
		geXForm3d	Xf;
		geBoolean	Result;

		// save actor that its locked to
		Cus->Actor = ASound->AStart->Actor;

		// get bone location
		Result = geActor_GetBoneTransform( Cus->Actor, "EFFECT_BONE01", &Xf );
		if ( Result == GE_FALSE )
		{
			Result = geActor_GetBoneTransform( Cus->Actor, NULL, &Xf );
			if ( Result == GE_FALSE )
			{
				return GE_FALSE;
			}
		}
		geVec3d_Copy( &( Xf.Translation ), &( Sound.Pos ) );
	}

	// add sound
	if ( Effect_New( EManager->ESystem, Effect_Sound, &Sound, 0, AmbientSoundM_Death, Root, &( Root->EffectList[0] ) ) == GE_FALSE )
	{
		return GE_FALSE;
	}

	// all done
	return GE_TRUE;

} // AmbientSoundM_Create()



////////////////////////////////////////////////////////////////////////////////////////
//
//	AmbientSoundM_Destroy()
//
////////////////////////////////////////////////////////////////////////////////////////
static void AmbientSoundM_Destroy(
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

} // AmbientSoundM_Destroy()



////////////////////////////////////////////////////////////////////////////////////////
//
//	AmbientSoundM_Update()
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean AmbientSoundM_Update(
	EffectManager	*EManager,	// effect manager to which it belongs
	EffectM_Root	*Root )		// root data
{

	// locals
	Custom	*Cus;

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );

	// terminate the effect if required
	if ( Root->Terminate == GE_TRUE )
	{
		return GE_FALSE;
	}

	// get custom data
	Cus = Root->Custom;
	assert( Cus != NULL );

	// adjust position if its locked to an actor
	if ( Cus->Actor != NULL )
	{

		// locals
		geXForm3d	Xf;
		geBoolean	Result;
		Snd			Sound;

		// get bone location
		Result = geActor_GetBoneTransform( Cus->Actor, "EFFECT_BONE01", &Xf );
		if ( Result == GE_FALSE )
		{
			Result = geActor_GetBoneTransform( Cus->Actor, NULL, &Xf );
			if ( Result == GE_FALSE )
			{
				return GE_FALSE;
			}
		}
		geVec3d_Copy( &( Xf.Translation ), &( Sound.Pos ) );

		// adjust sound
		Result = Effect_Modify(	EManager->ESystem,
								Effect_Sound,
								&Sound,
								Root->EffectList[0],
								SND_POS );
		assert( Result == GE_TRUE );
	}

	// all done
	return GE_TRUE;

} // AmbientSoundM_Update()
