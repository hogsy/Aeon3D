/****************************************************************************************/
/*  SPool.c																				*/
/*                                                                                      */
/*  Description:    A poor mans sound pool												*/
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
#include "Ram.h"
#include "IndexList.h"
#include "SPool.h"
#include "ErrorLog.h"


////////////////////////////////////////////////////////////////////////////////////////
//	Sound pool struct.
////////////////////////////////////////////////////////////////////////////////////////
typedef struct SoundPool
{
	geSound_System	*SoundSystem;
	IndexList		*DefList;

} SoundPool;


////////////////////////////////////////////////////////////////////////////////////////
//	Sound info struct.
////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	char		*Name;
	geSound_Def	*SoundDef;

} SoundInfo;



////////////////////////////////////////////////////////////////////////////////////////
//
//	SPool_Create()
//
//	Create the sound pool.
//
////////////////////////////////////////////////////////////////////////////////////////
SoundPool * SPool_Create(
	geSound_System	*SoundSystem )	// sound system to use
{

	// locals
	SoundPool	*SPool;

	// ensure valid data 
	assert( SoundSystem != NULL );

	// allocate sound pool struct
	SPool = geRam_Allocate( sizeof( *SPool ) );
	if ( SPool == NULL )
	{
		geErrorLog_AddString(GE_ERR_MEMORY_RESOURCE,"SPool_Create: geRam_Allocate failed.\n",NULL);
		return NULL;
	}
	memset( SPool, 0, sizeof( *SPool ) );

	// create sounddef list
	SPool->DefList = IndexList_Create( 10, 10 );
	if ( SPool->DefList == NULL )
	{
		SPool_Destroy( &SPool );
		geErrorLog_AddString(GE_ERR_MEMORY_RESOURCE,"SPool_Create: IndexList_Create failed.\n",NULL);
		return NULL;
	}

	// all done
	SPool->SoundSystem = SoundSystem;
	return SPool;

} // SPool_Create()



////////////////////////////////////////////////////////////////////////////////////////
//
//	SPool_Destroy()
//
//	Destroy the sound pool.
//
////////////////////////////////////////////////////////////////////////////////////////
void SPool_Destroy(
	SoundPool	**SPool )	// sound pool to zap
{

	// locals
	SoundPool	*DeadSPool;
	int			i;

	// ensure valid data
	assert( SPool != NULL );

	// setup spool pointer
	DeadSPool = *SPool;
	assert( DeadSPool != NULL );

	// free all sound defs
	if ( DeadSPool->DefList != NULL )
	{

		// locals
		int			Count;
		SoundInfo	*SInfo;

		// free each one
		Count = IndexList_GetListSize( DeadSPool->DefList );
		for ( i = 0; i < Count; i++ )
		{

			// skip this slot if its empty
			SInfo = IndexList_GetElement( DeadSPool->DefList, i );
			if ( SInfo == NULL )
			{
				continue;
			}

			// remove this element
			IndexList_DeleteElement( DeadSPool->DefList, i );

			// free the sound
			geSound_FreeSoundDef( DeadSPool->SoundSystem, SInfo->SoundDef );

			// free the struct
			assert( SInfo->Name != NULL );
			geRam_Free( SInfo->Name );
			geRam_Free( SInfo );
		}

		// free the sound def list
		IndexList_Destroy( &( DeadSPool->DefList ) );
	}

	// free the sound pool
	geRam_Free( DeadSPool );

	// zap pointer
	SPool = NULL;

} // SPool_Destroy()



////////////////////////////////////////////////////////////////////////////////////////
//
//	SPool_Get()
//
//	Get a sound def from the sound pool.
//
////////////////////////////////////////////////////////////////////////////////////////
geSound_Def * SPool_Get(
	SoundPool	*SPool,	// sound pool to retrieve it from
	int			Num )	// sound that we want
{

	// locals
	SoundInfo	*SInfo;

	// ensure valid data
	assert( SPool != NULL );
	assert( Num >= 0 );
	assert( Num < IndexList_GetListSize( SPool->DefList ) );

	// return sound def pointer
	SInfo = IndexList_GetElement( SPool->DefList, Num );
	assert( SInfo != NULL );
	return SInfo->SoundDef;

} // SPool_Get()



////////////////////////////////////////////////////////////////////////////////////////
//
//	SPool_Add()
//
//	Add a sound to the sound pool.
//
////////////////////////////////////////////////////////////////////////////////////////
int SPool_Add(
	SoundPool	*SPool,		// sound pool to add sound to
	geVFile		*File,		// file system to use
	char		*Name )		// sound name
{

	// locals
	geVFile		*SndFile;
	int			Slot;
	SoundInfo	*SInfo;
	int			Length;

	// ensure valid data
	assert( SPool != NULL );
	assert( Name != NULL );

	// get string length
	Length = strlen( Name );
	assert( Length > 0 );

	// check if the sound already exists
	{

		// locals
		int	ListSize;
		int	i;

		// check all currently loaded sound defs
		ListSize = IndexList_GetListSize( SPool->DefList );
		for ( i = 0; i < ListSize; i++ )
		{
			
			// skip this slot if its empty
			SInfo = IndexList_GetElement( SPool->DefList, i );
			if ( SInfo == NULL )
			{
				continue;
			}

			// if its a match then just return the id
			if ( strnicmp( Name, SInfo->Name, Length ) == 0 )
			{
				return i;
			}
		}
	}

	// allocate sound info struct
	SInfo = geRam_AllocateClear( sizeof( *SInfo ) );
	if ( SInfo == NULL )
	{
		geErrorLog_AddString(GE_ERR_MEMORY_RESOURCE,"SPool_Add: geRam_AllocateClear failed.\n",NULL);
		return -1;
	}

	// copy sound name
	SInfo->Name = geRam_Allocate( Length + 1 );
	if ( SInfo->Name == NULL )
	{
		geRam_Free( SInfo );
		geErrorLog_AddString(GE_ERR_MEMORY_RESOURCE,"SPool_Add: geRam_Allocate failed.\n",NULL);
		return -1;
	}
	strcpy( SInfo->Name, Name );

	// get an empty slot in the list
	Slot = IndexList_GetEmptySlot( SPool->DefList );
	if ( Slot == -1 )
	{
		geErrorLog_AddString(-1,"SPool_Add: IndexList_GetEmptySlot failed.\n",NULL);
		return -1;
	}

	// open the file
	if ( File == NULL )
	{
		SndFile = geVFile_OpenNewSystem( NULL, GE_VFILE_TYPE_DOS, Name, NULL, GE_VFILE_OPEN_READONLY );
	}
	else
	{
		SndFile = geVFile_Open( File, Name, GE_VFILE_OPEN_READONLY );
	}
	if ( SndFile == NULL )
	{
		geErrorLog_AddString(-1,"SPool_Add: Unable to open file.\n",Name);
		return -1;
	}

	// create the sound def
	SInfo->SoundDef = geSound_LoadSoundDef( SPool->SoundSystem, SndFile );
	geVFile_Close( SndFile );
	if ( SInfo->SoundDef == NULL )
	{
		geErrorLog_AddString(-1,"SPool_Add: geSound_LoadSoundDef failed.\n",NULL);
		return -1;
	}

	// add the sound to the list
	IndexList_AddElement( SPool->DefList, Slot, SInfo );

	// return the index
	return Slot;

} // SPool_Add()
