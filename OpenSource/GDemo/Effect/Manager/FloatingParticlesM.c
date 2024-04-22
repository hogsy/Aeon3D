////////////////////////////////////////////////////////////////////////////////////////
//	FloatingParticlesM.c
//                                                                                      
//  Author: Peter Siamidis                                                              
//  Description:    
//		Floating particles that move in a spiral pattern.
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
	int			TextureNumber;
	geActor		*Actor;
	geBitmap	*Bmp;
	geXForm3d	*Xf;
	float		*Speed;
	int			ParticleCount;
	float		Radius;
	float		Height;
	float		XSlant;
	float		ZSlant;
	float		MinSpeed;
	float		MaxSpeed;
	geVec3d		BasePos;

} Custom;


////////////////////////////////////////////////////////////////////////////////////////
//	Interface setup
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean	FloatingParticlesM_Create( EffectManager *EManager, EffectM_Root *Root );
static void			FloatingParticlesM_Destroy( EffectManager *EManager, EffectM_Root *Root );
static geBoolean	FloatingParticlesM_Update( EffectManager *EManager, EffectM_Root *Root );
EffectM_Interface	FloatingParticlesM_Interface =
{
	FloatingParticlesM_Create,
	FloatingParticlesM_Destroy,
	FloatingParticlesM_Update
};



////////////////////////////////////////////////////////////////////////////////////////
//
//	FloatingParticlesM_Create()
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean FloatingParticlesM_Create(
	EffectManager	*EManager,	// effect manager to add it to
	EffectM_Root	*Root )		// root data
{

	// locals
	EM_FloatingParticles	*Fp;
	Sprite					Spr;
	int						i;
	Custom					*Cus;

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );

	// get data
	Fp = Root->UserData;
	assert( Fp != NULL );
	assert( Root->UserDataSize == sizeof( *Fp ) );
	
	// don't launch this effect if its trigger based
	if ( EffectC_IsStringNull( Fp->TriggerName ) == GE_FALSE )
	{
		return GE_FALSE;
	}

	// properly null stuff thats been messed up by the editor
	if ( EffectC_IsStringNull( Fp->AlphaName ) == GE_TRUE )
	{
		Fp->AlphaName = NULL;
	}

	// init custom data
	Root->Custom = geRam_AllocateClear( sizeof( Custom ) );
	if ( Root->Custom == NULL )
	{
		return GE_FALSE;
	}
	Cus = Root->Custom;
	Cus->ParticleCount = Fp->ParticleCount;
	Cus->Radius = Fp->Radius;
	Cus->Height = Fp->Height;
	Cus->XSlant = Fp->XSlant;
	Cus->ZSlant = Fp->ZSlant;
	Cus->MinSpeed = Fp->MinSpeed;
	Cus->MaxSpeed = Fp->MaxSpeed;

	// init remaining root data
	Root->EffectCount = Cus->ParticleCount;
	Root->EffectList = geRam_Allocate( sizeof( int ) * Root->EffectCount );
	if ( Root->EffectList == NULL )
	{
		return GE_FALSE;
	}
	for ( i = 0; i < Root->EffectCount; i++ )
	{
		Root->EffectList[i] = -1;
	}

	// init transform struct
	Cus->Xf = geRam_AllocateClear( Cus->ParticleCount * sizeof ( *( Cus->Xf ) ) );
	if ( Cus->Xf == NULL )
	{
		return GE_FALSE;
	}

	// init speed struct
	Cus->Speed = geRam_AllocateClear( Cus->ParticleCount * sizeof( *( Cus->Speed ) ) );
	if ( Cus->Speed == NULL )
	{
		return GE_FALSE;
	}

	// setup default sprite data
	memset( &Spr, 0, sizeof( Spr ) );
	Cus->TextureNumber = TPool_Add( EManager->TPool, NULL, Fp->BmpName, Fp->AlphaName );
	if ( Cus->TextureNumber == -1 )
	{
		return GE_FALSE;
	}
	Cus->Bmp = TPool_Get( EManager->TPool, Cus->TextureNumber );
	Spr.Texture = &( Cus->Bmp );
	if ( Spr.Texture == NULL )
	{
		return GE_FALSE;
	}
	assert( Fp->Color.r >= 0.0f );
	Spr.Color.r = Fp->Color.r;
	assert( Fp->Color.g >= 0.0f );
	Spr.Color.g = Fp->Color.g;
	assert( Fp->Color.b >= 0.0f );
	Spr.Color.b = Fp->Color.b;
	Spr.Color.a = 255.0f;
	Spr.TotalTextures = 1;
	Spr.Style = SPRITE_CYCLE_NONE;
	Spr.Scale = Fp->Scale;

	// setup base position on actor bone...
	if ( ( Fp->AStart != NULL ) && ( Fp->AStart->Actor != NULL ) )
	{

		// locals
		geXForm3d	Xf;
		geBoolean	Result;

		// save actor that its locked to
		Cus->Actor = Fp->AStart->Actor;

		// get bone location
		Result = geActor_GetBoneTransform( Cus->Actor, "EFFECT_BONE01", &Xf );
		if ( Result == GE_FALSE )
		{
			return GE_FALSE;
		}
		geVec3d_Copy( &( Xf.Translation ), &( Cus->BasePos ) );
	}
	// ...or on supplied position
	else
	{
		geVec3d_Copy( &( Fp->Position ), &( Cus->BasePos ) );
	}

	// add effects
	for ( i = 0; i < Root->EffectCount; i++ )
	{

		// set random direction
		geXForm3d_SetIdentity( &( Cus->Xf[i] ) );
		geXForm3d_RotateX( &( Cus->Xf[i] ), EffectC_Frand( -Cus->XSlant, Cus->XSlant ) );
		geXForm3d_RotateZ( &( Cus->Xf[i] ), EffectC_Frand( -Cus->ZSlant, Cus->ZSlant ) );

		// pick random start spot
		geVec3d_Copy( &( Cus->BasePos ), &( Cus->Xf[i].Translation ) );
		Cus->Xf[i].Translation.X += EffectC_Frand( -( Cus->Radius / 2.0f ), Cus->Radius / 2.0f );
		Cus->Xf[i].Translation.Z += EffectC_Frand( -( Cus->Radius / 2.0f ), Cus->Radius / 2.0f );
		geVec3d_Copy( &( Cus->Xf[i].Translation ), &( Spr.Pos ) );

		// set speed
		Cus->Speed[i] = EffectC_Frand( Cus->MinSpeed, Cus->MaxSpeed );

		// add effect
		if ( Effect_New( EManager->ESystem, Effect_Sprite, &Spr, 0, NULL, NULL, &( Root->EffectList[i] ) ) == GE_FALSE )
		{
			return GE_FALSE;
		}
	}

	// all done
	return GE_TRUE;

} // FloatingParticlesM_Create()



