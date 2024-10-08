////////////////////////////////////////////////////////////////////////////////////////
//	MorphM.c
//                                                                                      
//  Author: Peter Siamidis                                                              
//  Description:    
//		Morph one texture into another.
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
//	-Actor:			OPTIONAL, if present then its texture is used, otherwise world texture
//	-User Data:		REQUIRED, all the settings are here
//	-Editor use:	NOT SUPPORTED
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
	geBitmap	*StartBmp;
	geBitmap	*EndBmp;
	geBitmap	*MorphBmp;
	int			TotalStages;
	int			CurStage;
	float		DelayToNextStage;
	float		CurDelayToNextStage;
	int			StageDir;
	geBoolean	Loop;

} Custom;


////////////////////////////////////////////////////////////////////////////////////////
//	Interface setup
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean	MorphM_Create( EffectManager *EManager, EffectM_Root *Root );
static void			MorphM_Destroy( EffectManager *EManager, EffectM_Root *Root );
static geBoolean	MorphM_Update( EffectManager *EManager, EffectM_Root *Root );
EffectM_Interface	MorphM_Interface =
{
	MorphM_Create,
	MorphM_Destroy,
	MorphM_Update
};



////////////////////////////////////////////////////////////////////////////////////////
//
//	MorphM_Create()
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean MorphM_Create(
	EffectManager	*EManager,	// effect manager to add it to
	EffectM_Root	*Root )		// root data
{

	// locals
	EM_Morph		*Mor;
	int				TextureNumber;
	Custom			*Cus;
	geBitmap_Info	StartInfo, EndInfo, MorphInfo;
	geBoolean		Result;

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );

	// morph is disabled for software mode
	if ( EManager->SoftwareMode == GE_TRUE )
	{
		return GE_FALSE;
	}

	// get data
	Mor = Root->UserData;
	assert( Mor != NULL );
	assert( Root->UserDataSize == sizeof( *Mor ) );

	// fail if there are not enough morphing stages
	if ( Mor->MorphStages < 2 )
	{
		return GE_FALSE;
	}

	// don't launch this effect if its trigger based
	if ( EffectC_IsStringNull( Mor->TriggerName ) == GE_FALSE )
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
	Root->Custom = geRam_AllocateClear( sizeof( *Cus ) );
	if ( Root->Custom == NULL )
	{
		return GE_FALSE;
	}
	Cus = Root->Custom;
	Cus->StageDir = 1;
	Cus->DelayToNextStage = Cus->CurDelayToNextStage = Mor->DelayToNextStage;
	Cus->TotalStages = Mor->MorphStages - 1;
	Cus->Loop = Mor->Loop;

	// get start bitmap
	assert( Mor->StartBmp != NULL );
	TextureNumber = TPool_Add( EManager->TPool, NULL, Mor->StartBmp, NULL );
	if ( TextureNumber == -1 )
	{
		return GE_FALSE;
	}
	Cus->StartBmp = TPool_Get( EManager->TPool, TextureNumber );
	if ( Cus->StartBmp == NULL )
	{
		return GE_FALSE;
	}
	if ( geBitmap_GetInfo( Cus->StartBmp, &StartInfo, NULL ) == GE_FALSE )
	{
		return GE_FALSE;
	}
	Result = geBitmap_SetFormatMin( Cus->StartBmp, GE_PIXELFORMAT_24BIT_RGB );
	if ( Result == GE_FALSE )
	{
		return GE_FALSE;
	}

	// get end bitmap
	assert( Mor->EndBmp != NULL );
	TextureNumber = TPool_Add( EManager->TPool, NULL, Mor->EndBmp, NULL );
	if ( TextureNumber == -1 )
	{
		return GE_FALSE;
	}
	Cus->EndBmp = TPool_Get( EManager->TPool, TextureNumber );
	if ( Cus->EndBmp == NULL )
	{
		return GE_FALSE;
	}
	if ( geBitmap_GetInfo( Cus->EndBmp, &EndInfo, NULL ) == GE_FALSE )
	{
		return GE_FALSE;
	}
	Result = geBitmap_SetFormatMin( Cus->EndBmp, GE_PIXELFORMAT_24BIT_RGB );
	if ( Result == GE_FALSE )
	{
		return GE_FALSE;
	}

	// fail if start end end sizes don't match
	if ( ( StartInfo.Width != EndInfo.Width ) || ( StartInfo.Height != EndInfo.Height ) )
	{
		return GE_FALSE;
	}

	// get the output bitmap as an actor bitmap...
	if ( ( ( Mor->AStart != NULL ) && ( Mor->AStart->Actor != NULL ) ) || ( Mor->Actor != NULL ) )
	{

		// locals
		geActor_Def	*ActorDef;
		geBody		*Body;
		int			MaterialCount;
		int			i;
		char		*MaterialName;
		float		R, G, B;
		int			Length;

		// save actor that its locked to
		if ( ( Mor->AStart != NULL ) && ( Mor->AStart->Actor != NULL ) )
		{
			Cus->Actor = Mor->AStart->Actor;
		}
		else
		{
			Cus->Actor = Mor->Actor;
		}

		// get actor material count
		ActorDef = geActor_GetActorDef( Cus->Actor );
		if ( ActorDef == NULL )
		{
			return GE_FALSE;
		}
		Body = geActor_GetBody( ActorDef );
		if ( Body == NULL )
		{
			return GE_FALSE;
		}
		MaterialCount = geActor_GetMaterialCount( Cus->Actor );

		// get bitmap pointer
		Length = strlen( Mor->BitmapToAttachTo );
		for ( i = 0; i < MaterialCount; i++ )
		{
			if ( geBody_GetMaterial( Body, i, &MaterialName, &( Cus->MorphBmp ), &R, &G, &B ) == GE_FALSE )
			{
				continue;
			}
			if ( strnicmp( Mor->BitmapToAttachTo, MaterialName, Length ) == 0 )
			{
				break;
			}
		}
		if ( i == MaterialCount )
		{
			return GE_FALSE;
		}
	}
	// ...or a world bitmap
	else
	{
		Cus->MorphBmp = geWorld_GetBitmapByName( Effect_GetWorld( EManager->ESystem ), Mor->BitmapToAttachTo );
		if ( Cus->MorphBmp == NULL )
		{
			return GE_FALSE;
		}
	}

	// prepare the output bitmap
	if ( geBitmap_GetInfo( Cus->MorphBmp, &MorphInfo, NULL ) == GE_FALSE )
	{
		return GE_FALSE;
	}
	if ( ( StartInfo.Width != MorphInfo.Width ) || ( StartInfo.Height != MorphInfo.Height ) )
	{
		return GE_FALSE;
	}
	Result = geBitmap_SetFormatMin( Cus->MorphBmp, GE_PIXELFORMAT_24BIT_RGB );
	if ( Result != GE_TRUE )
	{
		return GE_FALSE;
	}
	geBitmap_ClearMips( Cus->MorphBmp );

	// all done
	return GE_TRUE;

} // MorphM_Create()



