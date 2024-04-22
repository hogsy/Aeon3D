/****************************************************************************************/
/*  Bolt.c                                                                              */
/*                                                                                      */
/*  Author: Peter Siamidis                                                              */
/*  Description:    Electric Bolt effect                				                */
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
#pragma warning ( disable : 4514 )
#include <memory.h>
#include <assert.h>
#include "EffectI.h"
#include "Ram.h"
#include "Bolt.h"
#include "EffectC.h"
#include "Errorlog.h"


////////////////////////////////////////////////////////////////////////////////////////
//	Resource struct
////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	EffectResource	*ExternalResource;	// external resources
	int				TypeID;				// effect type id

} BoltResource;


////////////////////////////////////////////////////////////////////////////////////////
//	Interface setup
////////////////////////////////////////////////////////////////////////////////////////
static char *		Bolt_GetName( void );
static void *		Bolt_Create( EffectResource *Resource, int TypeID );
static void			Bolt_Destroy( void **ResourceToZap );
static void *		Bolt_Add( BoltResource *Resource, Bolt *Data, Bolt *AttachData );
static void			Bolt_Remove( BoltResource *Resource, Bolt *Data );
static geBoolean	Bolt_Process( BoltResource *Resource, float TimeDelta, Bolt *Data );
static geBoolean	Bolt_Frame( BoltResource *Resource, float TimeDelta );
static geBoolean	Bolt_Modify( BoltResource *Resource, Bolt *Data, Bolt *NewData, uint32 Flags );
static void			Bolt_Pause( BoltResource *Resource, Bolt *Data, geBoolean Pause );
Effect_Interface	Bolt_Interface =
{
	Bolt_GetName,
	Bolt_Create,
	Bolt_Destroy,
	Bolt_Add,
	Bolt_Remove,
	Bolt_Process,
	Bolt_Frame,
	Bolt_Modify,
	Bolt_Pause,
	sizeof( Bolt )
};


////////////////////////////////////////////////////////////////////////////////////////
//	Bolt reserved struct
////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	GE_LVertex	Vertex[4];
	geVec3d		Current;
	geVec3d		LastA0;
	geVec3d		LastB0;
	geVec3d		LastA3;
	geVec3d		LastB3;
	int32		BoltCount;
	int32		CurrentBolt;
	gePoly		**Poly;
	float		*BoltLifeList;
	float		BoltDelay;
	geBoolean	EndReached;

} Reserved;



