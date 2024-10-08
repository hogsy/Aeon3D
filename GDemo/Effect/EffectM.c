////////////////////////////////////////////////////////////////////////////////////////
//  EffectM.c									                                        
//                                                                                      
//  Author: Peter Siamidis                                                              
//  Description:    
//			Effect manager, designed to group multiple effects into one type of effect.
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
#pragma warning ( disable : 4514 )
#include <memory.h>
#include <assert.h>
#include "Errorlog.h"
#include "Ram.h"


//////////////////////////////////////////////////////////////////////////////////////////
//	IF YOU ADD AN EFFECT ITS INTERFACE HEADER MUST BE HERE
//////////////////////////////////////////////////////////////////////////////////////////
#include "ExplosionM.h"
#include "RainM.h"
#include "CoronaM.h"
#include "SpoutM.h"
#include "AmbientSoundM.h"
#include "ProceduralM.h"
#include "FloatingParticlesM.h"
#include "MorphM.h"
#include "ElectricM.h"
#include "ScrollM.h"
#include "InvisibleM.h"
#include "RedWispM.h"
#include "FlipbookM.h"
#include "ChaosM.h"
#include "LightM.h"
#include "LightStyleM.h"
#include "ColorCycleM.h"
#include "ActorSettingsM.h"


//////////////////////////////////////////////////////////////////////////////////////////
//	IF YOU ADD AN EFFECT ITS INTERFACE POINTER MUST BE HERE
//////////////////////////////////////////////////////////////////////////////////////////
EffectM_Interface *EffectMInterfaceList[] =
{
	&ExplosionM_Interface,
	&RainM_Interface,
	&CoronaM_Interface,
	&SpoutM_Interface,
	&AmbientSoundM_Interface,
	&ProceduralM_Interface,
	&FloatingParticlesM_Interface,
	&MorphM_Interface,
	&ElectricM_Interface,
	&ScrollM_Interface,
	&InvisibleM_Interface,
	&RedWispM_Interface,
	&FlipbookM_Interface,
	&ChaosM_Interface,
	&LightM_Interface,
	&LightStyleM_Interface,
	&ColorCycleM_Interface,
	&ActorSettingsM_Interface

};
#define EffectMCount	( sizeof( EffectMInterfaceList ) / sizeof( EffectMInterfaceList[0] ) )


////////////////////////////////////////////////////////////////////////////////////////
//	IF YOU ADD AN EFFECT THAT WORKS IN THE EDITOR, ITS NAME MUST BE HERE
////////////////////////////////////////////////////////////////////////////////////////
static char	*EntityEffect[] = 
{
	"EM_Rain",
	"EM_Corona",
	"EM_Spout",
	"EM_AmbientSound",
	"EM_Procedural",
	"EM_FloatingParticles",
	"EM_Morph",
	"EM_Electric",
	"EM_Scroll",
	"EM_Invisible",
	"EM_RedWisp",
	"EM_Flipbook",
	"EM_Chaos",
	"EM_Light",
	"EM_LightStyle",
	"EM_ColorCycle",
	"EM_ActorSettings"

};
#define EntityEffectCount	( sizeof( EntityEffect ) / sizeof( EntityEffect[0] ) )
static int	EntityInfo[EntityEffectCount][2] = 
{
	{ EffectM_Rain, sizeof( EM_Rain ) },
	{ EffectM_Corona, sizeof( EM_Corona ) },
	{ EffectM_Spout, sizeof( EM_Spout ) },
	{ EffectM_AmbientSound, sizeof( EM_AmbientSound ) },
	{ EffectM_Procedural, sizeof( EM_Procedural ) },
	{ EffectM_FloatingParticles, sizeof( EM_FloatingParticles ) },
	{ EffectM_Morph, sizeof( EM_Morph ) },
	{ EffectM_Electric, sizeof( EM_Electric ) },
	{ EffectM_Scroll, sizeof( EM_Scroll ) },
	{ EffectM_Invisible, sizeof( EM_Invisible ) },
	{ EffectM_RedWisp, sizeof( EM_RedWisp ) },
	{ EffectM_Flipbook, sizeof( EM_Flipbook ) },
	{ EffectM_Chaos, sizeof( EM_Chaos ) },
	{ EffectM_Light, sizeof( EM_Light ) },
	{ EffectM_LightStyle, sizeof( EM_LightStyle ) },
	{ EffectM_ColorCycle, sizeof( EM_ColorCycle ) },
	{ EffectM_ActorSettings, sizeof( EM_ActorSettings ) }

};



