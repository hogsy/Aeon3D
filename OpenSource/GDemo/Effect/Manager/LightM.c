////////////////////////////////////////////////////////////////////////////////////////
//	LightM.c
//                                                                                      
//  Author: Peter Siamidis                                                              
//  Description:    
//		A dymamic light
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
//	-Actor:			OPTIONAL, if present a light gets locked to each of its effect bones
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
	char		*LString;
	float		LTime;
	float		LLoopStyle;
	int			CurLStringChar;
	int			LStringDir;
	float		LStringDelay, LStringCurDelay;
	int			LStringLength;
	float		*LPercent;

} Custom;


////////////////////////////////////////////////////////////////////////////////////////
//	Interface setup
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean	LightM_Create( EffectManager *EManager, EffectM_Root *Root );
static void			LightM_Destroy( EffectManager *EManager, EffectM_Root *Root );
static geBoolean	LightM_Update( EffectManager *EManager, EffectM_Root *Root );
EffectM_Interface	LightM_Interface =
{
	LightM_Create,
	LightM_Destroy,
	LightM_Update
};



////////////////////////////////////////////////////////////////////////////////////////
//
//	LightM_Create()
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean LightM_Create(
	EffectManager	*EManager,	// effect manager to add it to
	EffectM_Root	*Root )		// root data
{

	// locals
	EM_Light	*Lg;
	Glow		Gl;
	Custom		*Cus;
	int			i;

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );

	// get user data
	Lg = Root->UserData;
	assert( Lg != NULL );
	assert( Root->UserDataSize == sizeof( *Lg ) );

	// properly null stuff thats been messed up by the editor
	if ( EffectC_IsStringNull( Lg->LString ) == GE_TRUE )
	{
		Lg->LString = NULL;
	}

	// init custom data
	Root->Custom = geRam_AllocateClear( sizeof( Custom ) );
	if ( Root->Custom == NULL )
	{
		return GE_FALSE;
	}
	Cus = Root->Custom;
	Cus->CurLStringChar = 0;
	Cus->LStringDir = 1;
	Cus->LTime = Lg->LTime;
	Cus->LLoopStyle = Lg->LLoopStyle;

	// setup default light parameters
	memset( &Gl, 0, sizeof( Gl ) );
	Gl.RadiusMin = Lg->MinRadius;
	Gl.RadiusMax = Lg->MaxRadius;
	Gl.ColorMin.r = Lg->ColorMin.r;
	Gl.ColorMax.r = Lg->ColorMax.r;
	Gl.ColorMin.g = Lg->ColorMin.g;
	Gl.ColorMax.g = Lg->ColorMax.g;
	Gl.ColorMin.b = Lg->ColorMin.b;
	Gl.ColorMax.b = Lg->ColorMax.b;
	Gl.ColorMin.a = 255.0f;
	Gl.ColorMax.a = 255.0f;
	Gl.Intensity = 1.0f;

	// save light string
	if ( Lg->LString != NULL )
	{

		// locals
		int	i;

		// fail if we have bad data
		if ( Lg->LTime <= 0.0f )
		{
			return GE_FALSE;
		}

		// save string
		Cus->LStringLength = strlen( Lg->LString );
		if ( Cus->LStringLength < 2 )
		{
			return GE_FALSE;
		}
		Cus->LString = geRam_Allocate( Cus->LStringLength + 1 );
		if ( Cus->LString == NULL )
		{
			return GE_FALSE;
		}
		strcpy( Cus->LString, Lg->LString );

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

			// fail if we have a bad character
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

	// setup source position based on actor bone...
	if ( ( ( Lg->AStart != NULL ) && ( Lg->AStart->Actor != NULL ) ) )
	{

		// locals
		char		BoneName[256];
		geXForm3d	Xf;

		// save actor that its locked to
		Cus->Actor = Lg->AStart->Actor;

		// determine how many lights will need to be added
		Root->EffectCount = 0;
		while ( 1 )
		{
			if ( Root->EffectCount <= 9 )
			{
				sprintf( BoneName, "EFFECT_BONE0%d", Root->EffectCount + 1 );
			}
			else
			{
				sprintf( BoneName, "EFFECT_BONE%d", Root->EffectCount + 1 );
			}
			if ( geActor_GetBoneTransform( Cus->Actor, BoneName, &Xf ) == GE_FALSE )
			{
				break;
			}
			Root->EffectCount++;
		}
		if ( Root->EffectCount == 0 )
		{
			return GE_FALSE;
		}

		// init remaining root data
		Root->EffectList = geRam_Allocate( sizeof( uint32 ) * Root->EffectCount );
		if ( Root->EffectList == NULL )
		{
			return GE_FALSE;
		}
		for ( i = 0; i < Root->EffectCount; i++ )
		{
			Root->EffectList[i] = -1;
		}

		// add effects
		for ( i = 0; i < Root->EffectCount; i++ )
		{
			if ( Root->EffectCount <= 9 )
			{
				sprintf( BoneName, "EFFECT_BONE0%d", i + 1 );
			}
			else
			{
				sprintf( BoneName, "EFFECT_BONE%d", i + 1 );
			}
			if ( geActor_GetBoneTransform( Cus->Actor, BoneName, &Xf ) == GE_FALSE )
			{
				return GE_FALSE;
			}
			geVec3d_Copy( &( Xf.Translation ), &( Gl.Pos ) );
			if ( Effect_New( EManager->ESystem, Effect_Glow, &Gl, 0, NULL, NULL, &( Root->EffectList[i] ) ) == GE_FALSE )
			{
				return GE_FALSE;
			}
		}
	}
	// ...or without actor bone
	else
	{

		// init remaining root data
		Root->EffectCount = 1;
		Root->EffectList = geRam_Allocate( sizeof( uint32 ) * Root->EffectCount );
		if ( Root->EffectList == NULL )
		{
			return GE_FALSE;
		}
		Root->EffectList[0] = -1;

		// add effect
		geVec3d_Copy( &( Lg->Position ), &( Gl.Pos ) );
		if ( Effect_New( EManager->ESystem, Effect_Glow, &Gl, 0, NULL, NULL, &( Root->EffectList[0] ) ) == GE_FALSE )
		{
			return GE_FALSE;
		}
	}

	// all done
	return GE_TRUE;

} // LightM_Create()



