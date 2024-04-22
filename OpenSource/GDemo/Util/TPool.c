/****************************************************************************************/
/*  TPool.c																				*/
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
#include <string.h>
#include <assert.h>
#include "genesis.h"
#include "BitmapUtil.h"
#include "Ram.h"
#include "IndexList.h"
#include "TPool.h"
#include "ErrorLog.h"


////////////////////////////////////////////////////////////////////////////////////////
//	Texture pool struct.
////////////////////////////////////////////////////////////////////////////////////////
typedef struct TexturePool
{
	IndexList	*BmpList;
	geWorld		*World;

} TexturePool;


////////////////////////////////////////////////////////////////////////////////////////
//	Texture info struct.
////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	char		*Name;
	geBitmap	*Bmp;

} TextureInfo;



////////////////////////////////////////////////////////////////////////////////////////
//
//	TPool_Create()
//
//	Create the texture pool.
//
////////////////////////////////////////////////////////////////////////////////////////
TexturePool * TPool_Create(
	geWorld	*World )	// world to add bitmaps to
{

	// locals
	TexturePool	*TPool;

	// ensure valid data
	assert( World != NULL );

	// allocate texture pool struct
	TPool = geRam_AllocateClear( sizeof( *TPool ) );
	if ( TPool == NULL )
	{
		geErrorLog_AddString(GE_ERR_MEMORY_RESOURCE,"TPool_Create: geRam_Allocate failed.\n",NULL);
		return NULL;
	}

	// create bmplist
	TPool->BmpList = IndexList_Create( 10, 10 );
	if ( TPool->BmpList == NULL )
	{
		TPool_Destroy( &TPool );
		geErrorLog_AddString(-1,"TPool_Create: IndexList_Create failed.\n",NULL);
		return NULL;
	}

	// all done
	TPool->World = World;
	return TPool;

} // TPool_Create()



////////////////////////////////////////////////////////////////////////////////////////
//
//	TPool_Destroy()
//
//	Destroy the texture pool.
//
////////////////////////////////////////////////////////////////////////////////////////
void TPool_Destroy(
	TexturePool	**TPool )		// texture pool to zap
{

	// locals
	TexturePool	*DeadTPool;
	int			i;

	// ensure valid data
	assert( TPool != NULL );

	// setup tpool pointer
	DeadTPool = *TPool;
	assert( DeadTPool != NULL );

	// free all textures
	if ( DeadTPool->BmpList != NULL )
	{

		// locals
		int			Count;
		TextureInfo	*TInfo;

		// free each one
		Count = IndexList_GetListSize( DeadTPool->BmpList );
		for ( i = 0; i < Count; i++ )
		{

			// skip this slot if its empty
			TInfo = IndexList_GetElement( DeadTPool->BmpList, i );
			if ( TInfo == NULL )
			{
				continue;
			}

			// remove this element
			IndexList_DeleteElement( DeadTPool->BmpList, i );

			// free the texture
			geWorld_RemoveBitmap( DeadTPool->World, TInfo->Bmp );
			geBitmap_Destroy( &( TInfo->Bmp ) );

			// free the struct
			assert( TInfo->Name != NULL );
			geRam_Free( TInfo->Name );
			geRam_Free( TInfo );
		}

		// free the bitmap list
		IndexList_Destroy( &( DeadTPool->BmpList ) );
	}

	// free the texture pool
	geRam_Free( DeadTPool );

	// zap pointer
	TPool = NULL;

} // TPool_Destroy()



////////////////////////////////////////////////////////////////////////////////////////
//
//	TPool_Get()
//
//	Get a bitmap from the texture pool.
//
////////////////////////////////////////////////////////////////////////////////////////
geBitmap * TPool_Get(
	TexturePool	*TPool,	// texture pool to retrieve it from
	int			Num )	// texture that we want
{

	// locals
	TextureInfo	*TInfo;

	// ensure valid data
	assert( TPool != NULL );
	assert( Num >= 0 );
	assert( Num < IndexList_GetListSize( TPool->BmpList ) );

	// return bitmap pointer
	TInfo = IndexList_GetElement( TPool->BmpList, Num );
	assert( TInfo != NULL );
	return TInfo->Bmp;

} // TPool_Get()



