////////////////////////////////////////////////////////////////////////////////////////
//	ProceduralM.c
//                                                                                      
//  Author: Peter Siamidis                                                              
//  Description:    
//		World procedural texture.
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
//	Interface setup
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean	ProceduralM_Create( EffectManager *EManager, EffectM_Root *Root );
static void			ProceduralM_Destroy( EffectManager *EManager, EffectM_Root *Root );
static geBoolean	ProceduralM_Update( EffectManager *EManager, EffectM_Root *Root );
EffectM_Interface	ProceduralM_Interface =
{
	ProceduralM_Create,
	ProceduralM_Destroy,
	ProceduralM_Update
};



////////////////////////////////////////////////////////////////////////////////////////
//
//	ProceduralM_Create()
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean ProceduralM_Create(
	EffectManager	*EManager,	// effect manager to add it to
	EffectM_Root	*Root )		// root data
{

	// locals
	EM_Procedural		*Proc;
	ProceduralTexture	Pt;

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );

	// get data
	Proc = Root->UserData;
	assert( Proc != NULL );
	assert( Root->UserDataSize == sizeof( *Proc ) );

	// don't launch this effect if its trigger based
	if ( EffectC_IsStringNull( Proc->TriggerName ) == GE_FALSE )
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

	// add procedural
	memset( &Pt, 0, sizeof( Pt ) );
	Pt.UseWorldPoly = GE_TRUE;
	assert( Proc->ProceduralName != NULL );
	Pt.Type = Proc->ProceduralName;
	Pt.Parms = Proc->Parameters;
	if ( ( Pt.Parms != NULL ) && ( strnicmp( Pt.Parms, "<null>", 6 ) == 0 ) )
	{
		Pt.Parms = NULL;
	}
	assert( Proc->TextureName != NULL );
	Pt.Texture = geWorld_GetBitmapByName( Effect_GetWorld( EManager->ESystem ), Proc->TextureName );
	if ( Effect_New( EManager->ESystem, Effect_ProceduralTexture, &Pt, 0, NULL, NULL, &( Root->EffectList[0] ) ) == GE_FALSE )
	{
		return GE_FALSE;
	}

	// all done
	return GE_TRUE;

} // ProceduralM_Create()



////////////////////////////////////////////////////////////////////////////////////////
//
//	ProceduralM_Destroy()
//
////////////////////////////////////////////////////////////////////////////////////////
static void ProceduralM_Destroy(
	EffectManager	*EManager,	// effect manager in which it exists
	EffectM_Root	*Root )		// root data
{

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );

	// get rid of warnings
	EManager;
	Root;

} // ProceduralM_Destroy()



////////////////////////////////////////////////////////////////////////////////////////
//
//	ProceduralM_Update()
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean ProceduralM_Update(
	EffectManager	*EManager,	// effect manager to add it to
	EffectM_Root	*Root )		// root data
{

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );

	// all done
	return GE_TRUE;

	// get rid of warnings
	EManager;
	Root;

} // ProceduralM_Update()
