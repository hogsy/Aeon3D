/****************************************************************************************/
/*  Snd.c										                                        */
/*                                                                                      */
/*  Author: Peter Siamidis                                                              */
/*  Description:    Plays a sound														*/
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
#include <memory.h>
#include <assert.h>
#include "EffectI.h"
#include "Ram.h"
#include "Errorlog.h"
#include "Snd.h"


////////////////////////////////////////////////////////////////////////////////////////
//	Hardwired stuff
////////////////////////////////////////////////////////////////////////////////////////
//#define SND_DEBUGINFO
#define SND_MINAUDIBLEVOLUME	0.1f


////////////////////////////////////////////////////////////////////////////////////////
//	Resource struct
////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	EffectResource	*ExternalResource;	// external resource list
	int				TypeID;				// effect type id

} SndResource;


////////////////////////////////////////////////////////////////////////////////////////
//	Interface setup
////////////////////////////////////////////////////////////////////////////////////////
static char *		Snd_GetName( void );
static void *		Snd_Create( EffectResource *Resource, int TypeID );
static void			Snd_Destroy( void ** ResourceToZap );
static void *		Snd_Add( SndResource *Resource, Snd *Data, Snd *AttachData );
static void			Snd_Remove( SndResource *Resource, Snd *Data );
static geBoolean	Snd_Process( SndResource *Resource, float TimeDelta, Snd *Data );
static geBoolean	Snd_Frame( SndResource *Resource, float TimeDelta );
static geBoolean	Snd_Modify( SndResource *Resource, Snd *Data, Snd *NewData, uint32 Flags );
static void			Snd_Pause( SndResource *Resource, Snd *Data, geBoolean Pause );
Effect_Interface	Snd_Interface =
{
	Snd_GetName,
	Snd_Create,
	Snd_Destroy,
	Snd_Add,
	Snd_Remove,
	Snd_Process,
	Snd_Frame,
	Snd_Modify,
	Snd_Pause,
	sizeof( Snd )
};



