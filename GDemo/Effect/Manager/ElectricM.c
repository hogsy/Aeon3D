////////////////////////////////////////////////////////////////////////////////////////
//  ElectricM.c									                                        
//                                                                                      
//  Author: Peter Siamidis                                                              
//  Description:    
//			Electric bolt effect.
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
//	Electric bolt effect.
//	-Actor:			NOT SUPPORTED
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
	geVec3d			Source;
	geVec3d			Dest;
	geWorld_Model	*SourceModel;
	geWorld_Model	*DestModel;
	geActor			*SourceActor;
	geActor			*DestActor;
	char			*SourceBone;
	char			*DestBone;
	float			MinPauseTime;
	float			MaxPauseTime;
	float			MinFireTime;
	float			MaxFireTime;
	geBoolean		Paused;
	float			CurDelay;
	geBoolean		SupportDelay;

} Custom;


////////////////////////////////////////////////////////////////////////////////////////
//	Interface setup
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean	ElectricM_Create( EffectManager *EManager, EffectM_Root *Root );
static void			ElectricM_Destroy( EffectManager *EManager, EffectM_Root *Root );
static geBoolean	ElectricM_Update( EffectManager *EManager, EffectM_Root *Root );
EffectM_Interface	ElectricM_Interface =
{
	ElectricM_Create,
	ElectricM_Destroy,
	ElectricM_Update
};



