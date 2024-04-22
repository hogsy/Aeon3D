////////////////////////////////////////////////////////////////////////////////////////
//  ChaosM.c									                                        
//                                                                                      
//  Author: Peter Siamidis                                                              
//  Description:    
//			Modify a texture so that it seems bustling with activity.
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
#include <math.h>
#include <string.h>
#include <memory.h>
#include <assert.h>
#include "Ram.h"
#include "EffectMI.h"
#include "EffectC.h"


////////////////////////////////////////////////////////////////////////////////////////
//	Hardwired stuff
////////////////////////////////////////////////////////////////////////////////////////
#define GE_TWOPI		( GE_PI * 2.0f )
#define CHAOS_FORMAT	GE_PIXELFORMAT_24BIT_RGB


////////////////////////////////////////////////////////////////////////////////////////
//	Custom data
////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	geActor		*Actor;
	geBitmap	*AttachBmp;
	geBitmap	*OriginalBmp;
	geBitmap	*WorkBmp;
	float		XOffset;
	float		YOffset;
	float		XStep;
	float		YStep;
	int			SegmentSize;
	int			SegmentCount;
	int			XMaxSway;
	int			YMaxSway;

} Custom;


////////////////////////////////////////////////////////////////////////////////////////
//	Interface setup
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean	ChaosM_Create( EffectManager *EManager, EffectM_Root *Root );
static void			ChaosM_Destroy( EffectManager *EManager, EffectM_Root *Root );
static geBoolean	ChaosM_Update( EffectManager *EManager, EffectM_Root *Root );
EffectM_Interface	ChaosM_Interface =
{
	ChaosM_Create,
	ChaosM_Destroy,
	ChaosM_Update
};



////////////////////////////////////////////////////////////////////////////////////////
//
//	ChaosM_Create()
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean ChaosM_Create(
	EffectManager	*EManager,	// effect manager to add it to
	EffectM_Root	*Root )		// root data
{

	// locals
	EM_Chaos		*Ch;
	Custom			*Cus;
	geBitmap_Info	AttachInfo;
	geBoolean		Result;

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );

	// disabled for software mode
	if ( EManager->SoftwareMode == GE_TRUE )
	{
		return GE_FALSE;
	}

	// get data
	Ch = Root->UserData;
	assert( Ch != NULL );
	assert( Root->UserDataSize == sizeof( *Ch ) );

	// don't launch this effect if its trigger based
	if ( EffectC_IsStringNull( Ch->TriggerName ) == GE_FALSE )
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
	Cus->XMaxSway = Ch->MaxXSway;
	Cus->YMaxSway = Ch->MaxYSway;
	Cus->SegmentSize = 1;
	Cus->XStep = Ch->XStep;
	Cus->YStep = Ch->YStep;

	// get the attach bitmap as an actor bitmap...
	if ( ( ( Ch->AStart != NULL ) && ( Ch->AStart->Actor != NULL ) ) || ( Ch->Actor != NULL ) )
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
		if ( ( Ch->AStart != NULL ) && ( Ch->AStart->Actor != NULL ) )
		{
			Cus->Actor = Ch->AStart->Actor;
		}
		else
		{
			Cus->Actor = Ch->Actor;
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
		Length = strlen( Ch->AttachBmp );
		for ( i = 0; i < MaterialCount; i++ )
		{
			if ( geBody_GetMaterial( Body, i, &MaterialName, &( Cus->AttachBmp ), &R, &G, &B ) == GE_FALSE )
			{
				continue;
			}
			if ( strnicmp( Ch->AttachBmp, MaterialName, Length ) == 0 )
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
		Cus->AttachBmp = geWorld_GetBitmapByName( Effect_GetWorld( EManager->ESystem ), Ch->AttachBmp );
		if ( Cus->AttachBmp == NULL )
		{
			return GE_FALSE;
		}
	}

	// prepare the output bitmap
	Result = geBitmap_SetFormatMin( Cus->AttachBmp, CHAOS_FORMAT );
	if ( Result != GE_TRUE )
	{
		return GE_FALSE;
	}
	geBitmap_ClearMips( Cus->AttachBmp );
	if ( geBitmap_GetInfo( Cus->AttachBmp, &AttachInfo, NULL ) == GE_FALSE )
	{
		return GE_FALSE;
	}
	Cus->SegmentCount = AttachInfo.Width / Cus->SegmentSize;

	// fail if the sway amount is bigger than the texture size
	if ( ( Cus->XMaxSway >= AttachInfo.Width ) || ( Cus->YMaxSway >= AttachInfo.Height ) )
	{
		return GE_FALSE;
	}

	// keep a copy of the attach bitmap
	Cus->OriginalBmp = geBitmap_Create( AttachInfo.Width, AttachInfo.Height, 1, CHAOS_FORMAT );
	if ( Cus->OriginalBmp == NULL )
	{
		return GE_FALSE;
	}
	geBitmap_ClearMips( Cus->OriginalBmp );
	geBitmap_BlitBitmap( Cus->AttachBmp, Cus->OriginalBmp );

	// create a work bitmap
	Cus->WorkBmp = geBitmap_Create( AttachInfo.Width, AttachInfo.Height, 1, CHAOS_FORMAT );
	if ( Cus->WorkBmp == NULL )
	{
		return GE_FALSE;
	}
	geBitmap_ClearMips( Cus->WorkBmp );

	// all done
	return GE_TRUE;

} // ChaosM_Create()



