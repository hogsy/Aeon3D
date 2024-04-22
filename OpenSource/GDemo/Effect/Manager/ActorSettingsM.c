////////////////////////////////////////////////////////////////////////////////////////
//  ActorSettingsM.c									                                        
//                                                                                      
//  Author: Peter Siamidis                                                              
//  Description:    
//			Adjust an actors settings.
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
//	Adjust an actors settings.
//	-Actor:			REQUIRED
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
	float		TimeLeft;
	char		*LString;
	float		LTime;
	float		LLoopStyle;
	float		NewFillR, NewFillG, NewFillB;
	float		NewAmbientR, NewAmbientG, NewAmbientB;
	geBoolean	NewPerBoneLighting;
	geVec3d		NewFillLightNormal;
	int			CurLStringChar;
	int			LStringDir;
	float		LStringDelay, LStringCurDelay;
	int			LStringLength;
	float		*LPercent;
	geBoolean	FillNormalActorRelative;

} Custom;


////////////////////////////////////////////////////////////////////////////////////////
//	Interface setup
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean	ActorSettingsM_Create( EffectManager *EManager, EffectM_Root *Root );
static void			ActorSettingsM_Destroy( EffectManager *EManager, EffectM_Root *Root );
static geBoolean	ActorSettingsM_Update( EffectManager *EManager, EffectM_Root *Root );
EffectM_Interface	ActorSettingsM_Interface =
{
	ActorSettingsM_Create,
	ActorSettingsM_Destroy,
	ActorSettingsM_Update
};



////////////////////////////////////////////////////////////////////////////////////////
//
//	ActorSettingsM_Create()
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean ActorSettingsM_Create(
	EffectManager	*EManager,	// effect manager to add it to
	EffectM_Root	*Root )		// root data
{

	// locals
	EM_ActorSettings	*As;
	Custom				*Cus;
	geBoolean			Result;

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );

	// get data
	As = Root->UserData;
	assert( As != NULL );
	assert( Root->UserDataSize == sizeof( *As ) );

	// properly null stuff thats been messed up by the editor
	if ( EffectC_IsStringNull( As->LString ) == GE_TRUE )
	{
		As->LString = NULL;
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
	Root->Custom = geRam_AllocateClear( sizeof( Custom ) );
	if ( Root->Custom == NULL )
	{
		return GE_FALSE;
	}
	Cus = Root->Custom;
	assert( As->AStart != NULL );
	assert( As->AStart->Actor != NULL );
	Cus->Actor = As->AStart->Actor;
	Cus->TimeLeft = As->TotalLife;
	Cus->LTime = As->LTime;
	Cus->LLoopStyle = As->LLoopStyle;
	Cus->NewAmbientR = As->AmbientLight.r;
	Cus->NewAmbientG = As->AmbientLight.g;
	Cus->NewAmbientB = As->AmbientLight.b;
	Cus->NewFillR = As->FillLight.r;
	Cus->NewFillG = As->FillLight.g;
	Cus->NewFillB = As->FillLight.b;
	Cus->NewPerBoneLighting = As->PerBoneLighting;
	geVec3d_Normalize( &( As->FillLightNormal ) );
	geVec3d_Copy( &( As->FillLightNormal ), &( Cus->NewFillLightNormal ) );
	Cus->CurLStringChar = 0;
	Cus->LStringDir = 1;
	Cus->FillNormalActorRelative = As->FillNormalActorRelative;

	// save light string
	if ( As->LString != NULL )
	{

		// locals
		int	i;

		// fail if we have bad data
		if ( As->LTime <= 0.0f )
		{
			return GE_FALSE;
		}

		// save string
		Cus->LStringLength = strlen( As->LString );
		if ( Cus->LStringLength < 2 )
		{
			return GE_FALSE;
		}
		Cus->LString = geRam_Allocate( Cus->LStringLength + 1 );
		if ( Cus->LString == NULL )
		{
			return GE_FALSE;
		}
		strcpy( Cus->LString, As->LString );

		// allocate percentage array
		Cus->LPercent = geRam_Allocate( sizeof( float ) * Cus->LStringLength );
		if ( Cus->LPercent == NULL )
		{
			return GE_FALSE;
		}

		// lowercase the string
		for ( i = 0; i < Cus->LStringLength; i++ )
		{

			// lowercase it
			if ( ( Cus->LString[i] >= 65 ) && ( Cus->LString[i] <= 90 ) )
			{
				Cus->LString[i] += 32;
			}

			// fail if we havea bad character
			if ( ( Cus->LString[i] < 97 ) || ( Cus->LString[i] > 122 ) )
			{
				return GE_FALSE;
			}

			// add value to percentage array
			Cus->LPercent[i] = (float)( Cus->LString[i] - 97 ) / 26.0f;
		}

		// determine delay to next char
		if ( Cus->LTime > 0.0f )
		{
			Cus->LStringDelay = Cus->LStringCurDelay = Cus->LTime / (float)Cus->LStringLength;
		}
	}

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

	// apply new actor settings
	Result = geActor_SetLightingOptions(	Cus->Actor,						// actor to adjust
											GE_TRUE,						// use fill light
											&( As->FillLightNormal ),		// light normal
											As->FillLight.r,				// fill RGB
											As->FillLight.g,
											As->FillLight.b,
											As->AmbientLight.r,				// ambient RGB
											As->AmbientLight.g,
											As->AmbientLight.b,
											GE_FALSE,						// light from floor
											Cus->MaximumDynamicLightsToUse,	// max lights to use
											Cus->LightReferenceBoneName,	// LightReferenceBoneName
											As->PerBoneLighting );			// per bone lighting
	assert( Result == GE_TRUE );

	// all done
	return GE_TRUE;

} // ActorSettingsM_Create()



