////////////////////////////////////////////////////////////////////////////////////////
//	ScrollM.c
//                                                                                      
//  Author: Peter Siamidis                                                              
//  Description:    
//		Scroll a texture.
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
#include <assert.h>
#include "Ram.h"
#include "EffectMI.h"
#include "EffectC.h"


////////////////////////////////////////////////////////////////////////////////////////
//	Custom data
////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	geBitmap	*OutputBmp;
	geBitmap	*CopyBmp;
	float		X, Y;
	float		XOffset, YOffset;

} Custom;


////////////////////////////////////////////////////////////////////////////////////////
//	Interface setup
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean	ScrollM_Create( EffectManager *EManager, EffectM_Root *Root );
static void			ScrollM_Destroy( EffectManager *EManager, EffectM_Root *Root );
static geBoolean	ScrollM_Update( EffectManager *EManager, EffectM_Root *Root );
EffectM_Interface	ScrollM_Interface =
{
	ScrollM_Create,
	ScrollM_Destroy,
	ScrollM_Update
};



////////////////////////////////////////////////////////////////////////////////////////
//
//	ScrollM_Create()
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean ScrollM_Create(
	EffectManager	*EManager,	// effect manager to add it to
	EffectM_Root	*Root )		// root data
{

	// locals
	EM_Scroll		*Scr;
	Custom			*Cus;
	geBitmap_Info	BmpInfo;

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );

	// get data
	Scr = Root->UserData;
	assert( Scr != NULL );
	assert( Root->UserDataSize == sizeof( *Scr ) );

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
	Cus->XOffset = (float)Scr->XScroll;
	Cus->YOffset = (float)Scr->YScroll;

	// get output bitmap info
	Cus->OutputBmp = geWorld_GetBitmapByName( Effect_GetWorld( EManager->ESystem ), Scr->TextureName );
	if ( Cus->OutputBmp == NULL )
	{
		return GE_FALSE;
	}
	if ( geBitmap_GetInfo( Cus->OutputBmp, &BmpInfo, NULL ) == GE_FALSE )
	{
		return GE_FALSE;
	}
	geBitmap_ClearMips( Cus->OutputBmp );

	// create copy bitmap
	Cus->CopyBmp = geBitmap_Create( BmpInfo.Width, BmpInfo.Height, 1, BmpInfo.Format );
	if ( Cus->CopyBmp == NULL )
	{
		return GE_FALSE;
	}
	geBitmap_ClearMips( Cus->CopyBmp );
	if ( geBitmap_BlitBitmap( Cus->OutputBmp, Cus->CopyBmp ) == GE_FALSE )
	{
		return GE_FALSE;
	}

	// all done
	return GE_TRUE;

} // ScrollM_Create()



////////////////////////////////////////////////////////////////////////////////////////
//
//	ScrollM_Destroy()
//
////////////////////////////////////////////////////////////////////////////////////////
static void ScrollM_Destroy(
	EffectManager	*EManager,	// effect manager in which it exists
	EffectM_Root	*Root )		// root data
{

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );

	// get rid of warnings
	EManager;
	Root;

} // ScrollM_Destroy()



////////////////////////////////////////////////////////////////////////////////////////
//
//	ScrollM_Update()
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean ScrollM_Update(
	EffectManager	*EManager,	// effect manager to add it to
	EffectM_Root	*Root )		// root data
{

	// locals
	Custom			*Cus;
	geBitmap_Info	BmpInfo;
	geBoolean		Result;
	int				X, Y;

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );

	// get custom data
	Cus = Root->Custom;
	assert( Cus != NULL );

	// get bitmap info
	if ( geBitmap_GetInfo( Cus->OutputBmp, &BmpInfo, NULL ) == GE_FALSE )
	{
		return GE_FALSE;
	}

	// adjust offsets
	Cus->X += ( Root->TimeDelta * Cus->XOffset );
	if ( Cus->X >= BmpInfo.Width )
	{
		Cus->X = 0.0f;
	}
	else if ( Cus->X < 0 )
	{
		Cus->X = (float)( BmpInfo.Width - 1 );
	}
	Cus->Y += ( Root->TimeDelta * Cus->YOffset );
	if ( Cus->Y >= BmpInfo.Height )
	{
		Cus->Y = 0.0f;
	}
	else if ( Cus->Y < 0 )
	{
		Cus->Y = (float)( BmpInfo.Height - 1 );
	}

	// get new coordinates
	X = (int)Cus->X;
	Y = (int)Cus->Y;

	// blit part 1
	Result = geBitmap_Blit( Cus->CopyBmp, 0, 0, Cus->OutputBmp, X, Y, BmpInfo.Width - X, BmpInfo.Height - Y );
	assert( Result == GE_TRUE );

	// blit part 2
	if ( ( X != 0 ) && ( Y != 0 ) )
	{
		Result = geBitmap_Blit( Cus->CopyBmp, BmpInfo.Width - X, BmpInfo.Height - Y, Cus->OutputBmp, 0, 0, X, Y );
		assert( Result == GE_TRUE );
	}

	// blit part 3
	if ( Y != 0 )
	{
		Result = geBitmap_Blit( Cus->CopyBmp, 0, BmpInfo.Height - Y, Cus->OutputBmp, X, 0, BmpInfo.Width - X, Y );
		assert( Result == GE_TRUE );
	}

	// blit part 4
	if ( X != 0 )
	{
		Result = geBitmap_Blit( Cus->CopyBmp, BmpInfo.Width - X, 0, Cus->OutputBmp, 0, Y, X, BmpInfo.Height - Y );
		assert( Result == GE_TRUE );
	}

	// all done
	return GE_TRUE;

} // ScrollM_Update()
