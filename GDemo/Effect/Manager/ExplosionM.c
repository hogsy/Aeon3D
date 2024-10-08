////////////////////////////////////////////////////////////////////////////////////////
//  ExplosionM.c									                                        
//                                                                                      
//  Author: Peter Siamidis                                                              
//  Description:    
//			Explosion effect.
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
//	Explosion effect.
//	-Actor:			REQUIRED, has explosion location and direction
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
	geFloat		TimeLeft;

} Custom;


////////////////////////////////////////////////////////////////////////////////////////
//	Interface setup
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean	ExplosionM_Create( EffectManager *EManager, EffectM_Root *Root );
static void			ExplosionM_Destroy( EffectManager *EManager, EffectM_Root *Root );
static geBoolean	ExplosionM_Update( EffectManager *EManager, EffectM_Root *Root );
EffectM_Interface	ExplosionM_Interface =
{
	ExplosionM_Create,
	ExplosionM_Destroy,
	ExplosionM_Update
};



////////////////////////////////////////////////////////////////////////////////////////
//
//	ExplosionM_Create()
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean ExplosionM_Create(
	EffectManager	*EManager,	// effect manager to add it to
	EffectM_Root	*Root )		// root data
{

	// locals
	geXForm3d		Xf;
	EM_Explosion	*Exp;
	Custom			*Cus;
	geBoolean		Result;
	Spray			Sp;
	geVec3d			In;

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );

	// get data
	Exp = Root->UserData;
	assert( Exp != NULL );
	assert( Root->UserDataSize == sizeof( *Exp ) );

	// don't launch this effect if its trigger based
	if ( EffectC_IsStringNull( Exp->TriggerName ) == GE_FALSE )
	{
		return GE_FALSE;
	}

	// properly null stuff thats been messed up by the editor
	if ( EffectC_IsStringNull( Exp->AlphaName ) == GE_TRUE )
	{
		Exp->AlphaName = NULL;
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
	Cus->TimeLeft = 3.0f;
	Cus->Actor = Exp->AStart->Actor;

	// setup location
	Result = geActor_GetBoneTransform( Cus->Actor, "EFFECT_BONE01", &Xf );
	if ( Result == GE_FALSE )
	{
		return GE_FALSE;
	}
	geVec3d_Copy( &( Xf.Translation ), &( Sp.Source ) );
	geXForm3d_GetIn( &Xf, &In );
	geVec3d_Add( &( Sp.Source ), &In, &( Sp.Dest ) );

	// show explosion effect
	memset( &Sp, 0, sizeof( Sp ) );
	Sp.ColorMin.r = 237.0f;
	Sp.ColorMax.r = 237.0f;
	Sp.ColorMin.g = 40.0f;
	Sp.ColorMax.g = 230.0f;
	Sp.ColorMin.b = 37.0f;
	Sp.ColorMax.b = 37.0f;
	Sp.ColorMin.a = 200.0f;
	Sp.ColorMax.a = 255.0f;
	Sp.Rate = 0.005f;
	Sp.SourceVariance = 2;
	Sp.DestVariance = 2;
	Sp.MinScale = 0.15f;
	Sp.MaxScale = 0.25f;
	Sp.MinUnitLife = 2.5f;
	Sp.MaxUnitLife = 3.4f;
	Sp.SprayLife = 0.35f;
	Sp.MinSpeed = 90.0f;
	Sp.MaxSpeed = 110.0f;
	geVec3d_Set( &( Sp.Gravity ), 0.0f, 0.0f, -45.0f );
	Exp->TextureNumber = TPool_Add( EManager->TPool, NULL, Exp->BmpName, Exp->AlphaName );
	if ( Exp->TextureNumber == -1 )
	{
		return -1;
	}
	Sp.Texture = TPool_Get( EManager->TPool, Exp->TextureNumber );
	if ( Sp.Texture == NULL )
	{
		return GE_FALSE;
	}
	if ( Effect_New( EManager->ESystem, Effect_Spray, &Sp, 0, NULL, NULL, NULL ) == GE_FALSE )
	{
		return GE_FALSE;
	}

	// all done
	return GE_TRUE;

} // ExplosionM_Create()



////////////////////////////////////////////////////////////////////////////////////////
//
//	ExplosionM_Destroy()
//
////////////////////////////////////////////////////////////////////////////////////////
static void ExplosionM_Destroy(
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
	assert( Cus != NULL );

	// get rid of warnings
	EManager;

} // ExplosionM_Destroy()



////////////////////////////////////////////////////////////////////////////////////////
//
//	ExplosionM_Update()
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean ExplosionM_Update(
	EffectManager	*EManager,	// effect manager to add it to
	EffectM_Root	*Root )		// root data
{

	// locals
	Custom			*Cus;
	geXForm3d		Xf;
	geBoolean		Result;
	Spray			Sp;
	geVec3d			In;

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

	// setup location
	Result = geActor_GetBoneTransform( Cus->Actor, "EFFECT_BONE01", &Xf );
	if ( Result == GE_FALSE )
	{
		return GE_FALSE;
	}
	geVec3d_Copy( &( Xf.Translation ), &( Sp.Source ) );
	geXForm3d_GetIn( &Xf, &In );
	geVec3d_Add( &( Sp.Source ), &In, &( Sp.Dest ) );

	// adjust position
	Result = Effect_Modify(	EManager->ESystem,
							Effect_Spray,
							&Sp,
							Root->EffectList[0],
							SPRAY_SOURCE | SPRAY_DEST );
	assert( Result == GE_TRUE );

	// all done
	return GE_TRUE;

} // ExplosionM_Update()
