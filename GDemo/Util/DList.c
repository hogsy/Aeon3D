/****************************************************************************************/
/*  DList.c																				*/
/*                                                                                      */
/*  Description:    Link list management functions										*/
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
#include <memory.h>
#include "ErrorLog.h"
#include "Ram.h"
#include "DList.h"

static DList_NodeP DList_InitArray( int32 itemN, int32 dataSize )
{
	DList_NodeP NodeArray, Node;
	int i;

	NodeArray = (DList_NodeP)geRam_Allocate( itemN * (sizeof( DList_NodeS)+dataSize ) );
	if( NodeArray == NULL )
	{
		geErrorLog_AddString(GE_ERR_MEMORY_RESOURCE,"DList_InitArray: Allocation failed.\n",NULL);
		return( NULL );
	}
	Node = NodeArray;
	for( i = 0; i < (itemN-1); i++ )
	{
		Node->Next = (DList_NodeP)(((char*)NodeArray) + ((i+1)* (sizeof( DList_NodeS)+dataSize) ));
		Node = Node->Next;
		Node->Prev = NULL;
	}
	Node->Next = NULL;
	return( NodeArray );
}

DListP DList_Create( int32 itemN, int32 dataSize )
{
	DListP DList;

	DList = (DListP)geRam_Allocate( sizeof( DListS ) );
	if( DList == NULL )
	{
		geErrorLog_AddString(GE_ERR_MEMORY_RESOURCE,"DList_Create: Allocation failed.\n",NULL);
		return( NULL );
	}
	DList->Head = NULL;
	DList->Tail = NULL;
	DList->DataSize = dataSize;

	if( itemN )
		DList->NodeArray = DList_InitArray( itemN, dataSize );
	else
		DList->NodeArray = NULL;
	DList->FreeNode = DList->NodeArray;
	DList->NodeN = 0;

	return( DList );
}
	

//Returns if any nodes have  been added to the list
int32 DList_NodeN( DListP List )
{
	return( List->NodeN );
}

static DList_NodeP DList_NewNode( DListP List )
{
	DList_NodeP Node;

	if( List->NodeArray )
	{
		Node = List->FreeNode;
		if( Node )
		{
			List->FreeNode = Node->Next;
			Node->Next = NULL;
		}
	}
	else
	{
		Node = (DList_NodeP)geRam_Allocate( sizeof( DList_NodeS)+List->DataSize );
		Node->Prev = NULL;
		Node->Next = NULL;
	}
	if( Node )
		List->NodeN++;
	return( Node );
}

static void DList_DestroyNode( DListP List, DList_NodeP Node, DList_DestroyCB Callback, void* Context )
{
	if( Callback )
		(*Callback)( Node->Data, Context );
	if( List->NodeArray )
	{
		Node->Prev = NULL;
		Node->Next = List->FreeNode;
		List->FreeNode = Node;
	}
	else
	{
		geRam_Free( Node );
	}
	List->NodeN--;

}

geBoolean DList_PreInsert( DListP List, DList_NodeHandle NodeH, void* Data )
{
	DList_NodeP Node, NewNode;

	switch( NodeH )
	{
	case DLIST_HEAD:
		Node = List->Head;
		break;

	case DLIST_TAIL:
		Node = List->Tail;
		break;

	default:
		Node = (DList_NodeP)NodeH;
		break;
	}
	NewNode = DList_NewNode( List );
	if( NewNode == NULL )
	{
		geErrorLog_AddString(-1,"DList_PreInsert: DList_NewNode failed.\n",NULL);
		return( FALSE );
	}
	
	memcpy( NewNode->Data, Data, List->DataSize );
	// the  List is empty.
	//Make the new node both the head and tail
	if( List->Head == NULL )
	{
		List->Head = NewNode;
		List->Tail = NewNode;
		return( TRUE );
	}
	// we are preinserting before the header
	// make the new node the head.
	if( List->Head == Node )
	{
		List->Head = NewNode;
		Node->Prev = NewNode;
		NewNode->Next = Node;
		return( TRUE );
	}

	Node->Prev->Next = NewNode;
	NewNode->Prev = Node->Prev;
	Node->Prev = NewNode;
	NewNode->Next = Node;
	return( TRUE );
}