////////////////////////////////////////////////////////////////////////////////////////
//
//	ElectricM_Create()
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean ElectricM_Create(
	EffectManager	*EManager,	// effect manager to add it to
	EffectM_Root	*Root )		// root data
{

	// locals
	EM_Electric		*El;
	Custom			*Cus;
	Bolt			Bl;
	int				TextureNumber;
	geXForm3d		Xf;

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );

	// get data
	El = Root->UserData;
	assert( El != NULL );
	assert( Root->UserDataSize == sizeof( *El ) );

	// properly null stuff thats been messed up by the editor
	if ( EffectC_IsStringNull( El->AlphaName ) == GE_TRUE )
	{
		El->AlphaName = NULL;
	}
	if ( EffectC_IsStringNull( El->SoundName ) == GE_TRUE )
	{
		El->SoundName = NULL;
	}

	// init remaining root data
	Root->EffectCount = 2;
	Root->EffectList = geRam_Allocate( sizeof( int ) * Root->EffectCount );
	if ( Root->EffectList == NULL )
	{
		return GE_FALSE;
	}
	Root->EffectList[0] = -1;
	Root->EffectList[1] = -1;

	// init custom data
	Root->Custom = geRam_AllocateClear( sizeof( *Cus ) );
	if ( Root->Custom == NULL )
	{
		return GE_FALSE;
	}
	Cus = Root->Custom;
	Cus->MinPauseTime = El->MinPauseTime;
	Cus->MaxPauseTime = El->MaxPauseTime;
	Cus->MinFireTime = El->MinFireTime;
	Cus->MaxFireTime = El->MaxFireTime;
	Cus->Paused = GE_FALSE;
	assert( Cus->MinPauseTime >= 0.0f );
	assert( Cus->MaxPauseTime >= Cus->MinPauseTime );
	assert( Cus->MinFireTime >= 0.0f );
	assert( Cus->MaxFireTime >= Cus->MinFireTime );
	if ( ( Cus->MaxPauseTime > 0.0f ) && ( Cus->MaxFireTime > 0.0f ) )
	{
		Cus->SupportDelay = GE_TRUE;
	}

	// setup source position based on actor bone...
	if ( ( El->Source->AStart != NULL ) && ( El->Source->AStart->Actor != NULL ) )
	{

		// locals
		geXForm3d	Xf;
		geBoolean	Result;

		// save actor that its locked to
		Cus->SourceActor = El->Source->AStart->Actor;

		// save bone name
		Cus->SourceBone = geRam_Allocate( strlen( El->Source->BoneName ) + 1 );
		if ( Cus->SourceBone == NULL )
		{
			return GE_FALSE;
		}
		strcpy( Cus->SourceBone, El->Source->BoneName );

		// get bone location
		Result = geActor_GetBoneTransform( Cus->SourceActor, Cus->SourceBone, &Xf );
		if ( Result == GE_FALSE )
		{
			return GE_FALSE;
		}

		// setup position
		geVec3d_Copy( &( Xf.Translation ), &( Cus->Source ) );
	}
	// ...or based on model...
	else if ( El->Source->Model != NULL )
	{
		Cus->SourceModel = El->Source->Model;
		geWorld_GetModelXForm( Effect_GetWorld( EManager->ESystem ), Cus->SourceModel, &Xf );
		geVec3d_Copy( &( Xf.Translation ), &( Cus->Source ) );
	}
	// ...or based on position
	else
	{
		geVec3d_Copy( &( El->Source->Position ), &( Cus->Source ) );
	}

	// setup dest position based on actor bone...
	if ( ( El->Dest->AStart != NULL ) && ( El->Dest->AStart->Actor != NULL ) )
	{

		// locals
		geXForm3d	Xf;
		geBoolean	Result;

		// save actor that its locked to
		Cus->DestActor = El->Dest->AStart->Actor;

		// save bone name
		Cus->DestBone = geRam_Allocate( strlen( El->Dest->BoneName ) + 1 );
		if ( Cus->DestBone == NULL )
		{
			return GE_FALSE;
		}
		strcpy( Cus->DestBone, El->Dest->BoneName );

		// get bone location
		Result = geActor_GetBoneTransform( Cus->DestActor, Cus->DestBone, &Xf );
		if ( Result == GE_FALSE )
		{
			return GE_FALSE;
		}

		// setup position
		geVec3d_Copy( &( Xf.Translation ), &( Cus->Dest ) );
	}
	// ...or based on model...
	else if ( El->Dest->Model != NULL )
	{
		Cus->DestModel = El->Dest->Model;
		geWorld_GetModelXForm( Effect_GetWorld( EManager->ESystem ), Cus->DestModel, &Xf );
		geVec3d_Copy( &( Xf.Translation ), &( Cus->Dest ) );
	}
	// ...or based on position
	else
	{
		geVec3d_Copy( &( El->Dest->Position ), &( Cus->Dest ) );
	}

	// show bolt effect
	memset( &Bl, 0, sizeof( Bl ) );
	geVec3d_Copy( &( Cus->Source ), &( Bl.Start ) );
	geVec3d_Copy( &( Cus->Dest ), &( Bl.End ) );
	Bl.SegmentLength = El->SegmentLength;
	assert( Bl.SegmentLength > 0 );
	Bl.SegmentWidth = El->SegmentWidth;
	assert( Bl.SegmentWidth > 0.0f );
	Bl.Offset = El->Offset;
	assert( Bl.Offset >= 0.0f );
	Bl.BoltLimit = El->BoltSegmentLimit;
	assert( Bl.BoltLimit > 0 );
	Bl.Loop = GE_TRUE;
	Bl.Color.r = El->Color.r;
	assert( Bl.Color.r <= 255.0f );
	assert( Bl.Color.r >= -255.0f );
	Bl.Color.g = El->Color.r;
	assert( Bl.Color.g <= 255.0f );
	assert( Bl.Color.g >= -255.0f );
	Bl.Color.b = El->Color.r;
	assert( Bl.Color.b <= 255.0f );
	assert( Bl.Color.b >= -255.0f );
	Bl.Color.a = 255.0f;
	TextureNumber = TPool_Add( EManager->TPool, NULL, El->BmpName, El->AlphaName );
	if ( TextureNumber == -1 )
	{
		return GE_FALSE;
	}
	Bl.Texture = TPool_Get( EManager->TPool, TextureNumber );
	if ( Bl.Texture == NULL )
	{
		return GE_FALSE;
	}
	if ( Effect_New( EManager->ESystem, Effect_Bolt, &Bl, 0, NULL, NULL, &( Root->EffectList[0] ) ) == GE_FALSE )
	{
		return GE_FALSE;
	}

	// add sound effect
	if ( ( EManager->SPool != NULL ) && ( El->SoundName != NULL ) )
	{

		// locals
		int		SoundNumber;
		geVec3d	Pos;
		Snd		Sound;

		// get sound
		memset( &Sound, 0, sizeof( Sound ) );
		SoundNumber = SPool_Add( EManager->SPool, NULL, El->SoundName );
		if ( SoundNumber == -1 )
		{
			return GE_FALSE;
		}
		Sound.SoundDef = SPool_Get( EManager->SPool, SoundNumber );
		if ( Sound.SoundDef == NULL )
		{
			return GE_FALSE;
		}

		// setup position
		geVec3d_Subtract( &( Cus->Dest ), &( Cus->Source ), &Pos );
		geVec3d_AddScaled( &( Cus->Source ), &Pos, 0.5f, &( Sound.Pos ) );

		// play sound
		Sound.Min = El->SoundMinDistance;
		assert( Sound.Min >= 0.0f );
		Sound.Loop = GE_TRUE;
		if ( Effect_New( EManager->ESystem, Effect_Sound, &Sound, 0, NULL, NULL, &( Root->EffectList[1] ) ) == GE_FALSE )
		{
			return GE_FALSE;
		}
	}
	else
	{
		Root->EffectList[1] = -1;
	}

	// all done
	return GE_TRUE;

} // ElectricM_Create()



