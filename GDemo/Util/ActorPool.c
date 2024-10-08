/****************************************************************************************/
/*  ActorPool.c                                                                         */
/*                                                                                      */
/*  Description:    Manages actor defs									                */
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
#include <assert.h>
#include <string.h>

#include "errorlog.h"
#include "ActorPool.h"
#include "Ram.h"
#include "DList.h"

typedef struct {
	char		Name[NAME_MAX];
	geActor_Def	*pActorDef;
	int32		Usage;
} ActorEntry;

typedef struct ActorPool{
	DListP Pool;
} ActorPool;


ActorPool * ActorPool_Create( void )
{
	ActorPool* pActorPool;

	pActorPool = (ActorPool*)geRam_Allocate( sizeof( ActorPool ) );
	if( pActorPool == NULL )
	{
		geErrorLog_AddString(GE_ERR_MEMORY_RESOURCE,"ActorPool_Create: Allocation failed.\n",NULL);
		return( NULL );
	}
	pActorPool->Pool = DList_Create( 0, sizeof(ActorEntry ) );
	if( pActorPool->Pool == NULL )
	{
		geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE,"ActorPool_Create: Unable to create linked list.\n",NULL);
		geRam_Free( pActorPool );
		return( NULL );
	}
	return( pActorPool );
}

static int32 ActorPool_Search( void* NodeData, void* CompData )
{
	ActorEntry* pActorEntry = (ActorEntry*)NodeData;
	char* CompName = (char*)CompData;

	return( stricmp( pActorEntry->Name, CompName ) );
}

geBoolean ActorPool_Exists( ActorPool* pActorPool, char* ActorName )
{
	// locals
	DList_NodeHandle	Node;

	if(DList_Search( pActorPool->Pool, &Node, ActorName, ActorPool_Search ) )
		return( TRUE );

	return FALSE;
}

geBoolean ActorPool_Add( ActorPool* pActorPool, char* ActorName )
{

	// locals
	ActorEntry			NewEntry;
	DList_NodeHandle	Node;
	geVFile				*File;
	char				ActorFileName[256];

	if(DList_Search( pActorPool->Pool, &Node, ActorName, ActorPool_Search ) )
		return( TRUE );
#ifndef NDEBUG
	{
	int32 sLen;
	sLen = strlen( ActorName );
	assert( sLen < NAME_MAX );
	}
#endif
	strcpy( NewEntry.Name, ActorName );
	NewEntry.Usage = 0;
	sprintf(ActorFileName,".\\actors\\%s.act", ActorName);
	File = geVFile_OpenNewSystem( NULL, GE_VFILE_TYPE_DOS, ActorFileName, NULL, GE_VFILE_OPEN_READONLY );
	if( File == NULL )
	{
		geErrorLog_AddString(-1,"ActorPool_Add: Unable to open file.\n",ActorName);
		return FALSE;
	}

	// create actor def 
	NewEntry.pActorDef = geActor_DefCreateFromFile( File );
	geVFile_Close ( File );
	if ( NewEntry.pActorDef == NULL )
	{
		geErrorLog_AddString(-1,"ActorPool_Add: Unable to create from file.\n",ActorName);
		return FALSE;
	}

	DList_Insert( pActorPool->Pool, DLIST_TAIL, &NewEntry );
	return( TRUE );
}



geActor_Def * ActorPool_Get( ActorPool* pActorPool, char* ActorName )
{
	ActorEntry ActorEntry;
	DList_NodeHandle Node;

	//Actor Add will add it if does not exist and return true if it does
	ActorPool_Add( pActorPool, ActorName );
	if(DList_Search( pActorPool->Pool, &Node, ActorName, ActorPool_Search ) )
	{
		DList_Get( pActorPool->Pool, &Node, &ActorEntry );
		ActorEntry.Usage++;
		DList_Set( pActorPool->Pool, &Node, &ActorEntry );
		return( ActorEntry.pActorDef );
	}
	return( NULL );
}


geBoolean ActorPool_Release( ActorPool* pActorPool, char* ActorName )
{
	ActorEntry ActorEntry;
	DList_NodeHandle Node;

	if(DList_Search( pActorPool->Pool, &Node, ActorName, ActorPool_Search ) )
	{
		DList_Get( pActorPool->Pool, &Node, &ActorEntry );
		if( ActorEntry.Usage > 0 )
			ActorEntry.Usage--;
		DList_Set( pActorPool->Pool, &Node, &ActorEntry );
		return( TRUE );
	}
	return( FALSE );
}

static void ActorPool_EntryDestroy( void* NodeData, void* Context )
{

	// locals
	geBoolean		Result;
	ActorEntry		*pActorEntry = (ActorEntry*)NodeData;

	// destroy the actor def
	Result = geActor_DefDestroy( &( pActorEntry->pActorDef ) );
	assert( Result == GE_TRUE );

	// get rid of warnings
	Context;
}

void ActorPool_Destroy( ActorPool** hActorPool )
{
	if( (*hActorPool)->Pool != NULL )
		DList_Destroy( &(*hActorPool)->Pool, ActorPool_EntryDestroy, NULL );
	geRam_Free( (*hActorPool) );
}