////////////////////////////////////////////////////////////////////////////////////////
//
//	ChaosM_Destroy()
//
////////////////////////////////////////////////////////////////////////////////////////
static void ChaosM_Destroy(
	EffectManager	*EManager,	// effect manager in which it exists
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

	// zap custom stuff
	if ( Cus != NULL )
	{

		// restore original art
		if ( ( Cus->OriginalBmp != NULL ) && ( Cus->AttachBmp != NULL ) )
		{
			geBitmap_BlitBitmap( Cus->OriginalBmp, Cus->AttachBmp );
		}

		// zap copy bitmap
		if ( Cus->OriginalBmp != NULL )
		{
			geBitmap_Destroy( &( Cus->OriginalBmp ) );
		}

		// zap work bitmap
		if ( Cus->WorkBmp != NULL )
		{
			geBitmap_Destroy( &( Cus->WorkBmp ) );
		}
	}

	// get rid of warnings
	EManager;

} // ChaosM_Destroy()



////////////////////////////////////////////////////////////////////////////////////////
//
//	ChaosM_Update()
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean ChaosM_Update(
	EffectManager	*EManager,	// effect manager to add it to
	EffectM_Root	*Root )		// root data
{

	// locals
	Custom			*Cus;
	geBitmap_Info	AttachInfo, OriginalInfo;
	geBoolean		Result;
	int				Row, Col;
	int				XPos, YPos;
	float			CosStep;
	float			CurYOffset;
	float			CurXOffset;

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );

	// get custom data
	Cus = Root->Custom;
	assert( Cus != NULL );

	// do nothing if the texture is not visible
	if ( Cus->Actor == NULL )
	{
		if ( geWorld_BitmapIsVisible( Effect_GetWorld( EManager->ESystem ), Cus->AttachBmp ) == GE_FALSE )
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

	// make sure everything jives
	if ( geBitmap_GetInfo( Cus->AttachBmp, &AttachInfo, NULL ) == GE_FALSE )
	{
		return GE_FALSE;
	}
	if ( geBitmap_GetInfo( Cus->OriginalBmp, &OriginalInfo, NULL ) == GE_FALSE )
	{
		return GE_FALSE;
	}
	assert( OriginalInfo.Height == AttachInfo.Height );
	assert( OriginalInfo.Width  >= AttachInfo.Width  );

	// compute vertical offset
	Cus->YOffset += (float)Cus->YStep * Root->TimeDelta;
	if ( Cus->YOffset > GE_TWOPI )
	{
		Cus->YOffset = 0.0f;
	}
	CosStep = GE_TWOPI / (float)Cus->SegmentCount;
	CurYOffset = Cus->YOffset;

	// adjust vertically
	for ( Col = 0; Col < Cus->SegmentCount; Col++ )
	{

		// adjust offset
		CurYOffset += CosStep;
		if ( CurYOffset > GE_PI )
		{
			CosStep = -CosStep;
			CurYOffset = GE_PI - ( CurYOffset - GE_PI );
		}
		else if ( CurYOffset < 0.0 )
		{
			CosStep = -CosStep;
			CurYOffset = -CurYOffset;
		}

		// compute positions
		XPos = Col * Cus->SegmentSize;
		YPos = (int)( ( (float)cos( CurYOffset ) + 1.0f ) * ( (float)Cus->YMaxSway / 2.0f ) );

		// adjust bitmap
		Result = geBitmap_Blit( Cus->OriginalBmp, XPos, 0, Cus->WorkBmp, XPos, YPos, Cus->SegmentSize, AttachInfo.Height - YPos );
		assert( Result == GE_TRUE );
		Result = geBitmap_Blit( Cus->OriginalBmp, XPos, AttachInfo.Height - YPos, Cus->WorkBmp, XPos, 0, Cus->SegmentSize, YPos );
		assert( Result == GE_TRUE );
	}

	// compute horizontal offset
	Cus->XOffset += (float)Cus->XStep * Root->TimeDelta;
	if ( Cus->XOffset > GE_TWOPI )
	{
		Cus->XOffset = 0.0f;
	}
	CosStep = GE_TWOPI / (float)Cus->SegmentCount;
	CurXOffset = Cus->XOffset;
	
	// adjust horizontally
	for ( Row = 0; Row < Cus->SegmentCount; Row++ )
	{

		// adjust offset
		CurXOffset += CosStep;
		if ( CurXOffset > GE_PI )
		{
			CosStep = -CosStep;
			CurXOffset = GE_PI - ( CurXOffset - GE_PI );
		}
		else if ( CurXOffset < 0.0 )
		{
			CosStep = -CosStep;
			CurXOffset = -CurXOffset;
		}

		// compute positions
		XPos = (int)( ( (float)cos( CurXOffset ) + 1.0f ) * ( (float)Cus->XMaxSway / 2.0f ) );
		YPos = Row * Cus->SegmentSize;

		// adjust bitmap
		Result = geBitmap_Blit( Cus->WorkBmp, 0, YPos, Cus->AttachBmp, XPos, YPos, AttachInfo.Width - XPos, Cus->SegmentSize );
		assert( Result == GE_TRUE );
		Result = geBitmap_Blit( Cus->WorkBmp, AttachInfo.Width - XPos, YPos, Cus->AttachBmp, 0, YPos, XPos, Cus->SegmentSize );
		assert( Result == GE_TRUE );
	}

	// all done
	return GE_TRUE;

} // ChaosM_Update()
