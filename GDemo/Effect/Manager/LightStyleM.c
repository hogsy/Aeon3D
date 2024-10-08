////////////////////////////////////////////////////////////////////////////////////////
//	LightStyleM.c
//                                                                                      
//  Author: Peter Siamidis                                                              
//  Description:    
//		Effect to adjust light styles on the fly.
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
//	-Actor:			NOT SUPPORTED
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
	float		TotalTime;
	float		TimeRemaining;
	char		S1;
	char		S2;
	int			StyleToUse;
	geBoolean	Loop;
	float		CycleDir;

} Custom;


////////////////////////////////////////////////////////////////////////////////////////
//	Interface setup
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean	LightStyleM_Create( EffectManager *EManager, EffectM_Root *Root );
static void			LightStyleM_Destroy( EffectManager *EManager, EffectM_Root *Root );
static geBoolean	LightStyleM_Update( EffectManager *EManager, EffectM_Root *Root );
EffectM_Interface	LightStyleM_Interface =
{
	LightStyleM_Create,
	LightStyleM_Destroy,
	LightStyleM_Update
};



////////////////////////////////////////////////////////////////////////////////////////
//
//	LightStyleM_Create()
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean LightStyleM_Create(
	EffectManager	*EManager,	// effect manager to add it to
	EffectM_Root	*Root )		// root data
{

	// locals
	EM_LightStyle	*Ls;
	Custom			*Cus;
	geBoolean		Result;
	char			Style[2];

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );

	// get user data
	Ls = Root->UserData;
	assert( Ls != NULL );
	assert( Root->UserDataSize == sizeof( *Ls ) );

	// don't launch this effect if its trigger based
	if ( EffectC_IsStringNull( Ls->TriggerName ) == GE_FALSE )
	{
		return GE_FALSE;
	}

	// init custom data
	Root->Custom = geRam_AllocateClear( sizeof( Custom ) );
	if ( Root->Custom == NULL )
	{
		return GE_FALSE;
	}
	Cus = Root->Custom;
	Cus->TotalTime = Cus->TimeRemaining = Ls->MorphTime;
	Cus->StyleToUse = Ls->StyleToUse;
	Cus->Loop = Ls->Loop;
	Cus->CycleDir = 1.0f;
	
	// save style chars
	Cus->S1 = *( Ls->S1 );
	Cus->S2 = *( Ls->S2 );
	if ( ( Cus->S1 >= 65 ) && ( Cus->S1 <= 90 ) )
	{
		Cus->S1 += 32;
	}
	if ( ( Cus->S2 >= 65 ) && ( Cus->S2 <= 90 ) )
	{
		Cus->S2 += 32;
	}
	Style[0] = Cus->S1;
	Style[1] = '\0';

	// init remaining root data
	Root->EffectCount = 1;
	Root->EffectList = geRam_Allocate( sizeof( uint32 ) * Root->EffectCount );
	if ( Root->EffectList == NULL )
	{
		return GE_FALSE;
	}
	Root->EffectList[0] = -1;

	// setup default light
	Result = geWorld_SetLTypeTable( Effect_GetWorld( EManager->ESystem ), Ls->StyleToUse, Style );
	if ( Result == GE_FALSE )
	{
		return GE_FALSE;
	}

	// all done
	return GE_TRUE;

} // LightStyleM_Create()



////////////////////////////////////////////////////////////////////////////////////////
//
//	LightStyleM_Destroy()
//
////////////////////////////////////////////////////////////////////////////////////////
static void LightStyleM_Destroy(
	EffectManager	*EManager,	// effect manager in which it exists
	EffectM_Root	*Root )		// root data
{

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );

	// all done
	return;

	// get rid of warnings
	EManager;
	Root;

} // LightStyleM_Destroy()



////////////////////////////////////////////////////////////////////////////////////////
//
//	LightStyleM_Update()
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean LightStyleM_Update(
	EffectManager	*EManager,	// effect manager to add it to
	EffectM_Root	*Root )		// root data
{

	// locals
	Custom		*Cus;
	char		Style[2];
	geBoolean	Result;

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );

	// get custom data
	Cus = Root->Custom;
	assert( Cus != NULL );

	// adjust time remaining amount
	Cus->TimeRemaining -= ( Root->TimeDelta * Cus->CycleDir );
	if ( Cus->TimeRemaining < 0.0f )
	{
		Cus->TimeRemaining = 0.0f;
		Cus->CycleDir = -Cus->CycleDir;
	}
	else if ( Cus->TimeRemaining > Cus->TotalTime )
	{
		Cus->TimeRemaining = Cus->TotalTime;
		Cus->CycleDir = -Cus->CycleDir;
	}

	// pick new style
	if ( Cus->S2 == Cus->S1 )
	{
		geWorld_SetLTypeTable( Effect_GetWorld( EManager->ESystem ), Cus->StyleToUse, &( Cus->S1 ) );
		return GE_FALSE;
	}
	else
	{
		Style[0] = Cus->S2 + (char)( (float)( Cus->S1 - Cus->S2 ) * ( Cus->TimeRemaining / Cus->TotalTime ) );
	}

	// set new light style	
	Style[1] = '\0';
	Result = geWorld_SetLTypeTable( Effect_GetWorld( EManager->ESystem ), Cus->StyleToUse, Style );
	assert( Result == GE_TRUE );

	// if no time is remaining, then kill effect
	if ( ( Cus->Loop == GE_FALSE ) && ( Cus->TimeRemaining == 0.0f ) )
	{
		return GE_FALSE;
	}

	// all done
	return GE_TRUE;

} // LightStyleM_Update()