////////////////////////////////////////////////////////////////////////////////////////
//
//	Snd_GetName()
//
////////////////////////////////////////////////////////////////////////////////////////
static char * Snd_GetName(
	void )	// no parameters
{
	return "Sound";

} // Snd_GetName()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Snd_Get3dSoundValues()
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean Snd_Get3dSoundValues(
	EffectResource	*ExternalResource,	// assorted required external resources
	Snd				*Data,				// data of effect
	geFloat			*Volume,			// where to store the volume
	geFloat			*Pan,				// where to store the pan
	geFloat			*Frequency )		// where to store the frequency
{

	// locals
	const geXForm3d	*SoundXf;
	geFloat			VolDelta, PanDelta;

	// ensure valid data
	assert( ExternalResource != NULL );
	assert( Data != NULL );
	assert( Volume != NULL );
	assert( Pan != NULL );
	assert( Frequency != NULL );

	// get the camera xform
	assert( ExternalResource->Camera != NULL );
	SoundXf = geCamera_GetWorldSpaceXForm( ExternalResource->Camera );

	// get 3d sound values
	if ( Data->IgnoreObstructions == GE_FALSE )
	{
		geSound3D_GetConfig(
			ExternalResource->World,
			SoundXf, 
			&( Data->Pos ), 
			Data->Min, 
			0,
			Volume,
			Pan,
			Frequency );
	}
	else
	{
		geSound3D_GetConfigIgnoreObstructions(
			ExternalResource->World,
			SoundXf, 
			&( Data->Pos ), 
			Data->Min, 
			0,
			Volume,
			Pan,
			Frequency );
	}

	// return true or false depending on whether or not its worth modifying the sound
	VolDelta = Data->LastVolume - *Volume;
	if ( VolDelta < 0.0f )
	{
		VolDelta = -VolDelta;
	}
	PanDelta = Data->LastPan - *Pan;
	if ( PanDelta < 0.0f )
	{
		PanDelta = -PanDelta;
	}
	if ( ( VolDelta > 0.03f ) || ( PanDelta > 0.02f ) )
	{
		return GE_TRUE;
	}
	return GE_FALSE;

} // Snd_Get3dSoundValues()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Snd_Create()
//
////////////////////////////////////////////////////////////////////////////////////////
static void * Snd_Create(
	EffectResource	*ExternalResource,	// external resources
	int				TypeID )			// effect type id
{

	// locals
	SndResource	*Resource;

	// ensure valid data
	assert( ExternalResource != NULL );
	assert( TypeID > 0 );

	// allocate the resource list struct
	Resource = geRam_AllocateClear( sizeof( *Resource ) );
	if ( Resource == NULL )
	{
		geErrorLog_AddString( GE_ERR_MEMORY_RESOURCE, "Snd_Create: failed to create SndResource struct.", NULL );
		return NULL;
	}
	
	// save passed resource list
	Resource->ExternalResource = ExternalResource;

	// save type id
	Resource->TypeID = TypeID;

	// all done
	return Resource;

} // Snd_Create()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Snd_Destroy()
//
////////////////////////////////////////////////////////////////////////////////////////
static void Snd_Destroy(
	void	**ResourceToZap )	// resource list 
{

	// locals
	SndResource	*Resource;

	// ensure valid data
	assert( ResourceToZap != NULL );
	assert( *ResourceToZap != NULL );

	// get resource list
	Resource = *ResourceToZap;

	// zap resource list
	geRam_Free( Resource );
	ResourceToZap = NULL;

} // Snd_Destroy()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Snd_Add()
//
////////////////////////////////////////////////////////////////////////////////////////
static void * Snd_Add(
	SndResource	*Resource,		// available resources
	Snd			*Data,			// data of effect
	Snd			*AttachData )	// data of effect to attach to
{

	// locals
	Snd		*NewData;
	geFloat	Volume;
	geFloat	Pan;
	geFloat	Frequency;

	// ensure valid data
	assert( Resource != NULL );
	assert( Data != NULL );

	// fail if type ids don't match
	if ( Resource->TypeID != Data->TypeID )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Snd_Add: effect struct and effect type don't match.", NULL );
		return NULL;
	}

	// don't create anything if there is no sound system
	if ( Resource->ExternalResource->Sound == NULL )
	{
		return NULL;
	}

	// allocate Snd data
	NewData = geRam_Allocate( sizeof( *NewData ) );
	if ( NewData == NULL )
	{
		geErrorLog_AddString( GE_ERR_MEMORY_RESOURCE, "Snd_Add: failed to create Snd struct.", NULL );
		return NULL;
	}

	// copy passed data
	memcpy( NewData, Data, sizeof( *NewData ) );

	// fail if we have any bad data
	if ( NewData->SoundDef == NULL )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Snd_Add: effect not added, SoundDef is bad.", NULL );
		return NULL;
	}
	if ( NewData->Min < 0.0f )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Snd_Add: effect not added, sound min value is bad.", NULL );
		return NULL;
	}

	// play the sound
	Snd_Get3dSoundValues( Resource->ExternalResource, NewData, &Volume, &Pan, &Frequency );
	NewData->Sound = geSound_PlaySoundDef(	Resource->ExternalResource->Sound,
											NewData->SoundDef,
											Volume, Pan, Frequency,
											NewData->Loop );
	if ( NewData->Sound == NULL )
	{
		geErrorLog_AddString( GE_ERR_SOUND_RESOURCE, "Snd_Add: effect not added, failed to play sound def.", NULL );
		Snd_Remove( Resource, NewData );
		return NULL;
	}
	Data->LastVolume = Volume;
	Data->LastPan = Pan;

	// all done
	return NewData;

	// get rid of warnings
	AttachData;

} // Snd_Add()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Snd_Remove()
//
////////////////////////////////////////////////////////////////////////////////////////
static void Snd_Remove(
	SndResource	*Resource,	// available resources
	Snd			*Data )		// effect data
{

	// ensure valid data
	assert( Resource != NULL );
	assert( Data != NULL );

	// stop the sound
	if ( Data->Sound != NULL )
	{
		geSound_StopSound( Resource->ExternalResource->Sound, Data->Sound );
	}

	// free effect data
	geRam_Free( Data );

} // Snd_Remove()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Snd_Process()
//
//	Perform processing on an indivual effect. A return of GE_FALSE means that the
//	effect needs to be removed.
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean Snd_Process(
	SndResource	*Resource,		// available resources
	float		TimeDelta,		// elapsed time
	Snd			*Data )			// effect data
{

	// locals
	geBoolean	Result;
	geFloat		Volume;
	geFloat		Pan;
	geFloat		Frequency;

	// ensure valid data
	assert( Resource != NULL );
	assert( TimeDelta > 0.0f );
	assert( Data != NULL );

	// stop the sound if required...
	if ( Data->Sound != NULL )
	{

		// if the sound is done then zap this effect
		if (	( Data->Loop == GE_FALSE ) &&
				( geSound_SoundIsPlaying( Resource->ExternalResource->Sound, Data->Sound ) == GE_FALSE ) )
		{
			return GE_FALSE;
		}

		// adjust the sound if required
		if ( Snd_Get3dSoundValues( Resource->ExternalResource, Data, &Volume, &Pan, &Frequency ) == GE_TRUE )
		{
			Result = geSound_ModifySound(	Resource->ExternalResource->Sound,
											Data->Sound,
											Volume, Pan, Frequency );
			if ( Result == GE_FALSE )
			{
				geErrorLog_AddString( GE_ERR_SOUND_RESOURCE, "Snd_Process: sound modify failed.", NULL );
				return GE_TRUE;
			}
			Data->LastVolume = Volume;
			Data->LastPan = Pan;
		}

		// display debug info
		#ifdef SND_DEBUGINFO
		geEngine_Printf( Resource->ExternalResource->Engine, 100, 100, "Last:  Vol:%.2f Pan:%.2f", Data->LastVolume, Data->LastPan );
		geEngine_Printf( Resource->ExternalResource->Engine, 100, 120, "Cur:   Vol:%.2f Pan:%.2f", Volume, Pan );
		geEngine_Printf( Resource->ExternalResource->Engine, 100, 140, "Delta: Vol:%.2f Pan:%.2f", fabs( Data->LastVolume - Volume ), fabs( Data->LastPan - Pan ) );
		#endif

		// stop the sound if its volume is out of hearing range
		if ( Data->Loop == GE_TRUE )
		{
			if ( Data->LastVolume < SND_MINAUDIBLEVOLUME )
			{
				geSound_StopSound( Resource->ExternalResource->Sound, Data->Sound );
				Data->Sound = NULL;
				Data->LastVolume = 0;
				Data->LastPan = 0;
			}
		}
	}
	// ...or restart it
	else
	{

		// only restart looping non paused sounds
		if (	( Data->Loop == GE_TRUE ) &&
				( Data->Paused == GE_FALSE ) )
		{

			// restart it if its volume is now in hearing range
			Snd_Get3dSoundValues( Resource->ExternalResource, Data, &( Data->LastVolume ), &( Data->LastPan ), &Frequency );
			if ( Data->LastVolume >= SND_MINAUDIBLEVOLUME )
			{
				Data->Sound = geSound_PlaySoundDef(	Resource->ExternalResource->Sound,
													Data->SoundDef,
													Data->LastVolume, Data->LastPan, Frequency,
													Data->Loop );
				if( Data->Sound == NULL )
				{
					geErrorLog_AddString( GE_ERR_SOUND_RESOURCE, "Snd_Process: sound def play failed.", NULL );
					return GE_TRUE;
				}

			}
			else
			{
				Data->LastVolume = 0;
				Data->LastPan = 0;
			}
		}
	}

	// all done
	return GE_TRUE;

	// get rid of warnings
	TimeDelta;

} // Snd_Process()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Snd_Frame()
//
//	Perform once-per-frame processing for all effects of this type.
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean Snd_Frame(
	SndResource	*Resource,		// available resources
	float		TimeDelta )		// elapsed time
{

	// all done
	return GE_TRUE;

	// get rid of warnings
	Resource;
	TimeDelta;

} // Snd_Frame()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Snd_Modify()
//
//	Adjust the effect.
//
////////////////////////////////////////////////////////////////////////////////////////
geBoolean Snd_Modify(
	SndResource	*Resource,	// available resources
	Snd			*Data,		// effect data
	Snd			*NewData,	// new data
	uint32		Flags )		// user flags
{

	// ensure valid data
	assert( Resource != NULL );
	assert( Data != NULL );
	assert( NewData != NULL );

	// adjust the position
	if ( Flags & SND_POS )
	{

		// save new position
		geVec3d_Copy( &( NewData->Pos ), &( Data->Pos ) );

		// adjust the sound
		if ( Data->Sound != NULL )
		{

			// locals
			geBoolean	Result;
			geFloat		Volume;
			geFloat		Pan;
			geFloat		Frequency;

			// adjust the sound
			if ( Snd_Get3dSoundValues( Resource->ExternalResource, Data, &Volume, &Pan, &Frequency ) == GE_TRUE )
			{
				Result = geSound_ModifySound(	Resource->ExternalResource->Sound,
												Data->Sound,
												Volume, Pan, Frequency );
				if( Result == GE_FALSE )
				{
					geErrorLog_AddString( GE_ERR_SOUND_RESOURCE, "Snd_Modify: sound modify failed.", NULL );
					return GE_FALSE;
				}

				Data->LastVolume = Volume;
				Data->LastPan = Pan;
			}
		}
	}

	// all done
	return GE_TRUE;

} // Snd_Modify()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Snd_Pause()
//
//	Pause/unpause effect.
//
////////////////////////////////////////////////////////////////////////////////////////
void Snd_Pause(
	SndResource	*Resource,	// available resources
	Snd			*Data,		// effect data
	geBoolean	Pause )		// new pause state
{

	// ensure valid data
	assert( Resource != NULL );
	assert( Data != NULL );

	// pause the sound...
	if (	( Pause == GE_TRUE ) &&
			( Data->Paused == GE_FALSE ) )
	{
		if ( Data->Sound != NULL )
		{
			geSound_StopSound( Resource->ExternalResource->Sound, Data->Sound );
			Data->Sound = NULL;
			Data->LastVolume = 0;
			Data->LastPan = 0;
		}
		Data->Paused = GE_TRUE;
	}
	// ...or start it up again
	else if (	( Pause == GE_FALSE ) &&
				( Data->Paused == GE_TRUE ) )
	{

		// locals
		geFloat	Frequency;

		// play the sound
		if (	( Data->Sound == NULL ) ||
				( geSound_SoundIsPlaying( Resource->ExternalResource->Sound, Data->Sound ) == GE_FALSE ) )
		{
			Snd_Get3dSoundValues( Resource->ExternalResource, Data, &( Data->LastVolume ), &( Data->LastPan ), &Frequency );
			Data->Sound = geSound_PlaySoundDef(	Resource->ExternalResource->Sound,
												Data->SoundDef,
												Data->LastVolume, Data->LastPan, Frequency,
												Data->Loop );
			if( Data->Sound == NULL )
			{
				geErrorLog_AddString( GE_ERR_SOUND_RESOURCE, "Snd_Pause: sound def play failed.", NULL );
				return;
			}
			Data->Paused = GE_FALSE;
		}
	}

} // Snd_Pause()