////////////////////////////////////////////////////////////////////////////////////////
//
//	EffectM_CreateManager()
//
//	Create a new effect manager.
//
////////////////////////////////////////////////////////////////////////////////////////
EffectManager * EffectM_CreateManager(
  	EffectSystem	*ESystem,		// effect system to use
	TexturePool		*TPool,			// texture pool to use
	SoundPool		*SPool,			// sound pool to use
	void			*Context,		// context data
	geBoolean		SoftwareMode )	// whether or not software mode is being used
{

	// locals
	EffectManager	*EManager = NULL;

	// ensure valid data
	assert( ESystem != NULL );
	assert( TPool != NULL );

	// allocate struct
	EManager = geRam_Allocate( sizeof( *EManager ) );
	if ( EManager == NULL )
	{
		return NULL;
	}
	memset( EManager, 0, sizeof( *EManager ) );

	// create linked list to hold active effects
	EManager->EffectList = IndexList_Create( 50, 10 );
	if ( EManager->EffectList == NULL )
	{
		geRam_Free( EManager );
		return NULL;
	}

	// save other relevant data
	EManager->ESystem = ESystem;
	EManager->TPool = TPool;
	EManager->SPool = SPool;
	EManager->Context = Context;
	EManager->SoftwareMode = SoftwareMode;

	// all done
	return EManager;

} // EffectM_CreateManager()



////////////////////////////////////////////////////////////////////////////////////////
//
//	EffectM_Delete()
//
//	Destroy one effect of an effect manager.
//
////////////////////////////////////////////////////////////////////////////////////////
static void EffectM_Delete(
	EffectManager	*EManager,	// effect manager that it belongs to
	EffectM_Root	**Root )	// root data to wipe
{

	// locals
	EffectM_Root	*DeadRoot;
	int				i;

	// ensure valid data
	assert( Root != NULL );
	assert( *Root != NULL );

	// setup root pointer
	DeadRoot = *Root;

	// call the effects destroy function
	EffectMInterfaceList[DeadRoot->Type]->Destroy( EManager, DeadRoot );

	// free all of its associated effects and the list itself
	if ( DeadRoot->EffectList != NULL )
	{
		for ( i = 0; i < DeadRoot->EffectCount; i++ )
		{
			if ( DeadRoot->EffectList[i] != -1 )
			{
				Effect_Delete( EManager->ESystem, DeadRoot->EffectList[i] );
			}
		}
		geRam_Free( DeadRoot->EffectList );
	}

	// free any custom data
	if ( DeadRoot->Custom != NULL )
	{
		geRam_Free( DeadRoot->Custom );
	}

	// free the root
	geRam_Free( DeadRoot );
	Root = NULL;

} // EffectM_Delete()



////////////////////////////////////////////////////////////////////////////////////////
//
//	EffectM_Destroy()
//
//	Destroy one effect of an effect manager by id.
//
////////////////////////////////////////////////////////////////////////////////////////
geBoolean EffectM_Destroy(
	EffectManager	*EManager,	// effect manager that it belongs to
	int				Id )		// effect to destroy
{

	// locals
	EffectM_Root	*Root;

	// ensure valid data
	assert( EManager != NULL );
	assert( Id >= 0 );

	// get effect data
	Root = IndexList_GetElement( EManager->EffectList, Id );
	if ( Root == NULL )
	{
		return GE_FALSE;
	}

	// delete the effect
	EffectM_Delete( EManager, &Root );

	// all done
	return GE_TRUE;

} // EffectM_Destroy()



////////////////////////////////////////////////////////////////////////////////////////
//
//	EffectM_DestroyManager()
//
//	Destroy an effect manager.
//
////////////////////////////////////////////////////////////////////////////////////////
void EffectM_DestroyManager(
	EffectManager	**EManager )	// manager to zap
{

	// locals
	EffectManager	*DeadEManager;
	EffectM_Root	*Root;
	int				i;
	int				EffectCount;

	// ensure valid data
	assert( EManager != NULL );
	assert( *EManager != NULL );

	// setup effect manager pointer
	DeadEManager = *EManager;

	// free all of the managers effects
	EffectCount = IndexList_GetListSize( DeadEManager->EffectList );
	for ( i = 0; i < EffectCount; i++ )
	{

		// get effect data
		Root = IndexList_GetElement( DeadEManager->EffectList, i );
		if ( Root == NULL )
		{
			continue;
		}

		// destroy it
		EffectM_Delete( DeadEManager, &Root );
		IndexList_DeleteElement( DeadEManager->EffectList, i );
	}

	// destroy the effect list
	IndexList_Destroy( &( DeadEManager->EffectList ) );

	// zap the effect manager
	geRam_Free( *EManager );
	EManager = NULL;

} // EffectM_DestroyManager()