////////////////////////////////////////////////////////////////////////////////////////
//
//	ElectricM_Destroy()
//
////////////////////////////////////////////////////////////////////////////////////////
static void ElectricM_Destroy(
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

		// free source bone name
		if ( Cus->SourceBone != NULL )
		{
			geRam_Free( Cus->SourceBone );
		}

		// free dest bone name
		if ( Cus->DestBone != NULL )
		{
			geRam_Free( Cus->DestBone );
		}
	}

	// get rid of warnings
	EManager;

} // ElectricM_Destroy()



////////////////////////////////////////////////////////////////////////////////////////
//
//	ElectricM_Update()
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean ElectricM_Update(
	EffectManager	*EManager,	// effect manager to add it to
	EffectM_Root	*Root )		// root data
{

	// locals
	uint32		Flags;
	Bolt		Bl;
	geXForm3d	Xf;
	Custom		*Cus;
	geBoolean	Result;

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );

	// get custom data
	Cus = Root->Custom;
	assert( Cus != NULL );

	// adjust cur delay amount
	if ( Cus->SupportDelay == GE_TRUE )
	{
		Cus->CurDelay -= Root->TimeDelta;
		if ( Cus->CurDelay < 0.0f )
		{
			Cus->Paused = !Cus->Paused;
			if ( Cus->Paused == GE_TRUE )
			{
				Cus->CurDelay = EffectC_Frand( Cus->MinPauseTime, Cus->MaxPauseTime );
			}
			else
			{
				Cus->CurDelay = EffectC_Frand( Cus->MinFireTime, Cus->MaxFireTime );
			}
			Effect_SetPause( EManager->ESystem, Root->EffectList[0], Cus->Paused );
			Effect_SetPause( EManager->ESystem, Root->EffectList[1], Cus->Paused );
		}
	}

	// setup source
	Flags = 0;
	if ( Cus->SourceActor != NULL )
	{

		// get bone location
		Result = geActor_GetBoneTransform( Cus->SourceActor, Cus->SourceBone, &Xf );
		if ( Result == GE_FALSE )
		{
			return GE_FALSE;
		}

		// setup position
		geVec3d_Copy( &( Xf.Translation ), &( Bl.Start ) );
		Flags |= BOLT_START;
	}
	else if ( Cus->SourceModel != NULL )
	{
		geWorld_GetModelXForm( Effect_GetWorld( EManager->ESystem ), Cus->SourceModel, &Xf );
		geVec3d_Copy( &( Xf.Translation ), &( Bl.Start ) );
		Flags |= BOLT_START;
	}

	// setup dest
	if ( Cus->DestActor != NULL )
	{

		// get bone location
		Result = geActor_GetBoneTransform( Cus->DestActor, Cus->DestBone, &Xf );
		if ( Result == GE_FALSE )
		{
			return GE_FALSE;
		}

		// setup position
		geVec3d_Copy( &( Xf.Translation ), &( Bl.End ) );
		Flags |= BOLT_END;
	}
	else if ( Cus->DestModel != NULL )
	{
		geWorld_GetModelXForm( Effect_GetWorld( EManager->ESystem ), Cus->DestModel, &Xf );
		geVec3d_Copy( &( Xf.Translation ), &( Bl.End ) );
		Flags |= BOLT_END;
	}

	// make adjustments
	if ( Flags != 0 )
	{

		// adjust bolt
		Result = Effect_Modify(	EManager->ESystem,
								Effect_Bolt,
								&Bl,
								Root->EffectList[0],
								Flags );
		assert( Result == GE_TRUE );

		// adjust sound
		if ( Root->EffectList[1] != -1 )
		{

			// locals
			geVec3d	Pos;
			Snd		Sound;

			// setup sound position
			geVec3d_Subtract( &( Bl.End ), &( Bl.Start ), &Pos );
			geVec3d_AddScaled( &( Bl.Start ), &Pos, 0.5f, &( Sound.Pos ) );

			// adjust sound
			Result = Effect_Modify(	EManager->ESystem,
									Effect_Sound,
									&Sound,
									Root->EffectList[1],
									SND_POS );
			assert( Result == GE_TRUE );
		}
	}

	// all done
	return GE_TRUE;

} // ElectricM_Update()