////////////////////////////////////////////////////////////////////////////////////////
//
//	Bolt_GetName()
//
////////////////////////////////////////////////////////////////////////////////////////
static char * Bolt_GetName(
	void )	// no parameters
{
	return "Bolt";

} // Bolt_GetName()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Bolt_Create()
//
////////////////////////////////////////////////////////////////////////////////////////
static void * Bolt_Create(
	EffectResource	*ExternalResource,	// external resources
	int				TypeID )			// effect type id
{

	// locals
	BoltResource	*Resource;

	// ensure valid data
	assert( ExternalResource != NULL );
	assert( TypeID > 0 );

	// allocate the resource list struct
	Resource = geRam_AllocateClear( sizeof( *Resource ) );
	if ( Resource == NULL )
	{
		geErrorLog_AddString( GE_ERR_MEMORY_RESOURCE, "Bolt_Create: failed to create BoltResource struct.", NULL );
		return NULL;
	}

	// save passed resource list
	Resource->ExternalResource = ExternalResource;

	// save type id
	Resource->TypeID = TypeID;

	// all done
	return Resource;

} // Bolt_Create()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Bolt_Destroy()
//
////////////////////////////////////////////////////////////////////////////////////////
static void Bolt_Destroy(
	void	**ResourceToZap )	// resource list
{

	// locals
	BoltResource	*Resource;

	// ensure valid data
	assert( ResourceToZap != NULL );
	assert( *ResourceToZap != NULL );

	// get resource list
	Resource = *ResourceToZap;

	// zap resource list
	geRam_Free( Resource );
	ResourceToZap = NULL;

} // Bolt_Destroy()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Bolt_Add()
//
////////////////////////////////////////////////////////////////////////////////////////
static void * Bolt_Add(
	BoltResource	*Resource,		// available resources
	Bolt			*Data,			// effect data
	Bolt			*AttachData )	// data of effect to attach to
{

	// locals
	Bolt		*NewData;
	Reserved	*NewReservedData;
	int32		i;

	// ensure valid data
	assert( Resource != NULL );
	assert( Data != NULL );

	// fail if type ids don't match
	if ( Resource->TypeID != Data->TypeID )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Bolt_Add: effect struct and effect type don't match.", NULL );
		return NULL;
	}

	// allocate effect data
	NewData = geRam_Allocate( sizeof( Bolt ) );
	if ( NewData == NULL )
	{
		geErrorLog_AddString( GE_ERR_MEMORY_RESOURCE, "Bolt_Add: failed to create Bolt struct.", NULL );
		return NULL;
	}

	// copy passed data
	memcpy( NewData, Data, sizeof( *NewData ) );

	// allocate reservered effect data
	NewData->ReservedData = geRam_Allocate( sizeof( Reserved ) );
	if ( NewData->ReservedData == NULL )
	{
		geErrorLog_AddString( GE_ERR_MEMORY_RESOURCE, "Bolt_Add: failed to create bolt reserved data struct.", NULL );
		Bolt_Remove( Resource, NewData );
		return NULL;
	}
	NewReservedData = NewData->ReservedData;

	// fail if we have bad data
	if ( EffectC_IsColorGood( &( NewData->Color ) ) == GE_FALSE )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Bolt_Add: effect not added, Color is bad.", NULL );
		return NULL;
	}
	if ( NewData->Texture == NULL )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Bolt_Add: effect not added, Texture is bad.", NULL );
		return NULL;
	}
	if ( NewData->Offset < 0.0f )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Bolt_Add: effect not added, Offset is bad.", NULL );
		return NULL;
	}
	if ( NewData->SegmentLength <= 0 )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Bolt_Add: effect not added, SegmentLength is bad.", NULL );
		return NULL;
	}
	if ( NewData->SegmentWidth <= 0.0f )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Bolt_Add: effect not added, SegmentWidth is bad.", NULL );
		return NULL;
	}
	if ( NewData->BoltLimit <= 0 )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Bolt_Add: effect not added, BoltLimit is bad.", NULL );
		return NULL;
	}
	if ( NewData->CompleteLife < 0.0f )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Bolt_Add: effect not added, CompleteLife is bad.", NULL );
		return NULL;
	}
	if ( NewData->BoltLife < 0.0f )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Bolt_Add: effect not added, BoltLife is bad.", NULL );
		return NULL;
	}
	if ( NewData->BoltCreate < 0.0f )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Bolt_Add: effect not added, BoltCreate is bad.", NULL );
		return NULL;
	}

	// setup reserved data struct
	NewReservedData->BoltDelay = 0.0f;
	NewReservedData->EndReached = GE_FALSE;
	NewReservedData->CurrentBolt = 0;
	NewReservedData->BoltCount = 0;
	NewReservedData->Poly = geRam_Allocate( NewData->BoltLimit * sizeof( gePoly * ) );
	if ( NewReservedData->Poly == NULL )
	{
		geErrorLog_AddString( GE_ERR_MEMORY_RESOURCE, "Bolt_Add: failed to create bolt poly struct.", NULL );
		Bolt_Remove( Resource, NewData );
		return NULL;
	}
	memset( NewReservedData->Poly, 0, NewData->BoltLimit * sizeof( gePoly * ) );
	NewReservedData->BoltLifeList = geRam_Allocate( NewData->BoltLimit * sizeof( float ) );
	if ( NewReservedData->BoltLifeList == NULL )
	{
		geErrorLog_AddString( GE_ERR_MEMORY_RESOURCE, "Bolt_Add: failed to create bolt poly life struct.", NULL );
		Bolt_Remove( Resource, NewData );
		return NULL;
	}
	memset( NewReservedData->BoltLifeList, 0, NewData->BoltLimit * sizeof( float ) );

	// setup default vertex info
	for ( i = 0; i < 4; i++ )
	{
		NewReservedData->Vertex[i].a = NewData->Color.a;
		NewReservedData->Vertex[i].r = NewData->Color.r;
		NewReservedData->Vertex[i].g = NewData->Color.g;
		NewReservedData->Vertex[i].b = NewData->Color.b;
	}
	NewReservedData->Vertex[0].u = 0.0f;
	NewReservedData->Vertex[0].v = 0.0f;
	NewReservedData->Vertex[1].u = 1.0f;
	NewReservedData->Vertex[1].v = 0.0f;
	NewReservedData->Vertex[2].u = 1.0f;
	NewReservedData->Vertex[2].v = 1.0f;
	NewReservedData->Vertex[3].u = 0.0f;
	NewReservedData->Vertex[3].v = 1.0f;

	// set current position
	geVec3d_Copy( &( NewData->Start ), &( NewReservedData->Current ) );

	// all done
	return NewData;

	// get rid of warnings
	AttachData;

} // Bolt_Add()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Bolt_Remove()
//
////////////////////////////////////////////////////////////////////////////////////////
static void Bolt_Remove(
	BoltResource	*Resource,	// available resources
	Bolt			*Data )		// effect data
{

	// locals
	int32		i;
	Reserved	*ReservedData;

	// ensure valid data
	assert( Resource != NULL );
	assert( Data != NULL );

	// remove any polys
	ReservedData = Data->ReservedData;
	for ( i = 0; i < Data->BoltLimit; i++ )
	{
		if ( ReservedData->Poly[i] != NULL )
		{
			geWorld_RemovePoly( Resource->ExternalResource->World, ReservedData->Poly[i] );
		}
	}

	// free effect data
	if ( ReservedData->BoltLifeList != NULL )
	{
		geRam_Free( ReservedData->BoltLifeList );
	}
	if ( ReservedData->Poly != NULL )
	{
		geRam_Free( ReservedData->Poly );
	}
	if ( Data->ReservedData != NULL )
	{
		geRam_Free( Data->ReservedData );
	}
	geRam_Free( Data );

} // Bolt_Remove()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Bolt_Process()
//
//	Perform processing on an indivual effect. A return of GE_FALSE means that
//	this it needs to be removed.
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean Bolt_Process(
	BoltResource	*Resource,	// available resources
	float			TimeDelta,	// elapsed time
	Bolt			*Data )		// effect data
{

	//locals
	int32		i;
	Reserved	*ReservedData;

	// ensure valid data
	assert( Data != NULL );
	assert( Data->ReservedData != NULL );

	// get reserved data
	ReservedData = Data->ReservedData;

	// kill it if required
	if ( Data->CompleteLife > 0.0f )
	{
		Data->CompleteLife -= TimeDelta;
		if ( Data->CompleteLife <= 0.0f )
		{
			return GE_FALSE;
		}
	}

	// age all existing bolts
	for ( i = 0; i < Data->BoltLimit; i++ )
	{
		if ( ReservedData->Poly[i] != NULL )
		{
			ReservedData->BoltLifeList[i] -= TimeDelta;
			if ( ReservedData->BoltLifeList[i] <= 0.0f )
			{
				geWorld_RemovePoly( Resource->ExternalResource->World, ReservedData->Poly[i] );
				ReservedData->Poly[i] = NULL;
				ReservedData->BoltLifeList[i] = 0.0f;
				ReservedData->BoltCount--;
			}
		}
	}

	// remove it if all its bolts have expired
	assert( ReservedData->BoltCount >= 0 );
	if ( ( ReservedData->BoltCount == 0 ) && ( ReservedData->EndReached == GE_TRUE ) )
	{

		// if its the looping type then reset its current position...
		if ( Data->Loop == GE_TRUE )
		{
			geVec3d_Copy( &( Data->Start ), &( ReservedData->Current ) );
			ReservedData->EndReached = GE_FALSE;
			ReservedData->BoltDelay = 0.0f;
		}
		// ...otherwise kill it
		else
		{
			return GE_FALSE;
		}
	}

	// update bolt delay time
	ReservedData->BoltDelay += TimeDelta;

	// add more bolts if the end has not been reached
	if ( ReservedData->EndReached == GE_FALSE )
	{

		//locals
		geVec3d		NextCurrent;
		geXForm3d	Xf;
		float		Distance;

		// create as many bolts as required for this time interval
		while ( ReservedData->BoltDelay > 0.0f )
		{

			// adjust bolt delay
			ReservedData->BoltDelay -= Data->BoltCreate;

			// get current distance to end point	
			Distance = geVec3d_DistanceBetween( &( ReservedData->Current ), &( Data->End ) );

			// make end point the next point...
			if ( Distance < Data->SegmentLength )
			{

				// mark end of bolt stuff
				ReservedData->BoltDelay = 0.0f;
				ReservedData->EndReached = GE_TRUE;

				// setup next point
				geVec3d_Copy( &( Data->End ), &NextCurrent );

				// get a transform from current and next bolt points
				EffectC_XFormFromVector( &( ReservedData->Current ), &NextCurrent, &Xf );
			}
			// ...or pick an intermediate point
			else
			{

				// locals
				geVec3d		Delta;

				// get next bolt point
				geVec3d_Subtract( &( Data->End ), &( ReservedData->Current ), &Delta );
				geVec3d_Normalize( &Delta );
				geVec3d_Scale( &Delta, (float)Data->SegmentLength, &Delta );
				geVec3d_Add( &( ReservedData->Current ), &Delta, &NextCurrent );

				// get a transform from current and next bolt points
				EffectC_XFormFromVector( &( ReservedData->Current ), &NextCurrent, &Xf );

				// randomly rotate this transform
				geXForm3d_RotateX( &Xf, EffectC_Frand( -3.14159f, 3.14159f ) * Data->Offset );
				geXForm3d_RotateY( &Xf, EffectC_Frand( -3.14159f, 3.14159f ) * Data->Offset );
				geXForm3d_RotateZ( &Xf, EffectC_Frand( -3.14159f, 3.14159f ) * Data->Offset );

				// get new next bolt point
				geXForm3d_GetIn( &Xf, &Delta );
				geVec3d_Scale( &Delta, (float)Data->SegmentLength, &Delta );
				geVec3d_Add( &( ReservedData->Current ), &Delta, &NextCurrent );
			}

			// THIS IS THE SINGLE BOLT CODE
			{

				// locals
				const geXForm3d	*CameraXf;
				geVec3d			In;
				geVec3d			Left;
				gePoly			*PolyA;

				// get camera xform
				CameraXf = geCamera_GetWorldSpaceXForm( Resource->ExternalResource->Camera );

				// get left vector
				geXForm3d_GetIn( CameraXf, &In );
				geVec3d_Subtract( &( ReservedData->Current ), &NextCurrent, &Left );
				geVec3d_CrossProduct( &In, &Left, &Left );
				geVec3d_Normalize( &Left );
				geVec3d_Scale( &Left, Data->SegmentWidth / 2.0f, &Left );

				// setup verticies
				ReservedData->Vertex[1].X = NextCurrent.X - Left.X;
				ReservedData->Vertex[1].Y = NextCurrent.Y - Left.Y;
				ReservedData->Vertex[1].Z = NextCurrent.Z - Left.Z;
				ReservedData->Vertex[2].X = NextCurrent.X + Left.X;
				ReservedData->Vertex[2].Y = NextCurrent.Y + Left.Y;
				ReservedData->Vertex[2].Z = NextCurrent.Z + Left.Z;
				if ( ReservedData->BoltCount != 0 )
				{
					ReservedData->Vertex[0].X = ReservedData->LastA0.X;
					ReservedData->Vertex[0].Y = ReservedData->LastA0.Y;
					ReservedData->Vertex[0].Z = ReservedData->LastA0.Z;
					ReservedData->Vertex[3].X = ReservedData->LastA3.X;
					ReservedData->Vertex[3].Y = ReservedData->LastA3.Y;
					ReservedData->Vertex[3].Z = ReservedData->LastA3.Z;
				}
				else
				{
					ReservedData->Vertex[0].X = ReservedData->Current.X - Left.X;
					ReservedData->Vertex[0].Y = ReservedData->Current.Y - Left.Y;
					ReservedData->Vertex[0].Z = ReservedData->Current.Z - Left.Z;
					ReservedData->Vertex[3].X = ReservedData->Current.X + Left.X;
					ReservedData->Vertex[3].Y = ReservedData->Current.Y + Left.Y;
					ReservedData->Vertex[3].Z = ReservedData->Current.Z + Left.Z;
				}

				// save last end points
				geVec3d_Set( &( ReservedData->LastA0 ), ReservedData->Vertex[1].X, ReservedData->Vertex[1].Y, ReservedData->Vertex[1].Z );
				geVec3d_Set( &( ReservedData->LastA3 ), ReservedData->Vertex[2].X, ReservedData->Vertex[2].Y, ReservedData->Vertex[2].Z );

				// setup color
				for ( i = 0; i < 4; i++ )
				{
					ReservedData->Vertex[i].a = Data->Color.a;
					ReservedData->Vertex[i].r = Data->Color.r;
					ReservedData->Vertex[i].g = Data->Color.g;
					ReservedData->Vertex[i].b = Data->Color.b;
				}

				// add the poly to the world
				PolyA = NULL;
				PolyA = geWorld_AddPoly( Resource->ExternalResource->World, ReservedData->Vertex, 4, Data->Texture, GE_TEXTURED_POLY, GE_RENDER_DEPTH_SORT_BF, 1.0f );

				// remove other polys to accomodate the new one if required
				if ( ReservedData->Poly[ReservedData->CurrentBolt] != NULL )
				{
					geWorld_RemovePoly( Resource->ExternalResource->World, ReservedData->Poly[ReservedData->CurrentBolt] );
					ReservedData->BoltCount--;
				}
				ReservedData->Poly[ReservedData->CurrentBolt] = PolyA;
				ReservedData->BoltLifeList[ReservedData->CurrentBolt] = Data->BoltLife;
				ReservedData->BoltCount++;
				ReservedData->CurrentBolt++;
				if ( ReservedData->CurrentBolt >= Data->BoltLimit )
				{
					ReservedData->CurrentBolt = 0;
				}

				// the last location
				geVec3d_Copy( &NextCurrent, &( ReservedData->Current ) );
			}
		}
	}

	// all done
	return GE_TRUE;

} // Bolt_Process()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Bolt_Frame()
//
//	Perform once-per-frame processing for all effects of this type.
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean Bolt_Frame(
	BoltResource	*Resource,		// available resources
	float			TimeDelta )		// elapsed time
{

	// all done
	return GE_TRUE;

	// get rid of warnings
	Resource;
	TimeDelta;

} // Bolt_Frame()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Bolt_Modify()
//
//	Adjust effect.
//
////////////////////////////////////////////////////////////////////////////////////////
geBoolean Bolt_Modify(
	BoltResource	*Resource,	// available resources
	Bolt			*Data,		// effect data
	Bolt			*NewData,	// new data
	uint32			Flags )		// adjustment flags
{

	// ensure valid data
	assert( Resource != NULL );
	assert( Data != NULL );
	assert( NewData != NULL );

	// adjust start
	if ( Flags & BOLT_START )
	{
		geVec3d_Copy( &( NewData->Start ), &( Data->Start ) );
	}

	// adjust end
	if ( Flags & BOLT_END )
	{
		geVec3d_Copy( &( NewData->End ), &( Data->End ) );
	}

	// adjust color
	if ( Flags & BOLT_COLOR )
	{
		memcpy( &( Data->Color ), &( NewData->Color ), sizeof( Data->Color ) );
	}

	// all done
	return GE_TRUE;

	// get rid of warnings
	Resource;

} // Bolt_Modify()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Bolt_Pause()
//
//	Pause/unpause effect.
//
////////////////////////////////////////////////////////////////////////////////////////
void Bolt_Pause(
	BoltResource	*Resource,	// available resources
	Bolt			*Data,		// effect data
	geBoolean		Pause )		// new pause state
{

	// locals
	Reserved	*ReservedData;
	int32		i;

	// ensure valid data
	assert( Resource != NULL );
	assert( Data != NULL );

	// get reserved data
	ReservedData = Data->ReservedData;
	assert( ReservedData != NULL );

	// remove all polys if required
	if ( Pause == GE_TRUE )
	{
		for ( i = 0; i < Data->BoltLimit; i++ )
		{
			if ( ReservedData->Poly[i] != NULL )
			{
				geWorld_RemovePoly( Resource->ExternalResource->World, ReservedData->Poly[i] );
				ReservedData->Poly[i] = NULL;
				ReservedData->BoltCount--;
			}
		}
	}

} // Bolt_Pause()
