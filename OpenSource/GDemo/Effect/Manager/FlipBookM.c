////////////////////////////////////////////////////////////////////////////////////////
//  FlipbookM.c									                                        
//                                                                                      
//  Author: Peter Siamidis                                                              
//  Description:    
//		Plays back a sequence of sprites.
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
	geActor		*Actor;
	geBitmap	**BitmapList;
	geBitmap	*WorldBitmap;
	int			TextureCount;
	float		CycleSpeed;
	int			CurTexture;
	float		CurDelay;
	int			CycleStyle;
	int			CycleDir;

} Custom;


////////////////////////////////////////////////////////////////////////////////////////
//	Interface setup
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean	FlipbookM_Create( EffectManager *EManager, EffectM_Root *Root );
static void			FlipbookM_Destroy( EffectManager *EManager, EffectM_Root *Root );
static geBoolean	FlipbookM_Update( EffectManager *EManager, EffectM_Root *Root );
EffectM_Interface	FlipbookM_Interface =
{
	FlipbookM_Create,
	FlipbookM_Destroy,
	FlipbookM_Update
};



////////////////////////////////////////////////////////////////////////////////////////
//
//	FlipbookM_DeathCallback()
//
////////////////////////////////////////////////////////////////////////////////////////
static void FlipbookM_DeathCallback(
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

} // FlipbookM_DeathCallback()



////////////////////////////////////////////////////////////////////////////////////////
//
//	FlipbookM_Create()
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean FlipbookM_Create(
	EffectManager	*EManager,	// effect manager to add it to
	EffectM_Root	*Root )		// root data
{

	// locals
	EM_Flipbook	*Sq;
	Custom		*Cus;

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );

	// get data
	Sq = Root->UserData;
	assert( Sq != NULL );
	assert( Root->UserDataSize == sizeof( *Sq ) );
	
	// do nothing if no bitmap base name has been provided
	if ( EffectC_IsStringNull( Sq->BmpNameBase ) == GE_TRUE )
	{
		return GE_FALSE;
	}

	// don't launch this effect if its trigger based
	if ( EffectC_IsStringNull( Sq->TriggerName ) == GE_FALSE )
	{
		return GE_FALSE;
	}

	// properly null stuff thats been messed up by the editor
	if ( EffectC_IsStringNull( Sq->AlphaNameBase ) == GE_TRUE )
	{
		Sq->AlphaNameBase = NULL;
	}
	if ( EffectC_IsStringNull( Sq->WorldBitmapName ) == GE_TRUE )
	{
		Sq->WorldBitmapName = NULL;
	}

	// init custom data
	Root->Custom = geRam_AllocateClear( sizeof( Custom ) );
	if ( Root->Custom == NULL )
	{
		return GE_FALSE;
	}
	Cus = Root->Custom;
	Cus->TextureCount =  Sq->TextureCount;
	Cus->CycleSpeed = Sq->CycleSpeed;
	Cus->CycleStyle = Sq->CycleStyle;
	Cus->CycleDir = 1;

	// init remaining root data
	Root->EffectCount = 1;
	Root->EffectList = geRam_Allocate( sizeof( int ) * Root->EffectCount );
	if ( Root->EffectList == NULL )
	{
		return GE_FALSE;
	}
	Root->EffectList[0] = -1;

	// get world bitmap pointer
	if ( Sq->WorldBitmapName != NULL )
	{
		Cus->WorldBitmap = geWorld_GetBitmapByName( Effect_GetWorld( EManager->ESystem ), Sq->WorldBitmapName );
		if ( Cus->WorldBitmap == NULL )
		{
			return GE_FALSE;
		}
		geBitmap_ClearMips( Cus->WorldBitmap );
	}

	// setup textures
	{

		// locals
		char	BmpName[256];
		char	AlphaName[256];
		char	*Alpha;
		int		i;
		int		TextureNumber;

		// build texture list
		Cus->BitmapList = geRam_AllocateClear( sizeof( *( Cus->BitmapList ) ) * Sq->TextureCount );
		if ( Cus->BitmapList == NULL )
		{
			return GE_FALSE;
		}

		// process all textures
		for ( i = 0; i < Sq->TextureCount; i++ )
		{

			// build bmp and alpha names
			sprintf( BmpName, "%s%d%s", Sq->BmpNameBase, i, ".bmp" );
			if ( Sq->AlphaNameBase != NULL )
			{
				sprintf( AlphaName, "%s%d%s", Sq->AlphaNameBase, i, ".bmp" );
				Alpha = AlphaName;
			}
			else
			{
				Alpha = NULL;
			}

			// add texture to list
			TextureNumber = TPool_Add( EManager->TPool, NULL, BmpName, Alpha );
			if ( TextureNumber == -1 )
			{
				return GE_FALSE;
			}
			Cus->BitmapList[i] = TPool_Get( EManager->TPool, TextureNumber );
			if ( Cus->BitmapList[i] == NULL )
			{
				return GE_FALSE;
			}
		}
	}

	// if no world bitmap is provided, then do the flipbook this way
	if ( Cus->WorldBitmap == NULL )
	{

		// locals
		Sprite		Spr;

		// setup sprite struct			
		memset( &Spr, 0, sizeof( Spr ) );
		Spr.Texture = Cus->BitmapList;
		assert( Sq->Color.r >= 0.0f );
		Spr.Color.r = Sq->Color.r;
		assert( Sq->Color.g >= 0.0f );
		Spr.Color.g = Sq->Color.g;
		assert( Sq->Color.b >= 0.0f );
		Spr.Color.b = Sq->Color.b;
		Spr.Color.a = 255.0f;
		Spr.Scale = Sq->Scale;
		Spr.TotalTextures = Sq->TextureCount;
		switch ( Sq->CycleStyle )
		{
			case 1:
				Spr.Style = SPRITE_CYCLE_REVERSE;
				break;
			case 2:
				Spr.Style = SPRITE_CYCLE_ONCE;
				break;
			default:
				Spr.Style = SPRITE_CYCLE_RESET;
				break;
		}
		Spr.TextureRate = Sq->CycleSpeed;
		assert( Spr.TextureRate >= 0.0f );

		// setup base position on actor bone...
		if ( ( ( Sq->AStart != NULL ) && ( Sq->AStart->Actor != NULL ) ) || ( Sq->Actor != NULL ) )
		{

			// locals
			geXForm3d	Xf;
			geBoolean	Result;

			// save actor that its locked to
			if ( ( Sq->AStart != NULL ) && ( Sq->AStart->Actor != NULL ) )
			{
				Cus->Actor = Sq->AStart->Actor;
			}
			else
			{
				Cus->Actor = Sq->Actor;
			}

			// get bone location
			Result = geActor_GetBoneTransform( Cus->Actor, "EFFECT_BONE01", &Xf );
			if ( Result == GE_FALSE )
			{
				return GE_FALSE;
			}
			geVec3d_Copy( &( Xf.Translation ), &( Spr.Pos ) );
		}
		// ...or on supplied position
		else
		{
			geVec3d_Copy( &( Sq->Position ), &( Spr.Pos ) );
		}

		// add sprite
		if ( Effect_New( EManager->ESystem, Effect_Sprite, &Spr, 0, FlipbookM_DeathCallback, Root, &( Root->EffectList[0] ) ) == GE_FALSE )
		{
			return GE_FALSE;
		}
	}

	// all done
	return GE_TRUE;

} // FlipbookM_Create()