////////////////////////////////////////////////////////////////////////////////////////
//
//	MorphM_Destroy()
//
////////////////////////////////////////////////////////////////////////////////////////
static void MorphM_Destroy(
	EffectManager	*EManager,	// effect manager in which it exists
	EffectM_Root	*Root )		// root data
{

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );

	// get rid of warnings
	EManager;
	Root;

} // MorphM_Destroy()



////////////////////////////////////////////////////////////////////////////////////////
//
//	MorphM_Update()
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean MorphM_Update(
	EffectManager	*EManager,	// effect manager to add it to
	EffectM_Root	*Root )		// root data
{

	// locals
	Custom			*Cus;
	geBitmap		*StartLocked = NULL;
	geBitmap		*EndLocked = NULL;
	geBitmap		*MorphLocked = NULL;
	geBitmap_Info	StartInfo, EndInfo, MorphInfo;
	uint8			*Start, *End, *Morph;
	uint8			*CurStart, *CurEnd, *CurMorph;
	int				Row, Col;
	geBoolean		Result;
	float			EndFrac;

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );

	// get custom data
	Cus = Root->Custom;
	assert( Cus != NULL );

	// do nothing if next stage is not ready
	Cus->CurDelayToNextStage -= Root->TimeDelta;
	if ( Cus->CurDelayToNextStage > 0.0f )
	{
		return GE_TRUE;
	}

	// adjust stage number
	Cus->CurDelayToNextStage = Cus->DelayToNextStage;;
	Cus->CurStage += Cus->StageDir;
	if ( ( Cus->CurStage > Cus->TotalStages ) || ( Cus->CurStage < 0 ) )
	{
		if ( Cus->Loop == GE_FALSE )
		{
			return GE_FALSE;
		}
		Cus->StageDir = -Cus->StageDir;
		Cus->CurStage += Cus->StageDir;
	}

	// do nothing further if the texture is not visible
	if ( Cus->Actor == NULL )
	{
		if ( geWorld_BitmapIsVisible( Effect_GetWorld( EManager->ESystem ), Cus->MorphBmp ) == GE_FALSE )
		{
			return GE_TRUE;
		}
	}
	else
	{
		if ( geWorld_IsActorPotentiallyVisible( Effect_GetWorld( EManager->ESystem ), Cus->Actor, Effect_GetCamera( EManager->ESystem ) ) == GE_FALSE )
		{
			return GE_TRUE;
		}
	}

	// setup frac
	EndFrac = (float)Cus->CurStage / (float)Cus->TotalStages;

	// lock start bitmap for reading
	if ( geBitmap_LockForRead( Cus->StartBmp, &StartLocked, 0, 0, GE_PIXELFORMAT_24BIT_RGB, GE_TRUE, 255 ) == GE_FALSE )
	{
		goto ALLDONE;
	}
	if ( geBitmap_GetInfo( StartLocked, &StartInfo, NULL ) == GE_FALSE )
	{
		goto ALLDONE;
	}
	Start = geBitmap_GetBits( StartLocked );
	if ( Start == NULL )
	{
		goto ALLDONE;
	}

	// lock end bitmap for reading
	if ( geBitmap_LockForRead( Cus->EndBmp, &EndLocked, 0, 0, GE_PIXELFORMAT_24BIT_RGB, GE_TRUE, 255 ) == GE_FALSE )
	{
		goto ALLDONE;
	}
	if ( geBitmap_GetInfo( EndLocked, &EndInfo, NULL ) == GE_FALSE )
	{
		goto ALLDONE;
	}
	End = geBitmap_GetBits( EndLocked );
	if ( End == NULL )
	{
		goto ALLDONE;
	}

	// lock the destination bitmap for writing
	if ( geBitmap_LockForWriteFormat( Cus->MorphBmp, &MorphLocked, 0, 0, GE_PIXELFORMAT_24BIT_RGB ) == GE_FALSE )
	{
		goto ALLDONE;
	}
	if ( geBitmap_GetInfo( MorphLocked, &MorphInfo, NULL ) == GE_FALSE )
	{
		goto ALLDONE;
	}
	Morph = geBitmap_GetBits( MorphLocked );
	if ( Morph == NULL )
	{
		goto ALLDONE;
	}

	// setup morph bitmap
	assert( StartInfo.Height >= MorphInfo.Height );
	assert( StartInfo.Width  >= MorphInfo.Width  );
	assert( EndInfo.Height   >= MorphInfo.Height );
	assert( EndInfo.Width    >= MorphInfo.Width  );
	for ( Row = 0; Row < MorphInfo.Height; Row++ )
	{

		// setup pointers
		CurMorph = Morph + ( MorphInfo.Stride * Row * 3 );
		CurStart = Start + ( StartInfo.Stride * Row * 3 );
		CurEnd = End + ( EndInfo.Stride * Row * 3 );

		// copy data
		for ( Col = 0; Col < MorphInfo.Width; Col++ )
		{

			// adjust pixel
			CurMorph[0] = CurStart[0] + (char)( (float)( CurEnd[0] - CurStart[0] ) * EndFrac );
			CurMorph[1] = CurStart[1] + (char)( (float)( CurEnd[1] - CurStart[1] ) * EndFrac );
			CurMorph[2] = CurStart[2] + (char)( (float)( CurEnd[2] - CurStart[2] ) * EndFrac );

			// adjust pointers
			CurMorph += 3;
			CurStart += 3;
			CurEnd += 3;
		}
	}

	// unlock all bitmaps
	ALLDONE:
	if ( StartLocked != NULL )
	{
		Result = geBitmap_UnLock( StartLocked );
		assert( Result == GE_TRUE );
	}
	if ( EndLocked != NULL )
	{
		Result = geBitmap_UnLock( EndLocked );
		assert( Result == GE_TRUE );
	}
	if ( MorphLocked != NULL )
	{
		Result = geBitmap_UnLock( MorphLocked );
		assert( Result == GE_TRUE );
	}

	// all done
	return GE_TRUE;

} // MorphM_Update()