geBoolean DList_Insert( DListP List, DList_NodeHandle NodeH, void* Data )
{
	DList_NodeP Node, NewNode;

	switch( NodeH )
	{
	case DLIST_HEAD:
		Node = List->Head;
		break;

	case DLIST_TAIL:
		Node = List->Tail;
		break;

	default:
		Node = (DList_NodeP)NodeH;
		break;
	}
	NewNode = DList_NewNode( List );
	if( NewNode == NULL )
	{
		geErrorLog_AddString(-1,"DList_Insert: DList_NewNode failed.\n",NULL);
		return( FALSE );
	}
	
	memcpy( NewNode->Data, Data, List->DataSize );
	// the  List is empty.
	//Make the new node both the head and tail
	if( List->Head == NULL )
	{
		List->Head = NewNode;
		List->Tail = NewNode;
		return( TRUE );
	}
	// we are inserting after the tail
	// make the new node the tail.
	if( List->Tail == Node )
	{
		List->Tail = NewNode;
		Node->Next = NewNode;
		NewNode->Prev = Node;
		return( TRUE );
	}

	Node->Next->Prev = NewNode;
	NewNode->Next = Node->Next;
	Node->Next = NewNode;
	NewNode->Prev = Node;
	return( TRUE );
}

geBoolean DList_Get( DListP List, DList_NodeHandle *NodeH, void* Data )
{
	DList_NodeP Node;

	switch( *NodeH )
	{
	case DLIST_HEAD:
		Node = List->Head;
		break;

	case DLIST_TAIL:
		Node = List->Tail;
		break;

	default:
		Node = (DList_NodeP)*NodeH;
		break;
	}
	*NodeH = (DList_NodeHandle)Node;
	if( Node == NULL )
		return FALSE ;
	
	if( Data )
		memcpy( Data, Node->Data, List->DataSize );
	return( TRUE );
}

geBoolean DList_Set( DListP List, DList_NodeHandle *NodeH, void* Data )
{
	DList_NodeP Node;

	switch( *NodeH )
	{
	case DLIST_HEAD:
		Node = List->Head;
		break;

	case DLIST_TAIL:
		Node = List->Tail;
		break;

	default:
		Node = (DList_NodeP)*NodeH;
		break;
	}
	*NodeH = (DList_NodeHandle)Node;
	if( Node == NULL )
	{
		geErrorLog_AddString(-1,"DList_Set: Invalid node\n",NULL);
		return( FALSE );
	}
	
	if( Data )
		memcpy( Node->Data, Data, List->DataSize );
	return( TRUE );
}


geBoolean DList_GetNext( DListP List, DList_NodeHandle* NodeH, void* Data )
{
	DList_NodeP Node;

	switch( *NodeH )
	{
	case DLIST_HEAD:
		Node = List->Head;
		if( Node )
			Node = Node->Next;
		break;

	case DLIST_TAIL:
		return( FALSE );

	case 0:
		Node = List->Head;
		break;

	default:
		Node = (DList_NodeP)*NodeH;
		if( Node )
			Node = Node->Next;
		break;
	}
	*NodeH = (DList_NodeHandle)Node;
	if( Node == NULL )
	{
		geErrorLog_AddString(-1,"DList_GetNext: Invalid node\n",NULL);
		return( FALSE );
	}
	
	if( Data )
		memcpy( Data, Node->Data, List->DataSize );
	return(TRUE );
}

