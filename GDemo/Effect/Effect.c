////////////////////////////////////////////////////////////////////////////////////////
//  Effect.c									                                        
//                                                                                      
//  Author: Peter Siamidis                                                              
//  Description:    
//				API to control special effects.
//				Effect_SystemFrame() must be called once per frame.
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
//#define	EFFECTSYSTEMTIMINGON
#ifdef EFFECTSYSTEMTIMINGON
#include <windows.h>
#endif
#include <memory.h>
#include <assert.h>
#include "Ram.h"
#include "Effect.h"
#include "IndexList.h"
#include "Errorlog.h"


////////////////////////////////////////////////////////////////////////////////////////
//	Hardwired stuff
////////////////////////////////////////////////////////////////////////////////////////
#define	EFFECT_TIMEDELTACAP	0.5f
#define EFFECT_LISTINCSIZE	50


////////////////////////////////////////////////////////////////////////////////////////
//	Effect interface list (IF YOU ADD AN EFFECT ITS INTERFACE MUST BE LISTED HERE)
////////////////////////////////////////////////////////////////////////////////////////
Effect_Interface *EffectInterfaceList[] =
{
	NULL,
	&Spray_Interface,
	&Sprite_Interface,
	&Snd_Interface,
	&ProceduralTexture_Interface,
	&Corona_Interface,
	&Bolt_Interface,
	&Glow_Interface

};
#define EffectCount	( sizeof( EffectInterfaceList ) / sizeof( EffectInterfaceList[0] ) )


////////////////////////////////////////////////////////////////////////////////////////
//	Top level effect struct
////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	EffectType			Type;			// effect type
	geBoolean			Paused;			// whether or not the effect is paused
	int					Slot;			// slot which this effect occupies
	void				*Data;			// effect data
	EffectDeathCallback	DeathCallback;	// death callback function
	void				*DeathContext;	// death context data

} EffectRoot;


////////////////////////////////////////////////////////////////////////////////////////
//	Effect system struct
////////////////////////////////////////////////////////////////////////////////////////
typedef struct EffectSystem
{
	IndexList		*EffectList;					// list of all effect root structs
	int				ActiveEffects[EffectCount];		// how many effects are currently active
	void			*ResourceList[EffectCount];		// resource list pointers
	EffectResource	Resource;						// list of effect resources

} EffectSystem;



