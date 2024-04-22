/****************************************************************************************/
/*  Corona.c                                                                            */
/*                                                                                      */
/*  Author: Peter Siamidis                                                              */
/*  Description:    Displays a bitmap which fades when out of view.						*/
/*					Does not support attachments										*/
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
#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include <assert.h>
#include "EffectI.h"
#include "EffectC.h"
#include "Ram.h"
#include "Corona.h"
#include "Errorlog.h"


////////////////////////////////////////////////////////////////////////////////////////
//	Resource struct
////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	EffectResource	*ExternalResource;	// external resource list
	int				TypeID;				// effect type id

} CoronaResource;


////////////////////////////////////////////////////////////////////////////////////////
//	Interface setup
////////////////////////////////////////////////////////////////////////////////////////
static char *		Corona_GetName( void );
static void *		Corona_Create( EffectResource *Resource, int TypeID );
static void			Corona_Destroy( void ** ResourceToZap );
static void *		Corona_Add( CoronaResource *Resource, Corona *Data, Corona *AttachData );
static void			Corona_Remove( CoronaResource *Resource, Corona *Data );
static geBoolean	Corona_Process( CoronaResource *Resource, float TimeDelta, Corona *Data );
static geBoolean	Corona_Frame( CoronaResource *Resource, float TimeDelta );
static geBoolean	Corona_Modify( CoronaResource *Resource, Corona *Data, Corona *NewData, uint32 Flags );
static void			Corona_Pause( CoronaResource *Resource, Corona *Data, geBoolean Pause );
Effect_Interface	Corona_Interface =
{
	Corona_GetName,
	Corona_Create,
	Corona_Destroy,
	Corona_Add,
	Corona_Remove,
	Corona_Process,
	Corona_Frame,
	Corona_Modify,
	Corona_Pause,
	sizeof( Corona )
};



