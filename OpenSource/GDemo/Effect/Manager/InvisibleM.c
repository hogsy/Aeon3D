////////////////////////////////////////////////////////////////////////////////////////
//	InvisibleM.c
//                                                                                      
//  Author: Peter Siamidis                                                              
//  Description:    
//		A invisiblity effect.
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
//	-Actor:			REQUIRED
//	-User Data:		REQUIRED, all the settings are here
//	-Editor use:	SUPPORTED
//	-for effect manager use only
////////////////////////////////////////////////////////////////////////////////////////
#pragma warning ( disable : 4514 )
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
	geBoolean	UseFillLight;
	geVec3d		FillLightNormal;
	geFloat		FillLightRed;
	geFloat		FillLightGreen;
	geFloat		FillLightBlue;
	geFloat		AmbientLightRed;
	geFloat		AmbientLightGreen;
	geFloat		AmbientLightBlue;
	geBoolean	UseAmbientLightFromFloor;
	int			MaximumDynamicLightsToUse;
	char		*LightReferenceBoneName;
	geBoolean	PerBoneLighting;
	geBitmap	**OriginalBitmapList;
	float		TimeLeft;

} Custom;


////////////////////////////////////////////////////////////////////////////////////////
//	Interface setup
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean	InvisibleM_Create( EffectManager *EManager, EffectM_Root *Root );
static void			InvisibleM_Destroy( EffectManager *EManager, EffectM_Root *Root );
static geBoolean	InvisibleM_Update( EffectManager *EManager, EffectM_Root *Root );
EffectM_Interface	InvisibleM_Interface =
{
	InvisibleM_Create,
	InvisibleM_Destroy,
	InvisibleM_Update
};



////////////////////////////////////////////////////////////////////////////////////////
//
//	InvisibleM_Create()
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean InvisibleM_Create(
	EffectManager	*EManager,	// effect manager to add it to
	EffectM_Root	*Root )		// root data
{

	// locals
	EM_Invisible	*Inv;
	Custom			*Cus;
	geBoolean		Result;
	int				TextureNumber;
	geBitmap		*Bitmap;
	int32			Material;
	int32			MaterialCount;
	float			R, G, B;

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );

	// get data
	Inv = Root->UserData;
	assert( Inv != NULL );
	assert( Root->UserDataSize == sizeof( *Inv ) );

	// properly null stuff thats been messed up by the editor
	if ( EffectC_IsStringNull( Inv->AlphaName ) == GE_TRUE )
	{
		Inv->AlphaName = NULL;
	}

	// init remaining root data
	Root->EffectCount = 1;
	Root->EffectList = geRam_Allocate( sizeof( uint32 ) * Root->EffectCount );
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
	assert( Inv->AStart != NULL );
	assert( Inv->AStart->Actor != NULL );
	Cus->Actor = Inv->AStart->Actor;
	Cus->TimeLeft = Inv->TotalLife;

	// save original actor lighting settings
	Result = geActor_GetLightingOptions(	Cus->Actor,
											&( Cus->UseFillLight ),
											&( Cus->FillLightNormal ),
											&( Cus->FillLightRed ),
											&( Cus->FillLightGreen ),
											&( Cus->FillLightBlue ),
											&( Cus->AmbientLightRed ),
											&( Cus->AmbientLightGreen ),
											&( Cus->AmbientLightBlue ),
											&( Cus->UseAmbientLightFromFloor ),
											&( Cus->MaximumDynamicLightsToUse ),
											&( Cus->LightReferenceBoneName ),
											&( Cus->PerBoneLighting ) );
	assert( Result == GE_TRUE );

	// get invisible texture
	assert( Inv->BmpName != NULL );
	TextureNumber = TPool_Add( EManager->TPool, NULL, Inv->BmpName, Inv->AlphaName );
	if ( TextureNumber == -1 )
	{
		return -1;
	}
	Bitmap = TPool_Get( EManager->TPool, TextureNumber );
	if ( Bitmap == NULL )
	{
		return GE_FALSE;
	}

	// adjust all materials on this actor
	MaterialCount = geActor_GetMaterialCount( Cus->Actor );
	assert( MaterialCount > 0 );
	Cus->OriginalBitmapList = geRam_AllocateClear( sizeof( geBitmap * ) * MaterialCount );
	if ( Cus->OriginalBitmapList == NULL )
	{
		return GE_FALSE;
	}
	for ( Material = 0; Material < MaterialCount; Material++ )
	{

		// save old material
		Result = geActor_GetMaterial( Cus->Actor, Material, &( Cus->OriginalBitmapList[Material] ), &R, &G, &B );
		assert( Result == GE_TRUE );
		Result = geWorld_AddBitmap( Effect_GetWorld( EManager->ESystem ), Cus->OriginalBitmapList[Material] );
		assert( Result == GE_TRUE );

		// apply new material
		Result = geActor_SetMaterial( Cus->Actor, Material, Bitmap, 255.0f, 255.0f, 255.0f );
		assert( Result == GE_TRUE );
	}

	// all done
	return GE_TRUE;

} // InvisibleM_Create()