////////////////////////////////////////////////////////////////////////////////////////
//
//	FlipbookM_Destroy()
//
////////////////////////////////////////////////////////////////////////////////////////
static void FlipbookM_Destroy(
	EffectManager	*EManager,	// effect manager in which it exists
	EffectM_Root	*Root )		// root data
{

	// locals
	Custom		*Cus;

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );

	// get custom data
	Cus = Root->Custom;

	// zap custom data
	if ( Cus != NULL )
	{

		// zap bitmap list
		if ( Cus->BitmapList != NULL )
		{
			geRam_Free( Cus->BitmapList );
		}
	}

	// all done
	return;

	// get rid of warnings
	EManager;

} // FlipbookM_Destroy()



////////////////////////////////////////////////////////////////////////////////////////
//
//	FlipbookM_Update()
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean FlipbookM_Update(
	EffectManager	*EManager,	// effect manager to which it belongs
	EffectM_Root	*Root )		// root data
{

	// locals
	Custom		*Cus;
	Sprite		Spr;

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

	// if its not hooked to a world bitmap then update it this way...
	if ( Cus->WorldBitmap == NULL )
	{

		// adjust position if its locked to an actor
		if ( Cus->Actor != NULL )
		{

			// locals
			geXForm3d	Xf;
			geBoolean	Result;

			// get bone location
			Result = geActor_GetBoneTransform( Cus->Actor, "EFFECT_BONE01", &Xf );
			if ( Result == GE_FALSE )
			{
				return GE_FALSE;
			}
			geVec3d_Copy( &( Xf.Translation ), &( Spr.Pos ) );

			// modify effect
			Result = Effect_Modify(	EManager->ESystem,
									Effect_Sprite,
									&Spr,
									Root->EffectList[0],
									SPRITE_POS );
			assert( Result == GE_TRUE );
		}
	}
	// ...otherwise update it this way
	else
	{

		// locals
		geBoolean	Result;

		// adjust delay
		Cus->CurDelay -= Root->TimeDelta;
		if ( Cus->CurDelay <= 0.0f )
		{

			// adjust texture number
			Cus->CurDelay = Cus->CycleSpeed;
			Cus->CurTexture += Cus->CycleDir;

			// decide what to do when list end reached
			if ( ( Cus->CurTexture >= Cus->TextureCount ) || ( Cus->CurTexture < 0 ) )
			{
				switch ( Cus->CycleStyle )
				{
					case 2:
					{
						return GE_FALSE;
						break;
					}
					case 1:
					{
						Cus->CycleDir = -Cus->CycleDir;
						Cus->CurTexture += Cus->CycleDir;
						break;
					}
					default:
					{
						Cus->CurTexture = 0;
						break;
					}
				}
			}

			// draw new texture
			assert( Cus->BitmapList[Cus->CurTexture] != NULL );
			assert( Cus->WorldBitmap != NULL );
			Result = geBitmap_BlitBitmap( Cus->BitmapList[Cus->CurTexture], Cus->WorldBitmap );
		}
	}

	// all done
	return GE_TRUE;

} // FlipbookM_Update()