////////////////////////////////////////////////////////////////////////////////////////
//
//	Effect_SystemCreate()
//
//	Create an effect system and return a pointer to it.
//
////////////////////////////////////////////////////////////////////////////////////////
EffectSystem * Effect_SystemCreate(
	geEngine		*EngineToUse,	// engine to use
	geWorld			*WorldToUse,	// world to use
	geSound_System	*SoundToUse,	// sound system to use
	geCamera		*CameraToUse )	// camera to use
{

	// locals
	EffectSystem	*System;
	int				i;

	// ensure valid data
	assert( EngineToUse != NULL );
	assert( WorldToUse != NULL );
	assert( CameraToUse != NULL );

	// allocate effect system struct and init it
	System = geRam_AllocateClear( sizeof( *System ) );
	if ( System == NULL )
	{
		geErrorLog_AddString( GE_ERR_MEMORY_RESOURCE, "Effect_SystemCreate: failed to create effect system struct.", NULL );
		return NULL;
	}		

	// save resource list
	System->Resource.Engine = EngineToUse;
	System->Resource.World = WorldToUse;
	System->Resource.Sound = SoundToUse;
	System->Resource.Camera = CameraToUse;

	// create particle system
	System->Resource.PS = Particle_SystemCreate( System->Resource.World );
	if ( System->Resource.PS == NULL )
	{
		geErrorLog_AddString( GE_ERR_INTERNAL_RESOURCE, "Effect_SystemCreate: failed to create particle system.", NULL );
		Effect_SystemDestroy( &System );
		return NULL;
	}

	// create effect list
	System->EffectList = IndexList_Create( EFFECT_LISTINCSIZE, EFFECT_LISTINCSIZE );
	if ( System->EffectList == NULL )
	{
		geErrorLog_AddString( GE_ERR_INTERNAL_RESOURCE, "Effect_SystemCreate: failed to create indexed effect list.", NULL );
		Effect_SystemDestroy( &System );
		return NULL;
	}

	// call all effect create functions
	for ( i = 1; i < EffectCount; i++ )
	{
		System->ResourceList[i] = EffectInterfaceList[i]->Create( &( System->Resource ), i );
		if ( System->ResourceList[i] == NULL )
		{
			geErrorLog_AddString( GE_ERR_INTERNAL_RESOURCE, "Effect_SystemCreate: failed to initialize an effect subsystem.", NULL );
			Effect_SystemDestroy( &System );
			return NULL;
		}		
	}

	// all done
	return System;

} // Effect_SystemCreate()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Effect_SystemDestroy()
//
//	Destroy an effect system.
//
////////////////////////////////////////////////////////////////////////////////////////
void Effect_SystemDestroy(
	EffectSystem	**System )	// effect system to zap
{

	// locals
	EffectSystem	*DeadSystem;
	EffectRoot		*Root;
	int				i;
	int				ListSize;
	geBoolean		Result;

	// ensure valid data
	assert( System != NULL );
	assert( *System != NULL );

	// get dead effect system pointer
	DeadSystem = *System;

	// destroy all active effects
	if ( DeadSystem->EffectList != NULL )
	{

		// remove all current effects
		ListSize = IndexList_GetListSize( DeadSystem->EffectList );
		for ( i = 0; i < ListSize; i++ )
		{
			Root = IndexList_GetElement( DeadSystem->EffectList, i );
			if ( Root != NULL )
			{
				Result = Effect_Delete( DeadSystem, i );
				assert( Result == GE_TRUE );
			}
		}

		// destroy the list itself
		IndexList_Destroy( &( DeadSystem->EffectList ) );

		// call all effect interface destroy functions
		for ( i = 1; i < EffectCount; i++ )
		{
			assert( DeadSystem->ActiveEffects[i] == 0 );
			assert( DeadSystem->ResourceList[i] != NULL );
			EffectInterfaceList[i]->Destroy( &( DeadSystem->ResourceList[i] ) );
		}
	}

	// free particle system
	if ( DeadSystem->Resource.PS != NULL )
	{
		Particle_SystemDestroy( DeadSystem->Resource.PS );
		DeadSystem->Resource.PS = NULL;
	}

	// free system struct memory
	geRam_Free( DeadSystem );
	System = NULL;

} // Effect_SystemDestroy()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Effect_New()
//
//	Create an effect and return its unique identifier, or return 0 if the effect
//	was not created.
//
////////////////////////////////////////////////////////////////////////////////////////
geBoolean Effect_New(
	EffectSystem		*System,		// effect system to add it to
	EffectType			Type,			// type of effect
	void				*Data,			// effect data
	int					AttachTo,		// identifier of effect to attach to
	EffectDeathCallback	DeathCallback,	// death callback function
	void				*DeathContext,	// context data
	int					*Id )			// where to save effect id
{

	// locals
	Effect_Interface	*Effect;
	EffectRoot			*Root;
	void				*AttachData = NULL;
	int					Slot;

	// ensure valid data
	assert( System != NULL );
	assert( System->EffectList != NULL );
	assert( Data != NULL );

	// pick the effect interface that we will use
	Effect = EffectInterfaceList[Type];
	assert( Effect != NULL );

	// get an empty slot
	Slot = IndexList_GetEmptySlot( System->EffectList );
	if ( Slot == -1 )
	{
		geErrorLog_AddString( GE_ERR_INTERNAL_RESOURCE, "Effect_New: failed to find available effect slot.", NULL );
		return GE_FALSE;
	}

	// if it wants to attach to another effect then verify that it still exists
	if ( AttachTo > 0 )
	{
		AttachData = IndexList_GetElement( System->EffectList, AttachTo );
		if ( AttachData == NULL )
		{
			geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Effect_New: AttachTo effect does not exist.", NULL );
			return GE_FALSE;
		}
	}

	// allocate root data space and init it
	Root = geRam_AllocateClear( sizeof( EffectRoot ) );
	if ( Root == NULL )
	{
		geErrorLog_AddString( GE_ERR_MEMORY_RESOURCE, "Effect_New: failed to create effect root struct.", NULL );
		return GE_FALSE;
	}
	Root->Paused = GE_FALSE;
	Root->Type = Type;

	// mark the type flag
	*( (int *)( Data ) ) = Root->Type;

	// add effect
	Root->Data = Effect->Add( System->ResourceList[Type], Data, AttachData );
	if ( Root->Data == NULL )
	{
		geErrorLog_AddString( GE_ERR_INTERNAL_RESOURCE, "Effect_New: failed to add an effect.", NULL );
		geRam_Free( Root );
		return GE_FALSE;
	}

	// add new effect to the list
	IndexList_AddElement( System->EffectList, Slot, Root );

	// adjust effect count
	System->ActiveEffects[Root->Type]++;

	// save callback info
	Root->DeathCallback = DeathCallback;
	Root->DeathContext = DeathContext;

	// save effect id number
	Root->Slot = Slot;
	if ( Id != NULL )
	{
		*Id = Slot;
	}

	// all done
	assert( Slot >= 0 );
	return GE_TRUE;

} // Effect_New()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Effect_Delete()
//
//	Removes an effect.
//
////////////////////////////////////////////////////////////////////////////////////////
geBoolean Effect_Delete(
	EffectSystem	*System,	// effect system to delete it from
	int				ID )		// id of effect to delete
{

	// locals
	Effect_Interface	*Effect;
	EffectRoot			*Root;

	// ensure valid data
	assert( System != NULL );
	assert( System->EffectList != NULL );

	// remove the effect
	Root = IndexList_GetElement( System->EffectList, ID );
	if ( Root != NULL )
	{

		// call its death callback if required
		if ( Root->DeathCallback != NULL )
		{
			Root->DeathCallback( Root->Slot, Root->DeathContext );
			Root->DeathCallback = NULL;
		}

		// remove the effecet
		Effect = EffectInterfaceList[Root->Type];
		Effect->Remove( System->ResourceList[Root->Type], Root->Data );
		System->ActiveEffects[Root->Type]--;

		// free root data
		geRam_Free( Root );
		IndexList_DeleteElement( System->EffectList, ID );
		return GE_TRUE;
	}

	// if we got to here then the effect was not found
	return GE_FALSE;

} // Effect_Delete()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Effect_SystemFrame()
//
//	Process all current effects. Return GE_TRUE if all effects were processed, or
//	GE_FALSE if one or more of the effects were not processed.
//
////////////////////////////////////////////////////////////////////////////////////////
geBoolean Effect_SystemFrame(
	EffectSystem	*System,		// effect system to process a frame for
	float			TimeDelta )		// amount of elapsed time
{

	// locals
	EffectRoot			*Root = NULL;
	Effect_Interface	*Effect;
	geBoolean			EffectAlive;
	int					i;
	int					ListSize;
	#ifdef EFFECTSYSTEMTIMINGON
	int					Type;
	LARGE_INTEGER		Frequency;
	LARGE_INTEGER		StartTime, EndTime;
	double				TotalTime[EffectCount];
	#endif

	// ensure valid data
	assert( System != NULL );
	assert( System->EffectList != NULL );

	// do nothing if no time has elapsed
	if ( TimeDelta <= 0.0f )
	{
		return GE_FALSE;
	}

	// get total start time
	#ifdef EFFECTSYSTEMTIMINGON
	QueryPerformanceFrequency( &Frequency );
	for ( i = 1; i < EffectCount; i++ )
	{
		TotalTime[i] = 0.0f;
	}
	#endif

	// cap the time delta
	if ( TimeDelta > EFFECT_TIMEDELTACAP )
	{
		TimeDelta = EFFECT_TIMEDELTACAP;
	}

	// process all effects
	ListSize = IndexList_GetListSize( System->EffectList );
	for ( i = 0; i < ListSize; i++ )
	{

		// do nothing if it is an empty slot
		Root = IndexList_GetElement( System->EffectList, i );
		if ( Root == NULL )
		{
			continue;
		}

		// get start time
		#ifdef EFFECTSYSTEMTIMINGON
		Type = Root->Type;
		QueryPerformanceCounter( &StartTime );
		#endif

		// process the effect...
		if ( Root->Paused == GE_FALSE )
		{

			// pick the effect interface that we will use
			Effect = EffectInterfaceList[Root->Type];
			assert( Effect != NULL );

			// process this effect
			EffectAlive = Effect->Process( System->ResourceList[Root->Type], TimeDelta, Root->Data );

			// remove it if required
			if ( EffectAlive == GE_FALSE )
			{

				// call its death callback if required
				if ( Root->DeathCallback != NULL )
				{
					Root->DeathCallback( Root->Slot, Root->DeathContext );
					Root->DeathCallback = NULL;
				}

				// remove the effect
				Effect->Remove( System->ResourceList[Root->Type], Root->Data );
				System->ActiveEffects[Root->Type]--;

				// free its data
				geRam_Free( Root );
				IndexList_DeleteElement( System->EffectList, i );
			}
		}

		// get start time
		#ifdef EFFECTSYSTEMTIMINGON
		QueryPerformanceCounter( &EndTime );
		TotalTime[Type] += ( (double)( EndTime.LowPart - StartTime.LowPart ) / (double)( Frequency.LowPart ) );
		#endif
	}

	// do once-per-frame processing
	for ( i = 1; i < EffectCount; i++ )
	{
		#ifdef EFFECTSYSTEMTIMINGON
		QueryPerformanceCounter( &StartTime );
		#endif
		EffectInterfaceList[i]->Frame( System->ResourceList[i], TimeDelta );
		#ifdef EFFECTSYSTEMTIMINGON
		QueryPerformanceCounter( &EndTime );
		TotalTime[i] += ( (double)( EndTime.LowPart - StartTime.LowPart ) / (double)( Frequency.LowPart ) );
		#endif
	}

	// do a partcle system frame
	Particle_SystemFrame( System->Resource.PS, TimeDelta );

	// output extra info
	#ifdef EFFECTSYSTEMTIMINGON
	{

		// locals
		int	TotalEffects = 0;
		int	Row = 0;
		double	CompleteTotalTime = 0.0f;

		// determine total time
		for ( i = 1; i < EffectCount; i++ )
		{
			CompleteTotalTime += TotalTime[i];
		}

		// output effect summary
		for ( i = 1; i < EffectCount; i++ )
		{
			if ( System->ActiveEffects[i] > 0 )
			{
				geEngine_Printf( System->Resource.Engine, 0, Row++ * 10, "%s, %d, %.2fms, %.0f%%", EffectInterfaceList[i]->GetName(), System->ActiveEffects[i], TotalTime[i] * 1000, ( TotalTime[i] / CompleteTotalTime ) * 100.0f );
			}
			TotalEffects += System->ActiveEffects[i];
		}
		geEngine_Printf( System->Resource.Engine, 0, Row++ * 10, "Total: %d, %.2fms", TotalEffects, CompleteTotalTime * 1000 );
	}
	#endif

	// all done
	return GE_TRUE;

} // Effect_SystemFrame()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Effect_StillExists()
//
//	Returns true if the effect is still alive, false if it isn't.
//
////////////////////////////////////////////////////////////////////////////////////////
geBoolean Effect_StillExists(
	EffectSystem	*System,	// effect system to search in
	int				ID )		// effect id to verify
{

	// locals
	void	*Data;

	// ensure valid data
	assert( System != NULL );

	// get effect data
	Data = IndexList_GetElement( System->EffectList, ID );

	// effect exists...
	if ( Data != NULL )
	{
		return GE_TRUE;
	}
	// ...or it doesn't exist
	else
	{
		return GE_FALSE;
	}

} // Effect_StillExists()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Effect_Modify()
//
//	Modify an effect.
//
////////////////////////////////////////////////////////////////////////////////////////
geBoolean Effect_Modify(
	EffectSystem	*System,	// effect system in which it exists
	EffectType		Type,		// type of effect
	void			*Data,		// new effect data
	int				ID,			// id of effect to modify
	uint32			Flags )		// how to use the new data
{

	// locals
	EffectRoot			*Root = NULL;
	Effect_Interface	*Effect;

	// ensure valid data
	assert( System != NULL );
	assert( System->EffectList != NULL );
	assert( Data != NULL );

	// get root data
	Root = IndexList_GetElement( System->EffectList, ID );
	if ( Root == NULL )
	{
		return GE_FALSE;
	}
	assert( Root->Type == Type );

	// pick the effect interface that we will use
	Effect = EffectInterfaceList[Root->Type];
	assert( Effect != NULL );

	// modify it
	assert( *( (int *)( Root->Data ) ) == Type );
	Effect->Modify( System->ResourceList[Root->Type], Root->Data, Data, Flags );

	// all done
	return GE_TRUE;
	
	// get rid of warnings
	Type;

} // Effect_Modify()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Effect_SetPause()
//
//	Pause/unpause an effect.
//
////////////////////////////////////////////////////////////////////////////////////////
geBoolean Effect_SetPause(
	EffectSystem	*System,	// effect system in which it exists
	int				ID,			// id of effect to modify
	geBoolean		Pause )		// new pause state
{

	// locals
	EffectRoot			*Root = NULL;
	Effect_Interface	*Effect;

	// ensure valid data
	assert( System != NULL );
	assert( System->EffectList != NULL );

	// get root data
	Root = IndexList_GetElement( System->EffectList, ID );
	if ( Root == NULL )
	{
		return GE_FALSE;
	}

	// pick the effect interface that we will use
	Effect = EffectInterfaceList[Root->Type];
	assert( Effect != NULL );

	// set pause flag
	Root->Paused = Pause;

	// call the effect pause function
	Effect->Pause( System->ResourceList[Root->Type], Root->Data, Pause );

	// all done
	return GE_TRUE;

} // Effect_SetPause()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Effect_GetEngine()
//
//	Get a pointer to the engine that this effect system is tied to.
//
////////////////////////////////////////////////////////////////////////////////////////
geEngine * Effect_GetEngine(
	EffectSystem	*ESystem )	// effect system whose engine we want
{

	// ensure valid data
	assert( ESystem != NULL );

	// return its engine
	assert( ESystem->Resource.Engine != NULL );
	return ESystem->Resource.Engine;

} // Effect_GetEngine()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Effect_GetWorld()
//
//	Get a pointer to the world that this effect system is tied to.
//
////////////////////////////////////////////////////////////////////////////////////////
geWorld * Effect_GetWorld(
	EffectSystem	*ESystem )	// effect system whose world we want
{

	// ensure valid data
	assert( ESystem != NULL );

	// return its world
	assert( ESystem->Resource.World != NULL );
	return ESystem->Resource.World;

} // Effect_GetWorld()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Effect_GetCamera()
//
//	Get a pointer to the camera that this effect system is tied to.
//
////////////////////////////////////////////////////////////////////////////////////////
geCamera * Effect_GetCamera(
	EffectSystem	*ESystem )	// effect system whose camera we want
{

	// ensure valid data
	assert( ESystem != NULL );

	// return its camera
	assert( ESystem->Resource.Camera != NULL );
	return ESystem->Resource.Camera;

} // Effect_GetCamera()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Effect_DeleteAll()
//
//	Destroys all effects.
//
////////////////////////////////////////////////////////////////////////////////////////
void Effect_DeleteAll(
	EffectSystem	*ESystem )	// effect system whose effects will be destroyed
{

	// locals
	int			i;
	int			ListSize;
	EffectRoot	*Root;
	geBoolean	Result;

	// ensure valid data
	assert( ESystem != NULL );

	// remove all current effects
	ListSize = IndexList_GetListSize( ESystem->EffectList );
	for ( i = 0; i < ListSize; i++ )
	{
		Root = IndexList_GetElement( ESystem->EffectList, i );
		if ( Root != NULL )
		{
			Result = Effect_Delete( ESystem, i );
			assert( Result == GE_TRUE );
		}
	}

	// make sure everything was zapped
	#ifndef	NDEBUG
	for ( i = 1; i < EffectCount; i++ )
	{
		assert( ESystem->ActiveEffects[i] == 0 );
	}
	#endif

} // Effect_DeleteAll()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Effect_ChangeWorld()
//
//	Changed the world that the effect system uses.
//
////////////////////////////////////////////////////////////////////////////////////////
geBoolean Effect_ChangeWorld(
	EffectSystem	*ESystem,	// effect system whose world will change
	geWorld			*World )	// the new world that will be used
{

	// locals
	Particle_System	*PS;

	// ensure valid data
	assert( ESystem != NULL );
	assert( World != NULL );

	// remove all current effects
	Effect_DeleteAll( ESystem );

	// save new world pointer
	ESystem->Resource.World = World;

	// recreate particle system
	assert( ESystem->Resource.PS != NULL );
	PS = Particle_SystemCreate( ESystem->Resource.World );
	if ( PS == NULL )
	{
		geErrorLog_AddString( GE_ERR_INTERNAL_RESOURCE, "Effect_ChangeWorld: failed to recreate particle system.", NULL );
		return GE_FALSE;
	}
	Particle_SystemDestroy( ESystem->Resource.PS );
	ESystem->Resource.PS = PS;

	// all done
	return GE_TRUE;

} // Effect_ChangeWorld()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Effect_SystemChangeSoundSystem()
//
//	Change the sound system that the effect system uses.
//
////////////////////////////////////////////////////////////////////////////////////////
void Effect_SystemChangeSoundSystem(
	EffectSystem	*System,			// effect system whose sound system will be changed
	geSound_System	*NewSoundSystem )	// new sound system to use
{
	
	// ensure valid data
	assert( System != NULL );
	assert( NewSoundSystem != NULL );

	// remove all current effects
	Effect_DeleteAll( System );

	// save new sound system pointer
	System->Resource.Sound = NewSoundSystem;

} // Effect_SystemChangeSoundSystem()
