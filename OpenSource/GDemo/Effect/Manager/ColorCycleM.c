////////////////////////////////////////////////////////////////////////////////////////
//  ColorCycleM.c									                                        
//                                                                                      
//  Author: Peter Siamidis                                                              
//  Description:    
//			Blends the destinations palette from its own to another supplied one.
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
	geActor				*Actor;
	int					StageDir;
	float				DelayToNextStage;
	int					TotalStages;
	float				CurDelayToNextStage;
	geBoolean			Loop;
	geBitmap			*BlendBmp;
	int					CurStage;
	geBitmap_Palette	*DestPal;
	GE_RGBA				StartColor[256];
	GE_RGBA				DeltaColor[256];

} Custom;


////////////////////////////////////////////////////////////////////////////////////////
//	Interface setup
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean	ColorCycleM_Create( EffectManager *EManager, EffectM_Root *Root );
static void			ColorCycleM_Destroy( EffectManager *EManager, EffectM_Root *Root );
static geBoolean	ColorCycleM_Update( EffectManager *EManager, EffectM_Root *Root );
EffectM_Interface	ColorCycleM_Interface =
{
	ColorCycleM_Create,
	ColorCycleM_Destroy,
	ColorCycleM_Update
};



////////////////////////////////////////////////////////////////////////////////////////
//
//	ColorCycleM_Create()
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean ColorCycleM_Create(
	EffectManager	*EManager,	// effect manager to add it to
	EffectM_Root	*Root )		// root data
{

	// locals
	EM_ColorCycle		*Cc;
	Custom				*Cus;
	geBoolean			Result;
	geBitmap_Palette	*StartPal, *EndPal;

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );

	// get data
	Cc = Root->UserData;
	assert( Cc != NULL );
	assert( Root->UserDataSize == sizeof( *Cc ) );

	// fail if there are not enough blending stages
	if ( Cc->BlendStages < 2 )
	{
		return GE_FALSE;
	}

	// don't launch this effect if its trigger based
	if ( EffectC_IsStringNull( Cc->TriggerName ) == GE_FALSE )
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
	Cus->DelayToNextStage = Cus->CurDelayToNextStage = Cc->DelayToNextStage;
	Cus->TotalStages = Cc->BlendStages - 1;
	Cus->Loop = Cc->Loop;

	// get the output bitmap as an actor bitmap...
	if ( ( ( Cc->AStart != NULL ) && ( Cc->AStart->Actor != NULL ) ) || ( Cc->Actor != NULL ) )
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
		if ( ( Cc->AStart != NULL ) && ( Cc->AStart->Actor != NULL ) )
		{
			Cus->Actor = Cc->AStart->Actor;
		}
		else
		{
			Cus->Actor = Cc->Actor;
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
		Length = strlen( Cc->BitmapToAttachTo );
		for ( i = 0; i < MaterialCount; i++ )
		{
			if ( geBody_GetMaterial( Body, i, &MaterialName, &( Cus->BlendBmp ), &R, &G, &B ) == GE_FALSE )
			{
				continue;
			}
			if ( strnicmp( Cc->BitmapToAttachTo, MaterialName, Length ) == 0 )
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
		Cus->BlendBmp = geWorld_GetBitmapByName( Effect_GetWorld( EManager->ESystem ), Cc->BitmapToAttachTo );
		if ( Cus->BlendBmp == NULL )
		{
			return GE_FALSE;
		}
	}

	// prepare the output bitmap
	Result = geBitmap_SetFormatMin( Cus->BlendBmp, GE_PIXELFORMAT_8BIT );
	if ( Result != GE_TRUE )
	{
		return GE_FALSE;
	}


	// get start palette
	Cus->DestPal = StartPal = geBitmap_GetPalette( Cus->BlendBmp );
	if ( Cus->DestPal == NULL )
	{
		return GE_FALSE;
	}

	// get end palette
	{

		// locals
		int			TextureNumber;
		geBitmap	*EndBmp;

		// get end bitmap
		assert( Cc->BmpWithEndPalette != NULL );
		TextureNumber = TPool_Add( EManager->TPool, NULL, Cc->BmpWithEndPalette, NULL );
		if ( TextureNumber == -1 )
		{
			return GE_FALSE;
		}

		// get end palette
		EndBmp = TPool_Get( EManager->TPool, TextureNumber );
		if ( EndBmp == NULL )
		{
			return GE_FALSE;
		}
		EndPal = geBitmap_GetPalette( EndBmp );
		if ( EndPal == NULL )
		{
			return GE_FALSE;
		}
	}

	// save all colors
	{

		// locals
		int		i;
		GE_RGBA	EndColor;
		int		R, G, B, A;

		/// save all colors
		for ( i = 0; i < 256; i++ )
		{

			// get start color
			Result = geBitmap_Palette_GetEntryColor( StartPal, i, &R, &G, &B, &A );
			if ( Result == GE_FALSE )
			{
				Cus->StartColor[i].r = 0.0f;
				Cus->StartColor[i].g = 0.0f;
				Cus->StartColor[i].b = 0.0f;
				Cus->StartColor[i].a = 0.0f;
			}
			else
			{
				Cus->StartColor[i].r = (float)R;
				Cus->StartColor[i].g = (float)G;
				Cus->StartColor[i].b = (float)B;
				Cus->StartColor[i].a = (float)A;
			}

			// get end color
			Result = geBitmap_Palette_GetEntryColor( EndPal, i, &R, &R, &B, &A );
			if ( Result == GE_FALSE )
			{
				EndColor.r = 0.0f;
				EndColor.g = 0.0f;
				EndColor.b = 0.0f;
				EndColor.a = 0.0f;
			}
			else
			{
				EndColor.r = (float)R;
				EndColor.g = (float)G;
				EndColor.b = (float)B;
				EndColor.a = (float)A;
			}

			// setup color deltas
			Cus->DeltaColor[i].r = EndColor.r - Cus->StartColor[i].r;
			Cus->DeltaColor[i].g = EndColor.g - Cus->StartColor[i].g;
			Cus->DeltaColor[i].b = EndColor.b - Cus->StartColor[i].b;
			Cus->DeltaColor[i].a = EndColor.a - Cus->StartColor[i].a;
		}
	}

	// all done
	return GE_TRUE;

} // ColorCycleM_Create()



