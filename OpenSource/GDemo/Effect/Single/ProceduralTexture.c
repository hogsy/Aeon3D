/****************************************************************************************/
/*  ProceduralTexture.c                                                                 */
/*                                                                                      */
/*  Author: Peter Siamidis                                                              */
/*  Description:    Procedural texture													*/
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
#include <assert.h>
#include "EffectI.h"
#include "Ram.h"
#include "ProceduralTexture.h"
#include "EffectC.h"
#include "ErrorLog.h"
#include "ProcEng.h"


////////////////////////////////////////////////////////////////////////////////////////
//	Resource struct
////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	EffectResource	*ExternalResource;	// external resource list
	ProcEng			*ProcedureSystem;	// procedural texture system
	int				TypeID;				// effect type id

} ProceduralTextureResource;


////////////////////////////////////////////////////////////////////////////////////////
//	Interface setup
////////////////////////////////////////////////////////////////////////////////////////
static char *		ProceduralTexture_GetName( void );
static void *		ProceduralTexture_Create( EffectResource *Resource, int TypeID );
static void			ProceduralTexture_Destroy( void ** ResourceToZap );
static void *		ProceduralTexture_Add( ProceduralTextureResource *Resource, ProceduralTexture *Data, ProceduralTexture *AttachData );
static void			ProceduralTexture_Remove( ProceduralTextureResource *Resource, ProceduralTexture *Data );
static geBoolean	ProceduralTexture_Process( ProceduralTextureResource *Resource, float TimeDelta, ProceduralTexture *Data );
static geBoolean	ProceduralTexture_Frame( ProceduralTextureResource *Resource, float TimeDelta );
static geBoolean	ProceduralTexture_Modify( ProceduralTextureResource *Resource, ProceduralTexture *Data, ProceduralTexture *NewData, uint32 Flags );
static void			ProceduralTexture_Pause( ProceduralTextureResource *Resource, ProceduralTexture *Data, geBoolean Pause );
Effect_Interface	ProceduralTexture_Interface =
{
	ProceduralTexture_GetName,
	ProceduralTexture_Create,
	ProceduralTexture_Destroy,
	ProceduralTexture_Add,
	ProceduralTexture_Remove,
	ProceduralTexture_Process,
	ProceduralTexture_Frame,
	ProceduralTexture_Modify,
	ProceduralTexture_Pause,
	sizeof( ProceduralTexture )
};



////////////////////////////////////////////////////////////////////////////////////////
//
//	ProceduralTexture_GetName()
//
////////////////////////////////////////////////////////////////////////////////////////
static char * ProceduralTexture_GetName(
	void )	// no parameters
{
	return "Procedural Texture";

} // ProceduralTexture_GetName()



////////////////////////////////////////////////////////////////////////////////////////
//
//	ProceduralTexture_Create()
//
////////////////////////////////////////////////////////////////////////////////////////
static void * ProceduralTexture_Create(
	EffectResource	*ExternalResource,	// external resources
	int				TypeID )			// effect type id
{

	// locals
	ProceduralTextureResource	*Resource;

	// ensure valid data
	assert( ExternalResource != NULL );
	assert( TypeID > 0 );

	// allocate the resource list struct
	Resource = geRam_AllocateClear( sizeof( *Resource ) );
	if ( Resource == NULL )
	{
		geErrorLog_AddString( GE_ERR_MEMORY_RESOURCE, "ProceduralTexture_Create: failed to create ProceduralTextureResource struct.", NULL );
		return NULL;
	}

	// save passed resource list
	Resource->ExternalResource = ExternalResource;

	// create procedural system
	Resource->ProcedureSystem = ProcEng_Create( NULL, Resource->ExternalResource->World );
	if ( Resource->ProcedureSystem == NULL )
	{
		geErrorLog_AddString( GE_ERR_INTERNAL_RESOURCE, "ProceduralTexture_Create: failed to create procedural texture subsystem.", NULL );
		ProceduralTexture_Destroy( &Resource );
		return GE_FALSE;
	}

	// save type id
	Resource->TypeID = TypeID;

	// all done
	return Resource;

} // ProceduralTexture_Create()