geBoolean DList_GetPrev( DListP List, DList_NodeHandle* NodeH, void* Data )
{
	DList_NodeP Node;

	switch( *NodeH )
	{
	case DLIST_HEAD:
		Node = List->Head;
		if( Node )
			Node = Node->Prev;
		break;

	case DLIST_TAIL:
		return( FALSE );

	case 0:
		Node = List->Tail;
		break;

	default:
		Node = (DList_NodeP)*NodeH;
		if( Node )
			Node = Node->Prev;
		break;
	}
	if( Node == NULL )
	{
		geErrorLog_AddString(-1,"DList_GetPrev: Invalid node\n",NULL);
		return( FALSE );
	}
	
	if( Data != NULL )
		memcpy( Data, Node->Data, List->DataSize );
	*NodeH = (DList_NodeHandle)Node;
	return(TRUE );
}

geBoolean DList_Unlink( DListP List, DList_NodeHandle NodeH, void* Data, DList_DestroyCB Callback, void * Context)
{
	DList_NodeP Node;

	switch( NodeH )
	{
	case DLIST_HEAD:
		Node = List->Head;
		break;

	case DLIST_TAIL:
	{
		Node = List->Tail;
		break;
	}

	default:
		Node = (DList_NodeP)NodeH;
		break;
	}
	if( !Node )
	{
		geErrorLog_AddString(-1,"DList_Unlink: Invalid node\n",NULL);
		return( FALSE );
	}
	if( Data )
		memcpy( Data, Node->Data, List->DataSize );
	if( Node == List->Head )
	{
		List->Head = Node->Next;
		if( List->Head )
			List->Head->Prev = NULL;
	}
	if( Node == List->Tail )
	{
		List->Tail = Node->Prev;
		if( List->Tail )
			List->Tail->Next = NULL;
	}

	{
		if( Node->Next )
				Node->Next->Prev = Node->Prev;

		if( Node->Prev )
			Node->Prev->Next = Node->Next;
	}
	DList_DestroyNode( List, Node, Callback, Context );

	return(TRUE );
}

geBoolean DList_Search( DListP List, DList_NodeHandle* NodeH, void* Data, DList_SearchCB CallBack )
{
	DList_NodeP Node;

	Node = List->Head;
	while( Node )
	{
		if( !CallBack( &Node->Data, Data ) )
		{
			*NodeH = (DList_NodeHandle)Node;
			break;
		}
		Node = Node->Next;
	}
	return( Node != NULL );
}





void DList_Destroy( DListP *List, DList_DestroyCB CallBack, void* Context )
{
	while( (*List)->Head )
	{
		DList_Unlink( *List, DLIST_HEAD, NULL, CallBack, Context );
	}
	if( (*List)->NodeArray )
		geRam_Free( (*List)->NodeArray  );
	geRam_Free( *List );
}


#ifdef SELF_TEST
#include <stdio.h>
int32 SearchCB( void* NodeData, void* CompData )
{
	int *i, *j;

	i = (int*)NodeData;
	j = (int*)CompData;

	return( !(*i == *j) );
}
void DestroyCB( void* NodeData )
{
	int *i = (int*)NodeData;

	printf( "D%d", *i);
}

void main()
{
	DListP List;
	DList_NodeHandle Node;
	int i, data;
	List = DList_Create( 20, sizeof( int ) );
	for( i = 0; i < 10; i++ )
	{
		DList_PreInsert( List, DLIST_HEAD, &i );
		DList_Insert( List, DLIST_TAIL, &i );
	}

	Node = NULL;
	for( i = 0; i < DList_NodeN(List ); i++ )
	{
		DList_GetNext( List, &Node, &data );
		printf( "%d ", data );
	}
	printf( "\n" );

	Node = NULL;
	for( i = 0; i < DList_NodeN(List ); i++ )
	{
		DList_GetPrev( List, &Node, &data );
		printf( "%d ", data );
	}
	printf( "\n" );
	data = 5;
	if( DList_Search( List, &Node, &data, SearchCB ) )
		printf( "found %d\n", data );
	DList_Destroy( &List, DestroyCB );
}

#endif //SELF_TEST