////////////////////////////////////////////////////////////////////////////////////////
//
//	InvisibleM_Destroy()
//
////////////////////////////////////////////////////////////////////////////////////////
static void InvisibleM_Destroy(
	EffectManager	*EManager,	// effect manager in which it exists
	EffectM_Root	*Root )		// root data
{

	// locals
	geActor_Def	*ActorDef;
	geBody		*Body;
	int32		Material;
	int32		MaterialCount;
	geBoolean	Result;
	geBitmap	*Bitmap;
	float		R, G, B;
	Custom		*Cus;
	char		*MaterialName;

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );

	// get custom data
	Cus = Root->Custom;
	assert( Cus != NULL );

	// get assorted actor info
	ActorDef = geActor_GetActorDef( Cus->Actor );
	assert( ActorDef != NULL );
	Body = geActor_GetBody( ActorDef );
	assert( Body != NULL );
	MaterialCount = geActor_GetMaterialCount( Cus->Actor );
	assert( MaterialCount > 0 );

	// reset all materials on this actor
	for ( Material = 0; Material < MaterialCount; Material++ )
	{
		Result = geBody_GetMaterial( Body, Material, &MaterialName, &Bitmap, &R, &G, &B );
		assert( Result == GE_TRUE );
		Result = geActor_SetMaterial( Cus->Actor, Material, Bitmap, R, G, B );
		assert( Result == GE_TRUE );
		if ( Cus->OriginalBitmapList != NULL )
		{
			if ( Cus->OriginalBitmapList[Material] != NULL )
			{
				Result = geWorld_RemoveBitmap( Effect_GetWorld( EManager->ESystem ), Cus->OriginalBitmapList[Material] );
				assert( Result == GE_TRUE );
			}
		}
	}

	// free bitmap list
	if ( Cus->OriginalBitmapList != NULL )
	{
		geRam_Free( Cus->OriginalBitmapList );
		Cus->OriginalBitmapList = NULL;
	}
	
	// reset lighting on this actor
	Result = geActor_SetLightingOptions(	Cus->Actor,
											Cus->UseFillLight,
											&( Cus->FillLightNormal ),
											Cus ->FillLightRed,
											Cus->FillLightGreen,
											Cus->FillLightBlue,
											Cus->AmbientLightRed,
											Cus->AmbientLightGreen,
											Cus->AmbientLightBlue,
											Cus->UseAmbientLightFromFloor,
											Cus->MaximumDynamicLightsToUse,
											Cus->LightReferenceBoneName,
											Cus->PerBoneLighting );
	assert( Result == GE_TRUE );

	// all done
	return;

} // InvisibleM_Destroy()



////////////////////////////////////////////////////////////////////////////////////////
//
//	InvisibleM_Update()
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean InvisibleM_Update(
	EffectManager	*EManager,	// effect manager to add it to
	EffectM_Root	*Root )		// root data
{

	// locals
	Custom		*Cus;

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );

	// get custom data
	Cus = Root->Custom;
	assert( Cus != NULL );

	// kill the effect if its time has run out
	if ( Cus->TimeLeft > 0.0f )
	{
		Cus->TimeLeft -= Root->TimeDelta;
		if ( Cus->TimeLeft <= 0.0f )
		{
			Cus->TimeLeft = 0.0f;
			return GE_FALSE;
		}
	}

	// all done
	return GE_TRUE;

} // InvisibleM_Update()