////////////////////////////////////////////////////////////////////////////////////////
//
//	ProceduralTexture_Destroy()
//
////////////////////////////////////////////////////////////////////////////////////////
static void ProceduralTexture_Destroy(
	void	**ResourceToZap )	// resource list 
{

	// locals
	ProceduralTextureResource	*Resource;

	// ensure valid data
	assert( ResourceToZap != NULL );
	assert( *ResourceToZap != NULL );

	// get resource list
	Resource = *ResourceToZap;

	// free procedural texture system
	if ( Resource->ProcedureSystem != NULL )
	{
		ProcEng_Destroy( &( Resource->ProcedureSystem ) );
		Resource->ProcedureSystem = NULL;
	}

	// zap resource list
	geRam_Free( Resource );
	ResourceToZap = NULL;

} // ProceduralTexture_Destroy()



////////////////////////////////////////////////////////////////////////////////////////
//
//	ProceduralTexture_Add()
//
////////////////////////////////////////////////////////////////////////////////////////
static void * ProceduralTexture_Add(
	ProceduralTextureResource	*Resource,		// available resources
	ProceduralTexture			*Data,			// data of effect
	ProceduralTexture			*AttachData )	// data of effect to attach to
{

	// locals
	geBoolean			Result;
	ProceduralTexture	*NewData;

	// ensure valid data
	assert( Resource != NULL );
	assert( Resource->ProcedureSystem != NULL );
	assert( Data != NULL );

	// fail if type ids don't match
	if ( Resource->TypeID != Data->TypeID )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "ProceduralTexture_Add: effect struct and effect type don't match.", NULL );
		return NULL;
	}

	// allocate ProceduralTexture data
	NewData = geRam_Allocate( sizeof( *NewData ) );
	if ( NewData == NULL )
	{
		geErrorLog_AddString( GE_ERR_MEMORY_RESOURCE, "ProceduralTexture_Add: failed to create ProceduralTexture struct.", NULL );
		return NULL;
	}

	// copy passed data
	memcpy( NewData, Data, sizeof( *NewData ) );

	// verify passed data
	if ( NewData->UseWorldPoly == GE_FALSE )
	{
		if (	( NewData->Type == NULL ) ||
				( NewData->Texture == NULL ) ||
				( NewData->Scale <= 0.0f ) ||
				( ( NewData->PolyType != GE_TEXTURED_POINT ) && ( NewData->PolyType != GE_TEXTURED_POLY ) ) )
		{
			geErrorLog_AddString( GE_ERR_INVALID_PARMS, "ProceduralTexture_Add: effect not added, bad data.", NULL );
			ProceduralTexture_Remove( Resource, NewData );
			return NULL;
		}
	}
	else
	{
		if (	( NewData->Type == NULL ) ||
				( NewData->Texture == NULL ) )
		{
			geErrorLog_AddString( GE_ERR_INVALID_PARMS, "ProceduralTexture_Add: effect not added, bad data.", NULL );
			ProceduralTexture_Remove( Resource, NewData );
			return NULL;
		}
	}

	// do non world poly specific inits
	if ( NewData->UseWorldPoly == GE_FALSE )
	{

		// init vert data
		if ( NewData->PolyType == GE_TEXTURED_POINT )
		{
			NewData->VertCount = 1;
		}
		else
		{
			NewData->VertCount = 4;
		}

		// add poly
		NewData->Poly = geWorld_AddPoly( Resource->ExternalResource->World, NewData->Vertex, NewData->VertCount, NewData->Texture, NewData->PolyType, NewData->RenderFlags, NewData->Scale );
		if ( NewData->Poly == NULL )
		{
			geErrorLog_AddString( GE_ERR_SYSTEM_RESOURCE, "ProceduralTexture_Add: effect not added, failed to add procedural texture poly to the world.", NULL );
			ProceduralTexture_Remove( Resource, NewData );
			return NULL;
		}
	}

	// apply procedural
	Result = ProcEng_AddProcedural( Resource->ProcedureSystem, NewData->Type, &( NewData->Texture ), NewData->Parms );
	if ( Result == GE_FALSE )
	{
		geErrorLog_AddString( GE_ERR_INTERNAL_RESOURCE, "ProceduralTexture_Add: effect not added, failed to attach procedural to texture.", NULL );
		ProceduralTexture_Remove( Resource, NewData );
		return NULL;
	}

	// all done
	return NewData;

	// get rid of warnings
	AttachData;

} // ProceduralTexture_Add()