////////////////////////////////////////////////////////////////////////////////////////
//
//	EffectM_Create()
//
//	Create a new effect.
//
////////////////////////////////////////////////////////////////////////////////////////
int EffectM_Create(
	EffectManager	*EManager,			// effect manager to add it to
	EffectM_Type	Type,				// type of effect to add
	void			*UserData,			// user data
	int32			UserDataSize )		// size of user data
{

	// locals
	EffectM_Root		*Root;
	geBoolean			Result;
	int					Slot;

	// ensure valid data
	assert( EManager != NULL );
	assert( Type >= 0 );
	assert( Type < EffectMCount );
	assert( UserData != NULL );
	assert( UserDataSize > 0 );

	// allocate root memory
	Root = geRam_Allocate( sizeof( *Root ) );
	if ( Root == NULL )
	{
		return -1;
	}
	memset( Root, 0, sizeof( *Root ) );

	// init the root
	Root->Type = Type;
	Root->UserData = UserData;
	Root->UserDataSize = UserDataSize;

	// create the effect
	Result = EffectMInterfaceList[Root->Type]->Create( EManager, Root );
	if ( Result == GE_FALSE )
	{
		geErrorLog_Add( 0, "Failed to add an effect" );
		EffectM_Delete( EManager, &Root );
		return -1;
	}

	// add new effect to the list
	Slot = IndexList_GetEmptySlot( EManager->EffectList );
	if ( Slot == -1 )
	{
		EffectM_Delete( EManager, &Root );
		return -1;
	}
	IndexList_AddElement( EManager->EffectList, Slot, Root );

	// all done
	return Slot;

} // EffectM_Create()



////////////////////////////////////////////////////////////////////////////////////////
//
//	EffectM_Update()
//
//	Update an effect managers effects.
//
////////////////////////////////////////////////////////////////////////////////////////
int EffectM_Update(
	EffectManager	*EManager,		// manager to update
	float			TimeDelta )		// amount of elased time
{

	// locals
	EffectM_Root		*Root;
	geBoolean			Result;
	int					EffectCount;
	int					i;
	int					EffectsProcessed;

	// ensure valid data
	assert( EManager != NULL );
	assert( TimeDelta > 0.0f );

	// update all effects
	EffectCount = IndexList_GetListSize( EManager->EffectList );
	EffectsProcessed = 0;
	for ( i = 0; i < EffectCount; i++ )
	{

		// get effect data, skipping it if its not active
		Root = IndexList_GetElement( EManager->EffectList, i );
		if ( Root == NULL )
		{
			continue;
		}
		
		// adjust effects processed count
		EffectsProcessed++;

		// update the effect
		assert ( Root->EffectList != NULL );
		Root->TimeDelta = TimeDelta;
		Result = EffectMInterfaceList[Root->Type]->Update( EManager, Root );

		// delete this effect if required
		if ( Result == GE_FALSE )
		{
			EffectM_Delete( EManager, &Root );
			IndexList_DeleteElement( EManager->EffectList, i );
		}
	}

	// all done
	return EffectsProcessed;

} // EffectM_Update()



////////////////////////////////////////////////////////////////////////////////////////
//
//	EffectM_LoadWorldEffects()
//
////////////////////////////////////////////////////////////////////////////////////////
int EffectM_LoadWorldEffects(
	EffectManager	*EManager,	// effect manager that will own them
	geWorld			*World )	// world whose entity effects will be loaded
{

	// locals
	geEntity_EntitySet	*EntitySet;
	geEntity			*Entity = NULL;
	int32				i;
	void				*Data;
	int32				EffectEntitiesAdded = 0;
	char				ErrorString[256];

	// ensure valid data
	assert( EManager != NULL );
	assert( World != NULL );

	// get entity set
	for ( i = 0; i < EntityEffectCount; i++ )
	{

		// get MenuInfo entity set
		EntitySet =  geWorld_GetEntitySet( World, EntityEffect[i] );
		if( EntitySet == NULL )
		{
			continue;
		}
		
		// process all entities of the current type
		while ( ( Entity = geEntity_EntitySetGetNextEntity( EntitySet, Entity ) ) != NULL )
		{

			// get entity data
			Data = geEntity_GetUserData( Entity );
			assert( Data != NULL );

			// add this effect...
			assert( EntityInfo[i][1] > 0 );
			if ( EffectM_Create( EManager, EntityInfo[i][0], Data, EntityInfo[i][1] ) != -1 )
			{
				EffectEntitiesAdded++;
			}
			// ...or log an error
			else
			{
				sprintf( ErrorString, "Could not add an entity effect of type %d\n", EntityInfo[i][0] );
				geErrorLog_Add( 0, ErrorString );
			}
		}
	}

	// all done
	return EffectEntitiesAdded;

} // EffectM_LoadWorldEffects()


/*
////////////////////////////////////////////////////////////////////////////////////////
//
//	EffectM_ChangeSoundPool()
//
//	Change the sound pool that the effect manager will use.
//
////////////////////////////////////////////////////////////////////////////////////////
void EffectM_ChangeSoundPool(
	EffectManager	*EManager,	// effect manager that will get a new sound pool
	SoundPool		*SPool )	// the new sound pool
{

	// ensure valid data
	assert( EManager != NULL );
	assert( SPool != NULL );

	// save new sound pool
	EManager->SPool = SPool;

} // EffectM_ChangeSoundPool()
*/