////////////////////////////////////////////////////////////////////////////////////////
//
//	TPool_ChangeWorld()
//
//	Change the world that the textures are tied to.
//
////////////////////////////////////////////////////////////////////////////////////////
void TPool_ChangeWorld(
	TexturePool	*TPool,		// texture pool whose world will change
	geWorld		*World )	// the new world
{

	// locals
	int			i;
	geBoolean	Result;
	int			Count;
	TextureInfo	*TInfo;

	// ensure valid data
	assert( TPool != NULL );
	assert( World != NULL );

	// remove each bitmap from current world and insert it into new one
	Count = IndexList_GetListSize( TPool->BmpList );
	for ( i = 0; i < Count; i++ )
	{

		// skip this slot if its empty
		TInfo = IndexList_GetElement( TPool->BmpList, i );
		if ( TInfo == NULL )
		{
			continue;
		}

		// remove it and reinsert it into new world
		Result = geWorld_RemoveBitmap( TPool->World, TInfo->Bmp );
		assert( Result == GE_TRUE );
		// this was a bug: FM
		//Result = geWorld_AddBitmap( TPool->World, TInfo->Bmp );
		Result = geWorld_AddBitmap( World, TInfo->Bmp );
		assert( Result == GE_TRUE );
	}

	// save new world pointer
	TPool->World = World;

} // TPool_ChangeWorld()



////////////////////////////////////////////////////////////////////////////////////////
//
//	TPool_Add()
//
//	Add a texture to the texture pool.
//
////////////////////////////////////////////////////////////////////////////////////////
int TPool_Add(
	TexturePool	*TPool,			// texture pool to add texture to
	geVFile		*File,			// file system to use
	char		*Name,			// texture name
	char		*AlphaName )	// name of alpha
{

	// locals
	int			Slot;
	TextureInfo	*TInfo;
	int			Length;

	// ensure valid data
	assert( TPool != NULL );
	assert( Name != NULL );

	// get string length
	Length = strlen( Name );
	assert( Length > 0 );

	// check if the texture already exists
	{

		// locals
		int	ListSize;
		int	i;

		// check all currently loaded textures
		ListSize = IndexList_GetListSize( TPool->BmpList );
		for ( i = 0; i < ListSize; i++ )
		{
			
			// skip this slot if its empty
			TInfo = IndexList_GetElement( TPool->BmpList, i );
			if ( TInfo == NULL )
			{
				continue;
			}

			// if its a match then just return the id
			if ( strnicmp( Name, TInfo->Name, Length ) == 0 )
			{
				return i;
			}
		}
	}

	// get an empty slot in the list
	Slot = IndexList_GetEmptySlot( TPool->BmpList );
	if ( Slot == -1 )
	{
		geErrorLog_AddString(-1,"TPool_Add: IndexList_GetEmptySlot failed.\n",NULL);
		return -1;
	}

	// allocate texture info struct
	TInfo = geRam_AllocateClear( sizeof( *TInfo ) );
	if ( TInfo == NULL )
	{
		geErrorLog_AddString(GE_ERR_MEMORY_RESOURCE,"TPool_Add: geRam_AllocateClear failed.\n",NULL);
		return -1;
	}

	// copy texture name
	TInfo->Name = geRam_Allocate( Length + 1 );
	if ( TInfo->Name == NULL )
	{
		geRam_Free( TInfo );
		geErrorLog_AddString(GE_ERR_MEMORY_RESOURCE,"TPool_Add: geRam_Allocate failed.\n",NULL);
		return -1;
	}
	strcpy( TInfo->Name, Name );

	// create the texture
	TInfo->Bmp = BitmapUtil_CreateFromFileName( File, Name, AlphaName, GE_FALSE, GE_FALSE );
	if ( TInfo->Bmp == NULL )
	{
		geRam_Free( TInfo->Name );
		geRam_Free( TInfo );
		geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE,"TPool_Add: BitmapUtil_CreateFromFileName failed.\n",NULL);
		return -1;
	}
	geWorld_AddBitmap( TPool->World, TInfo->Bmp );

	// add texture to the list
	IndexList_AddElement( TPool->BmpList, Slot, TInfo );

	// return the index
	return Slot;

} // TPool_Add()