////////////////////////////////////////////////////////////////////////////////////////
//
//	ColorCycleM_Destroy()
//
////////////////////////////////////////////////////////////////////////////////////////
static void ColorCycleM_Destroy(
	EffectManager	*EManager,	// effect manager in which it exists
	EffectM_Root	*Root )		// root data
{

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );

	// get rid of warnings
	EManager;
	Root;

} // ColorCycleM_Destroy()



////////////////////////////////////////////////////////////////////////////////////////
//
//	ColorCycleM_Update()
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean ColorCycleM_Update(
	EffectManager	*EManager,	// effect manager to add it to
	EffectM_Root	*Root )		// root data
{

	// locals
	Custom		*Cus;
	geBoolean	Result;
	float		EndFrac;
	int			i;

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
		if ( geWorld_BitmapIsVisible( Effect_GetWorld( EManager->ESystem ), Cus->BlendBmp ) == GE_FALSE )
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

	// adjust all colors
	for ( i = 0; i < 256; i++ )
	{
		Result = geBitmap_Palette_SetEntryColor(	Cus->DestPal, i,
													(int)Cus->StartColor[i].r + (int)( Cus->DeltaColor[i].r * EndFrac ),
													(int)Cus->StartColor[i].g + (int)( Cus->DeltaColor[i].g * EndFrac ),
													(int)Cus->StartColor[i].b + (int)( Cus->DeltaColor[i].b * EndFrac ),
													(int)Cus->StartColor[i].a + (int)( Cus->DeltaColor[i].a * EndFrac ) );
		assert( Result == GE_TRUE );
	}

	// all done
	Result = geBitmap_SetPalette( Cus->BlendBmp, Cus->DestPal );
	assert( Result == GE_TRUE );
	return GE_TRUE;

} // ColorCycleM_Update()