////////////////////////////////////////////////////////////////////////////////////////
//
//	ActorSettingsM_Destroy()
//
////////////////////////////////////////////////////////////////////////////////////////
static void ActorSettingsM_Destroy(
	EffectManager	*EManager,	// effect manager in which it exists
	EffectM_Root	*Root )		// root data
{

	// locals
	geBoolean	Result;
	Custom		*Cus;

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );

	// free any custom data
	Cus = Root->Custom;
	if ( Cus != NULL )
	{

		// free light string
		if ( Cus->LString != NULL )
		{
			geRam_Free( Cus->LString );
		}

		// free percentage array
		if ( Cus->LPercent != NULL )
		{
			geRam_Free( Cus->LPercent );
		}

		// reset actor settings
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
	}

	// all done
	return;

} // ActorSettingsM_Destroy()



////////////////////////////////////////////////////////////////////////////////////////
//
//	ActorSettingsM_Update()
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean ActorSettingsM_Update(
	EffectManager	*EManager,	// effect manager to add it to
	EffectM_Root	*Root )		// root data
{

	// locals
	geBoolean	Result;
	Custom		*Cus;
	float		AmbientR, AmbientG, AmbientB;
	geVec3d		NewFillNormal;

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

	// setup ambient light based on light string...
	if ( Cus->LString != NULL )
	{

		// adjust string char change delay time
		Cus->LStringCurDelay -= Root->TimeDelta;
		if ( Cus->LStringCurDelay <= 0.0f )
		{
			Cus->LStringCurDelay = Cus->LStringDelay;
			Cus->CurLStringChar += Cus->LStringDir;
			if ( Cus->CurLStringChar < 0 )
			{
				if ( Cus->LLoopStyle == 0 )
				{
					Cus->CurLStringChar = Cus->LStringLength - 1;
				}
				else
				{
					Cus->CurLStringChar = 1;
					Cus->LStringDir = -Cus->LStringDir;
				}
			}
			else if ( Cus->CurLStringChar >= Cus->LStringLength )
			{
				if ( Cus->LLoopStyle == 0 )
				{
					Cus->CurLStringChar = 0;
				}
				else
				{
					Cus->CurLStringChar = Cus->LStringLength - 2;
					Cus->LStringDir = -Cus->LStringDir;
				}
			}
		}

		// setup ambient light value
		AmbientR = Cus->NewAmbientR * Cus->LPercent[Cus->CurLStringChar];
		AmbientG = Cus->NewAmbientG * Cus->LPercent[Cus->CurLStringChar];
		AmbientB = Cus->NewAmbientB * Cus->LPercent[Cus->CurLStringChar];
	}
	// ...or without light string
	else
	{
		AmbientR = Cus->NewAmbientR;
		AmbientG = Cus->NewAmbientG;
		AmbientB = Cus->NewAmbientB;
	}

	// setup fill light normal
	if ( Cus->FillNormalActorRelative == GE_FALSE )
	{
		geVec3d_Copy( &( Cus->NewFillLightNormal ), &NewFillNormal );
	}
	else
	{

		// locals
		geXForm3d	Xf;
		geXForm3d	XfT;

		// get actors current xf
		Result = geActor_GetBoneTransform( Cus->Actor, NULL, &Xf );
		if ( Result == GE_FALSE )
		{
			return GE_FALSE;
		}

		// build new normal
		geXForm3d_GetTranspose( &Xf, &XfT );
		geXForm3d_Rotate( &XfT, &( Cus->NewFillLightNormal ), &NewFillNormal );
	}

	// apply new actor settings
	Result = geActor_SetLightingOptions(	Cus->Actor,						// actor to adjust
											Cus->UseFillLight,				// use fill light
											&NewFillNormal,					// light normal
											Cus->NewFillR,					// fill RGB
											Cus->NewFillG,
											Cus->NewFillB,
											AmbientR,						// ambient RGB
											AmbientG,
											AmbientB,
											GE_FALSE,						// light from floor
											Cus->MaximumDynamicLightsToUse,	// max lights to use
											Cus->LightReferenceBoneName,	// LightReferenceBoneName
											Cus->NewPerBoneLighting );		// per bone lighting
	assert( Result == GE_TRUE );

	// all done
	return GE_TRUE;

} // ActorSettingsM_Update()