////////////////////////////////////////////////////////////////////////////////////////
//
//	ProceduralTexture_Remove()
//
////////////////////////////////////////////////////////////////////////////////////////
static void ProceduralTexture_Remove(
	ProceduralTextureResource	*Resource,	// available resources
	ProceduralTexture			*Data )		// effect data
{

	// ensure valid data
	assert( Resource != NULL );
	assert( Data != NULL );

	// free its poly
	if ( Data->Poly != NULL )
	{
		geWorld_RemovePoly( Resource->ExternalResource->World, Data->Poly );
		Data->Poly = NULL;
	}

	// free effect data
	geRam_Free( Data );

} // ProceduralTexture_Remove()



////////////////////////////////////////////////////////////////////////////////////////
//
//	ProceduralTexture_Process()
//
//	Perform processing on an indivual effect. A return of GE_FALSE means that the
//	effect needs to be removed.
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean ProceduralTexture_Process(
	ProceduralTextureResource	*Resource,		// available resources
	float						TimeDelta,		// elapsed time
	ProceduralTexture			*Data )			// effect data
{

	// all done
	return GE_TRUE;

	// get rid of warnings
	Resource;
	TimeDelta;
	Data;

} // ProceduralTexture_Process()



////////////////////////////////////////////////////////////////////////////////////////
//
//	ProceduralTexture_Frame()
//
//	Perform once-per-frame processing for all effects of this type.
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean ProceduralTexture_Frame(
	ProceduralTextureResource	*Resource,		// available resources
	float						TimeDelta )		// elapsed time
{

	// locals
	geBoolean	Result;

	// perform a frame of animation on the procedural texture system
	Result = ProcEng_Animate( Resource->ProcedureSystem, TimeDelta );
	if ( Result == GE_FALSE )
	{
		geErrorLog_AddString( GE_ERR_INTERNAL_RESOURCE, "ProceduralTexture_Frame: failed to animate procedural texture.", NULL );
	}

	// all done
	return GE_TRUE;

} // ProceduralTexture_Frame()



////////////////////////////////////////////////////////////////////////////////////////
//
//	ProceduralTexture_Modify()
//
//	Adjust the effect.
//
////////////////////////////////////////////////////////////////////////////////////////
geBoolean ProceduralTexture_Modify(
	ProceduralTextureResource	*Resource,	// available resources
	ProceduralTexture			*Data,		// effect data
	ProceduralTexture			*NewData,	// new data
	uint32						Flags )		// user flags
{

	// ensure valid data
	assert( Resource != NULL );
	assert( Data != NULL );
	assert( NewData != NULL );

	// adjust poly position
	if ( Flags & PROCEDURALTEXTURE_POS )
	{

		// locals
		int32	i;

		// adjust vertex coordinates
		for ( i = 0; i < Data->VertCount; i++ )
		{
			gePoly_GetLVertex( Data->Poly, i, &( Data->Vertex[i] ) );
			Data->Vertex[i].X = NewData->Vertex[i].X;
			Data->Vertex[i].Y = NewData->Vertex[i].Y;
			Data->Vertex[i].Z = NewData->Vertex[i].Z;
			gePoly_SetLVertex( Data->Poly, i, &( Data->Vertex[i] ) );
		}
	}

	// adjust poly color
	if ( Flags & PROCEDURALTEXTURE_COLOR )
	{

		// locals
		int32		i;

		// adjust vertex colors
		for ( i = 0; i < Data->VertCount; i++ )
		{
			gePoly_GetLVertex( Data->Poly, i, &( Data->Vertex[i] ) );
			Data->Vertex[i].r = NewData->Vertex[i].r;
			Data->Vertex[i].g = NewData->Vertex[i].g;
			Data->Vertex[i].b = NewData->Vertex[i].b;
			Data->Vertex[i].a = NewData->Vertex[i].a;
			gePoly_SetLVertex( Data->Poly, i, &( Data->Vertex[i] ) );
		}
	}

	// all done
	return GE_TRUE;

	// get rid of warnings
	Resource;

} // ProceduralTexture_Modify()



////////////////////////////////////////////////////////////////////////////////////////
//
//	ProceduralTexture_Pause()
//
//	Pause/unpause effect.
//
////////////////////////////////////////////////////////////////////////////////////////
void ProceduralTexture_Pause(
	ProceduralTextureResource	*Resource,	// available resources
	ProceduralTexture			*Data,		// effect data
	geBoolean					Pause )		// new pause state
{

	// ensure valid data
	assert( Resource != NULL );
	assert( Data != NULL );

	// get rid of warnings
	Resource;
	Data;
	Pause;

} // ProceduralTexture_Pause()