////////////////////////////////////////////////////////////////////////////////////////
//
//	Corona_GetName()
//
////////////////////////////////////////////////////////////////////////////////////////
static char * Corona_GetName(
	void )	// no parameters
{
	return "Corona";

} // Corona_GetName()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Corona_Create()
//
////////////////////////////////////////////////////////////////////////////////////////
static void * Corona_Create(
	EffectResource	*ExternalResource,	// external resources
	int				TypeID )			// effect type id
{

	// locals
	CoronaResource	*Resource;

	// ensure valid data
	assert( ExternalResource != NULL );
	assert( TypeID > 0 );

	// allocate the resource list struct
	Resource = geRam_AllocateClear( sizeof( *Resource ) );
	if ( Resource == NULL )
	{
		geErrorLog_AddString( GE_ERR_MEMORY_RESOURCE, "Corona_Create: failed to create CoronaResource struct.", NULL );
		return NULL;
	}

	// save passed resource list
	Resource->ExternalResource = ExternalResource;

	// save type id
	Resource->TypeID = TypeID;

	// all done
	return Resource;

} // Corona_Create()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Corona_Destroy()
//
////////////////////////////////////////////////////////////////////////////////////////
static void Corona_Destroy(
	void	**ResourceToZap )	// resource list 
{

	// locals
	CoronaResource	*Resource;

	// ensure valid data
	assert( ResourceToZap != NULL );
	assert( *ResourceToZap != NULL );

	// get resource list
	Resource = *ResourceToZap;

	// zap resource list
	geRam_Free( Resource );
	ResourceToZap = NULL;

} // Corona_Destroy()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Corona_Add()
//
////////////////////////////////////////////////////////////////////////////////////////
static void * Corona_Add(
	CoronaResource	*Resource,		// available resources
	Corona			*Data,			// data of effect
	Corona			*AttachData )	// data of effect to attach to
{

	// locals
	Corona		*NewData;
	geVec3d		Pos;
	geBoolean	Result;

	// ensure valid data
	assert( Resource != NULL );
	assert( Data != NULL );

	// fail if type ids don't match
	if ( Resource->TypeID != Data->TypeID )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Corona_Add: effect struct and effect type don't match.", NULL );
		return NULL;
	}

	// allocate Corona data
	NewData = geRam_Allocate( sizeof( *NewData ) );
	if ( NewData == NULL )
	{
		geErrorLog_AddString( GE_ERR_MEMORY_RESOURCE, "Corona_Add: failed to create Corona struct.", NULL );
		return NULL;
	}

	// copy passed data
	memcpy( NewData, Data, sizeof( *NewData ) );

	// fail if we have any bad data
	if( NewData->Texture == NULL )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Corona_Add: effect not added, Texture is bad.", NULL );
		return NULL;
	}
	if( NewData->FadeTime < 0.0f )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Corona_Add: effect not added, FadeTime is bad.", NULL );
		return NULL;
	}
	if ( NewData->MaxVisibleDistance <= 0.0f )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Corona_Add: effect not added, MaxVisibleDistance is bad.", NULL );
		return NULL;
	}
	if( NewData->MinRadius < 0.0f )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Corona_Add: effect not added, MinRadius is bad.", NULL );
		return NULL;
	}
	if ( NewData->MaxRadius < NewData->MinRadius )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Corona_Add: effect not added, MaxRadius is bad.", NULL );
		return NULL;
	}
	if ( NewData->MinRadiusDistance < 0.0f )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Corona_Add: effect not added, MinRadiusDistance is bad.", NULL );
		return NULL;
	}
	if ( ( NewData->MaxRadiusDistance <= 0.0f ) || ( NewData->MaxRadiusDistance < NewData->MinRadiusDistance ) )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Corona_Add: effect not added, MaxRadiusDistance is bad.", NULL );
		return NULL;
	}

	// setup defaultdata
	NewData->Vertex.u = NewData->Vertex.v = 0.0f;
	NewData->LastVisibleRadius = 0.0f;

	// calculate leaf value
	geVec3d_Set( &Pos, NewData->Vertex.X, NewData->Vertex.Y, NewData->Vertex.Z );
	Result = geWorld_GetLeaf( Resource->ExternalResource->World, &Pos, &( NewData->Leaf ) );
	assert( Result == GE_TRUE );

	// all done
	return NewData;

	// get rid of warnings
	AttachData;
	Resource;

} // Corona_Add()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Corona_Remove()
//
////////////////////////////////////////////////////////////////////////////////////////
static void Corona_Remove(
	CoronaResource	*Resource,	// available resources
	Corona			*Data )		// effect data
{

	// ensure valid data
	assert( Resource != NULL );
	assert( Data != NULL );

	// free effect data
	geRam_Free( Data );

	// get rid of warnings
	Resource;

} // Corona_Remove()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Corona_Process()
//
//	Perform processing on an indivual effect. A return of GE_FALSE means that the
//	effect needs to be removed.
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean Corona_Process(
	CoronaResource	*Resource,		// available resources
	float			TimeDelta,		// elapsed time
	Corona			*Data )			// effect data
{

	// locals
	const geXForm3d	*CameraXf;
	geVec3d			Pos;
	geVec3d			Delta;
	geFloat			DistanceToCorona;
	geBoolean		Visible;

	// ensure valid data
	assert( Resource != NULL );
	assert( TimeDelta > 0.0f );
	assert( Data != NULL );

	// get camera xform
	CameraXf = geCamera_GetWorldSpaceXForm( Resource->ExternalResource->Camera );
	assert( CameraXf != NULL );

	// determine distance to corona
	geVec3d_Set( &Pos, Data->Vertex.X, Data->Vertex.Y, Data->Vertex.Z );
	geVec3d_Subtract( &Pos, &( CameraXf->Translation ), &Delta);
	DistanceToCorona = geVec3d_Length( &Delta );

	// determine distance to corona
	Visible = EffectC_IsPointVisible(	Resource->ExternalResource->World,
										Resource->ExternalResource->Camera,
										&Pos,
										Data->Leaf,
										EFFECTC_CLIP_LEAF | EFFECTC_CLIP_LINEOFSIGHT | EFFECTC_CLIP_SEMICIRCLE );

	// process the corona if required
	if ( Data->Paused == GE_FALSE )
	{

		// set new radius
		if ( Visible )
		{

			// locals
			float	DesiredRadius;

			// determine desired radius
			if ( DistanceToCorona >= Data->MaxRadiusDistance )
			{
				DesiredRadius = Data->MaxRadius;
			}
			else if	( DistanceToCorona <= Data->MinRadiusDistance )
			{
				DesiredRadius = Data->MinRadius;
			}
			else
			{

				// locals
				geFloat	Slope;

				// determine radius
				Slope = ( Data->MaxRadius - Data->MinRadius ) / ( Data->MaxRadiusDistance - Data->MinRadiusDistance );
				DesiredRadius = Data->MinRadius + Slope * ( DistanceToCorona - Data->MinRadiusDistance );
			}

			// scale radius upwards
			if ( Data->FadeTime > 0.0f )
			{
				Data->LastVisibleRadius += ( ( TimeDelta * Data->MaxRadius ) / Data->FadeTime );
				if ( Data->LastVisibleRadius > DesiredRadius )
				{
					Data->LastVisibleRadius = DesiredRadius;
				}
			}
			else
			{
				Data->LastVisibleRadius = DesiredRadius;
			}
		}
		else if ( Data->LastVisibleRadius > 0.0f )
		{

			// scale radius down
			if ( Data->FadeTime > 0.0f )
			{
				Data->LastVisibleRadius -= ( ( TimeDelta * Data->MaxRadius ) / Data->FadeTime );
				if ( Data->LastVisibleRadius < 0.0f )
				{
					Data->LastVisibleRadius = 0.0f;
				}
			}
			else
			{
				Data->LastVisibleRadius = 0.0f;
			}
		}

		// update the art
		if ( Data->LastVisibleRadius > 0.0f )
		{
			geWorld_AddPolyOnce(	Resource->ExternalResource->World,
									&( Data->Vertex ),
									1,
									Data->Texture,
									GE_TEXTURED_POINT,
									GE_RENDER_DO_NOT_OCCLUDE_SELF,
									Data->LastVisibleRadius );
		}
	}

	// all done
	return GE_TRUE;

} // Corona_Process()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Corona_Frame()
//
//	Perform once-per-frame processing for all effects of this type.
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean Corona_Frame(
	CoronaResource	*Resource,		// available resources
	float			TimeDelta )		// elapsed time
{

	// all done
	return GE_TRUE;

	// get rid of warnings
	Resource;
	TimeDelta;

} // Corona_Frame()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Corona_Modify()
//
//	Adjust the effect.
//
////////////////////////////////////////////////////////////////////////////////////////
geBoolean Corona_Modify(
	CoronaResource	*Resource,	// available resources
	Corona			*Data,		// effect data
	Corona			*NewData,	// new data
	uint32			Flags )		// user flags
{

	// locals
	geBoolean	RecalculateLeaf = GE_FALSE;

	// ensure valid data
	assert( Resource != NULL );
	assert( Data != NULL );
	assert( NewData != NULL );

	// adjust location
	if ( Flags & CORONA_POS )
	{
		Data->Vertex.X = NewData->Vertex.X;
		Data->Vertex.Y = NewData->Vertex.Y;
		Data->Vertex.Z = NewData->Vertex.Z;
		RecalculateLeaf = GE_TRUE;
	}

	// recalculate leaf value if required
	if ( RecalculateLeaf == GE_TRUE )
	{

		// locals
		geVec3d		Pos;
		geBoolean	Result;

		// recalc leaf
		geVec3d_Set( &Pos, NewData->Vertex.X, NewData->Vertex.Y, NewData->Vertex.Z );
		Result = geWorld_GetLeaf( Resource->ExternalResource->World, &Pos, &( Data->Leaf ) );
		assert( Result == GE_TRUE );
	}

	// all done
	return GE_TRUE;

	// get rid of warnings
	Resource;

} // Corona_Modify()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Corona_Pause()
//
//	Pause/unpause effect.
//
////////////////////////////////////////////////////////////////////////////////////////
void Corona_Pause(
	CoronaResource	*Resource,	// available resources
	Corona			*Data,		// effect data
	geBoolean		Pause )		// new pause state
{

	// ensure valid data
	assert( Resource != NULL );
	assert( Data != NULL );

	// set pause flag
	Data->Paused = Pause;

	// get rid of warnings
	Resource;

} // Corona_Pause()