////////////////////////////////////////////////////////////////////////////////////////
//
//	LightM_Destroy()
//
////////////////////////////////////////////////////////////////////////////////////////
static void LightM_Destroy(
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
	}

	// all done
	return;

} // LightM_Destroy()



////////////////////////////////////////////////////////////////////////////////////////
//
//	LightM_Update()
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean LightM_Update(
	EffectManager	*EManager,	// effect manager to add it to
	EffectM_Root	*Root )		// root data
{

	// locals
	Custom	*Cus;
	Glow	Gl;
	uint32	Flags;

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );

	// get custom data
	Cus = Root->Custom;
	assert( Cus != NULL );

	// reset flags
	Flags = 0;

	// determine new ambient light value
	if ( Cus->LStringDelay > 0.0f )
	{
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
			Gl.Intensity = Cus->LPercent[Cus->CurLStringChar];
			Flags |= GLOW_INTENSITY;
		}
	}

	// setup pos flag if required
	if ( Cus->Actor != NULL )
	{
		Flags |= GLOW_POS;
	}

	// make adjustments
	if ( Flags != 0 )
	{

		// locals
		geXForm3d	Xf;
		char		BoneName[256];
		int			i;

		// adjust lights
		for ( i = 0; i < Root->EffectCount; i++ )
		{

			// setup new position
			if ( Flags & GLOW_POS )
			{

				// get current position
				if ( Root->EffectCount <= 9 )
				{
					sprintf( BoneName, "EFFECT_BONE0%d", i + 1 );
				}
				else
				{
					sprintf( BoneName, "EFFECT_BONE%d", i + 1 );
				}
				if ( geActor_GetBoneTransform( Cus->Actor, BoneName, &Xf ) == GE_FALSE )
				{
					return GE_FALSE;
				}
				geVec3d_Copy( &( Xf.Translation ), &( Gl.Pos ) );
			}

			// adjust light
			Effect_Modify(	EManager->ESystem,
							Effect_Glow,
							&Gl,
							Root->EffectList[i],
							Flags );
		}
	}

	// all done
	return GE_TRUE;

} // LightM_Update()
