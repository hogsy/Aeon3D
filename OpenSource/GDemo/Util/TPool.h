/****************************************************************************************/
/*  TPool.h																				*/
/*                                                                                      */
/*  Description:    A poor mans texture pool											*/
/*                                                                                      */
/*  The contents of this file are subject to the Genesis3D Public License               */
/*  Version 1.01 (the "License"); you may not use this file except in                   */
/*  compliance with the License. You may obtain a copy of the License at                */
/*  http://www.genesis3d.com                                                            */
/*                                                                                      */
/*  Software distributed under the License is distributed on an "AS IS"                 */
/*  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See                */
/*  the License for the specific language governing rights and limitations              */
/*  under the License.                                                                  */
/*                                                                                      */
/*  Copyright (C) 1996-1999 Eclipse Entertainment, L.L.C. All Rights Reserved           */
/*                                                                                      */
/****************************************************************************************/
#ifndef TPOOL_H
#define TPOOL_H

#include "genesis.h"

#ifdef __cplusplus
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
//	Textture pool pointer
////////////////////////////////////////////////////////////////////////////////////////
typedef struct	TexturePool	TexturePool;


////////////////////////////////////////////////////////////////////////////////////////
//	Prototypes
////////////////////////////////////////////////////////////////////////////////////////

//	Create the texture pool.
//
////////////////////////////////////////////////////////////////////////////////////////
TexturePool * TPool_Create(
	geWorld	*World );	// world to add bitmaps to

//	Destroy the texture pool.
//
////////////////////////////////////////////////////////////////////////////////////////
void TPool_Destroy(
	TexturePool	**TPool );		// texture pool to zap

//	Get a bitmap from the texture pool.
//
////////////////////////////////////////////////////////////////////////////////////////
geBitmap * TPool_Get(
	TexturePool	*TPool,	// texture pool to retrieve it from
	int			Num );	// texture that we want

//	Change the world that the textures are tied to.
//
////////////////////////////////////////////////////////////////////////////////////////
void TPool_ChangeWorld(
	TexturePool	*TPool,		// texture pool whose world will change
	geWorld		*World );	// the new world

//	Add a texture to the texture pool.
//
////////////////////////////////////////////////////////////////////////////////////////
int TPool_Add(
	TexturePool	*TPool,			// texture pool to add texture to
	geVFile		*File,			// file system to use
	char		*Name,			// texture name
	char		*AlphaName );	// name of alpha


#ifdef __cplusplus
	}
#endif
#pragma warning ( default : 4068 )

#endif