////////////////////////////////////////////////////////////////////////////////////////
//
//	FloatingParticlesM_Destroy()
//
////////////////////////////////////////////////////////////////////////////////////////
static void FloatingParticlesM_Destroy(
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

	// free internal custom stuff
	if ( Cus != NULL )
	{

		// free xform list
		if ( Cus->Xf != NULL )
		{
			geRam_Free( Cus->Xf );
		}
	}

	// all done
	return;

	// get rid of warnings
	EManager;
	Root;

} // FloatingParticlesM_Destroy()



////////////////////////////////////////////////////////////////////////////////////////
//
//	FloatingParticlesM_Update()
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean FloatingParticlesM_Update(
	EffectManager	*EManager,	// effect manager to which it belongs
	EffectM_Root	*Root )		// root data
{

	// locals
	Custom		*Cus;
	geVec3d		Offset;
	geVec3d		Movement;
	geVec3d		Pos;
	Sprite		Spr;
	int			i;
	geBoolean	Result;

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );

	// get custom data
	Cus = Root->Custom;
	assert( Cus != NULL );

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
		geVec3d_Subtract( &( Xf.Translation ), &( Cus->BasePos ), &Movement );
		geVec3d_Copy( &( Xf.Translation ), &( Cus->BasePos ) );
	}
	else
	{
		geVec3d_Set( &Movement, 0.0f, 0.0f, 0.0f );
	}

	// modify effects
	for ( i = 0; i < Root->EffectCount; i++ )
	{

		// adjust this particles orientation
		geVec3d_Copy( &( Cus->Xf[i].Translation ), &Pos );
		geVec3d_Set( &( Cus->Xf[i].Translation ), 0.0f, 0.0f, 0.0f );
		geXForm3d_RotateY( &( Cus->Xf[i] ), Root->TimeDelta * 5.0f );
		geVec3d_Copy( &Pos, &( Cus->Xf[i].Translation ) );

		// adjust particle position
		geXForm3d_GetUp( &( Cus->Xf[i] ), &Offset );
		geVec3d_AddScaled( &( Cus->Xf[i].Translation ), &Offset, Root->TimeDelta * Cus->Speed[i], &( Cus->Xf[i].Translation ) );
		geVec3d_Add( &( Cus->Xf[i].Translation ), &Movement, &( Cus->Xf[i].Translation ) );
		geVec3d_Copy( &( Cus->Xf[i].Translation ), &( Spr.Pos ) );

		// reset particle if it has hit its height limit
		if ( ( Cus->Xf[i].Translation.Y - Cus->BasePos.Y ) > Cus->Height )
		{

			// set random direction
			geXForm3d_SetIdentity( &( Cus->Xf[i] ) );
			geXForm3d_RotateX( &( Cus->Xf[i] ), EffectC_Frand( -Cus->XSlant, Cus->XSlant ) );
			geXForm3d_RotateZ( &( Cus->Xf[i] ), EffectC_Frand( -Cus->ZSlant, Cus->ZSlant ) );

			// pick random start spot
			geVec3d_Copy( &( Cus->BasePos ), &( Cus->Xf[i].Translation ) );
			Cus->Xf[i].Translation.X += EffectC_Frand( -( Cus->Radius / 2.0f ), Cus->Radius / 2.0f );
			Cus->Xf[i].Translation.Z += EffectC_Frand( -( Cus->Radius / 2.0f ), Cus->Radius / 2.0f );
			geVec3d_Copy( &( Cus->Xf[i].Translation ), &( Spr.Pos ) );

			// set speed
			Cus->Speed[i] = EffectC_Frand( Cus->MinSpeed, Cus->MaxSpeed );
		}

		// modify effect
		Result = Effect_Modify(	EManager->ESystem,
								Effect_Sprite,
								&Spr,
								Root->EffectList[i],
								SPRITE_POS );
		assert( Result == GE_TRUE );
	}

	// all done
	return GE_TRUE;

} // FloatingParticlesM_Update()
