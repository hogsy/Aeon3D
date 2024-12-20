/****************************************************************************************/
/*  Spray.c										                                        */
/*                                                                                      */
/*  Author: Peter Siamidis                                                              */
/*  Description:    Create a particle spray												*/
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
#include "EffectC.h"
#include "Ram.h"
#include "Errorlog.h"
#include "Spray.h"


////////////////////////////////////////////////////////////////////////////////////////
//	Resource struct
////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	EffectResource	*ExternalResource;	// external resource list
	int				TypeID;				// effect type id

} SprayResource;


////////////////////////////////////////////////////////////////////////////////////////
//	Interface setup
////////////////////////////////////////////////////////////////////////////////////////
static char *		Spray_GetName( void );
static void *		Spray_Create( EffectResource *Resource, int TypeID );
static void			Spray_Destroy( void ** ResourceToZap );
static void *		Spray_Add( SprayResource *Resource, Spray *Data, Spray *AttachData );
static void			Spray_Remove( SprayResource *Resource, Spray *Data );
static geBoolean	Spray_Process( SprayResource *Resource, float TimeDelta, Spray *Data );
static geBoolean	Spray_Frame( SprayResource *Resource, float TimeDelta );
static geBoolean	Spray_Modify( SprayResource *Resource, Spray *Data, Spray *NewData, uint32 Flags );
static void			Spray_Pause( SprayResource *Resource, Spray *Data, geBoolean Pause );
Effect_Interface	Spray_Interface =
{
	Spray_GetName,
	Spray_Create,
	Spray_Destroy,
	Spray_Add,
	Spray_Remove,
	Spray_Process,
	Spray_Frame,
	Spray_Modify,
	Spray_Pause,
	sizeof( Spray )
};



////////////////////////////////////////////////////////////////////////////////////////
//
//	Spray_GetName()
//
////////////////////////////////////////////////////////////////////////////////////////
static char * Spray_GetName(
	void )	// no parameters
{
	return "Spray";

} // Spray_GetName()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Spray_Create()
//
////////////////////////////////////////////////////////////////////////////////////////
static void * Spray_Create(
	EffectResource	*ExternalResource,	// external resources
	int				TypeID )			// effect type id
{

	// locals
	SprayResource	*Resource;

	// ensure valid data
	assert( ExternalResource != NULL );
	assert( TypeID > 0 );

	// allocate the resource list struct
	Resource = geRam_AllocateClear( sizeof( *Resource ) );
	if ( Resource == NULL )
	{
		geErrorLog_AddString( GE_ERR_MEMORY_RESOURCE, "Spray_Create: failed to create SprayResource struct.", NULL );
		return NULL;
	}

	// save passed resource list
	Resource->ExternalResource = ExternalResource;

	// save type id
	Resource->TypeID = TypeID;

	// all done
	return Resource;

} // Spray_Create()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Spray_Destroy()
//
////////////////////////////////////////////////////////////////////////////////////////
static void Spray_Destroy(
	void	**ResourceToZap )	// resource list 
{

	// locals
	SprayResource	*Resource;

	// ensure valid data
	assert( ResourceToZap != NULL );
	assert( *ResourceToZap != NULL );

	// get resource list
	Resource = *ResourceToZap;

	// zap resource list
	geRam_Free( Resource );
	ResourceToZap = NULL;

} // Spray_Destroy()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Spray_Add()
//
////////////////////////////////////////////////////////////////////////////////////////
static void * Spray_Add(
	SprayResource	*Resource,		// available resources
	Spray			*Data,			// data of effect
	Spray			*AttachData )	// data of effect to attach to
{

	// locals
	Spray		*NewData;
	geBoolean	Result;

	// ensure valid data
	assert( Resource != NULL );
	assert( Data != NULL );

	// fail if type ids don't match
	if ( Resource->TypeID != Data->TypeID )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Spray_Add: effect struct and effect type don't match.", NULL );
		return NULL;
	}

	// allocate Spray data
	NewData = geRam_Allocate( sizeof( *NewData ) );
	if ( NewData == NULL )
	{
		geErrorLog_AddString( GE_ERR_MEMORY_RESOURCE, "Spray_Add: failed to create Spray struct.", NULL );
		return NULL;
	}

	// copy passed data
	memcpy( NewData, Data, sizeof( *NewData ) );

	// fail if we have bad data
	if ( EffectC_IsColorGood( &( NewData->ColorMin ) ) == GE_FALSE )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Spray_Add: effect not added, ColorMin is bad.", NULL );
		return NULL;
	}
	if ( EffectC_IsColorGood( &( NewData->ColorMax ) ) == GE_FALSE )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Spray_Add: effect not added, ColorMax is bad.", NULL );
		return NULL;
	}
	if ( NewData->ColorMax.r < NewData->ColorMin.r )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Spray_Add: effect not added, ColorMax Red is < ColorMin Red.", NULL );
		return NULL;
	}
	if ( NewData->ColorMax.g < NewData->ColorMin.g )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Spray_Add: effect not added, ColorMax Green is < ColorMin Green.", NULL );
		return NULL;
	}
	if ( NewData->ColorMax.b < NewData->ColorMin.b )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Spray_Add: effect not added, ColorMax Blue is < ColorMin Blue.", NULL );
		return NULL;
	}
	if ( NewData->SprayLife < 0.0f )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Spray_Add: effect not added, SprayLife is negative.", NULL );
		return NULL;
	}
	if ( NewData->DistanceMin < 0.0f )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Spray_Add: effect not added, DistanceMin is negative.", NULL );
		return NULL;
	}
	if ( NewData->DistanceMax < NewData->DistanceMin )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Spray_Add: effect not added, DistanceMax is less than DistanceMin.", NULL );
		return NULL;
	}
	if ( NewData->Texture == NULL )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Spray_Add: effect not added, no texture provided.", NULL );
		return NULL;
	}
	if ( NewData->Rate <= 0.0f )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Spray_Add: effect not added, Rate is negative.", NULL );
		return NULL;
	}
	if ( NewData->MinScale <= 0.0f )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Spray_Add: effect not added, MinScale is <= 0.", NULL );
		return NULL;
	}
	if ( NewData->MaxScale < NewData->MinScale )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Spray_Add: effect not added, MaxScale is less than MinScale.", NULL );
		return NULL;
	}
	if ( NewData->MinSpeed < 0.0f )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Spray_Add: effect not added, MinSpeed is negative.", NULL );
		return NULL;
	}
	if ( NewData->MaxSpeed < NewData->MinSpeed )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Spray_Add: effect not added, MaxSpeed is less than MinSpeed.", NULL );
		return NULL;
	}
	if ( NewData->MinUnitLife <= 0.0f )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Spray_Add: effect not added, MinUnitLife is <= 0.", NULL );
		return NULL;
	}
	if( NewData->MaxUnitLife < NewData->MinUnitLife )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Spray_Add: effect not added, MaxUnitLife is less than MinUnitLife.", NULL );
		return NULL;
	}
	if ( NewData->SourceVariance < 0 )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Spray_Add: effect not added, SourceVariance is negative.", NULL );
		return NULL;
	}
	if( NewData->DestVariance < 0 )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Spray_Add: effect not added, DestVariance is negative.", NULL );
		return NULL;
	}

	// setup defaults
	NewData->TimeRemaining = 0.0f;
	NewData->PolyCount = 0.0f;

	// setup particle gravity
	if ( geVec3d_Length( &( NewData->Gravity ) ) > 0.0f )
	{
		NewData->ParticleGravity = &( NewData->Gravity );
	}
	else
	{
		NewData->ParticleGravity = NULL;
	}

	// save the transform
	EffectC_XFormFromVector( &( NewData->Source ), &( NewData->Dest ), &( NewData->Xf ) );

	// setup default vertex data
	NewData->Vertex.u = 0.0f;
	NewData->Vertex.v = 0.0f;
	NewData->Vertex.r = 255.0f;
	NewData->Vertex.g = 255.0f;
	NewData->Vertex.b = 255.0f;

	// calculate leaf value
	Result = geWorld_GetLeaf( Resource->ExternalResource->World, &( NewData->Source ), &( NewData->Leaf ) );
	assert( Result == GE_TRUE );

	// all done
	return NewData;

	// get rid of warnings
	AttachData;

} // Spray_Add()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Spray_Remove()
//
////////////////////////////////////////////////////////////////////////////////////////
static void Spray_Remove(
	SprayResource	*Resource,	// available resources
	Spray			*Data )		// effect data
{

	// ensure valid data
	assert( Resource != NULL );
	assert( Data != NULL );

	// remove particle anchor
	if ( Data->AnchorPoint != NULL )
	{
		assert( Resource->ExternalResource != NULL );
		assert( Resource->ExternalResource->PS != NULL );
		Particle_SystemRemoveAnchorPoint( Resource->ExternalResource->PS, Data->AnchorPoint );
	}

	// free effect data
	geRam_Free( Data );

	// get rid of warnings
	Resource;

} // Spray_Remove()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Spray_Process()
//
//	Perform processing on an indivual effect. A return of GE_FALSE means that the
//	effect needs to be removed.
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean Spray_Process(
	SprayResource	*Resource,		// available resources
	float			TimeDelta,		// elapsed time
	Spray			*Data )			// effect data
{

	// locals
	geVec3d			Velocity;
	geVec3d			Left, Up;
	geVec3d			Source, Dest;
	const geXForm3d	*CameraXf;
	float			Scale;
	float			UnitLife;
	float			Distance;
	float			Adjustment = 1.0f;
	float			NewPolyCount = 0.0f;

	// ensure valid data
	assert( Resource != NULL );
	assert( TimeDelta > 0.0f );
	assert( Data != NULL );

	// adjust spray life, killing it if required
	if ( Data->SprayLife > 0.0f )
	{
		Data->SprayLife -= TimeDelta;
		if ( Data->SprayLife <= 0.0f )
		{
			return GE_FALSE;
		}
	}

	// do nothing if its paused
	if ( Data->Paused == GE_TRUE )
	{
		return GE_TRUE;
	}

	// do nothing if it isn't visible
	if ( EffectC_IsPointVisible(	Resource->ExternalResource->World,
									Resource->ExternalResource->Camera,
									&( Data->Source ),
									Data->Leaf,
									EFFECTC_CLIP_LEAF | EFFECTC_CLIP_SEMICIRCLE ) == GE_FALSE )
	{
		return GE_TRUE;
	}

	// get camera xform
	CameraXf = geCamera_GetWorldSpaceXForm( Resource->ExternalResource->Camera );

	// perform level of detail processing if required
	if ( Data->DistanceMax > 0.0f )
	{

		// do nothing if its too far away
		Distance = geVec3d_DistanceBetween( &( Data->Source ), &( CameraXf->Translation ) );
		if ( Distance > Data->DistanceMax )
		{
			return GE_TRUE;
		}

		// determine polygon adjustment amount
		if ( ( Data->DistanceMin > 0.0f ) && ( Distance > Data->DistanceMin ) )
		{
			Adjustment = ( 1.0f - ( ( Distance - Data->DistanceMin ) / ( Data->DistanceMax - Data->DistanceMin ) ) );
		}
	}

	// determine how many polys need to be added taking level fo detail into account
	Data->TimeRemaining += TimeDelta;
	while ( Data->TimeRemaining >= Data->Rate )
	{
		Data->TimeRemaining -= Data->Rate;
		NewPolyCount += 1.0f;
	}
	assert( Adjustment >= 0.0f );
	assert( Adjustment <= 1.0f );
	NewPolyCount *= Adjustment;
	Data->PolyCount += NewPolyCount;

	// add new textures
	while ( Data->PolyCount > 0 )
	{

		// adjust poly remaining count
		Data->PolyCount -= 1.0f;

		// pick a source point
		if ( Data->SourceVariance > 0 )
		{
			geXForm3d_GetLeft( &( Data->Xf ), &Left );
			geXForm3d_GetUp( &( Data->Xf ), &Up );
			geVec3d_Scale( &Left, (float)Data->SourceVariance * EffectC_Frand( -1.0f, 1.0f ), &Left );
			geVec3d_Scale( &Up, (float)Data->SourceVariance * EffectC_Frand( -1.0f, 1.0f ), &Up );
			geVec3d_Add( &Left, &Up, &Source );
			geVec3d_Add( &( Data->Source ), &Source, &Source );
		}
		else
		{
			geVec3d_Copy( &( Data->Source ), &Source );
		}
		Data->Vertex.X = Source.X;
		Data->Vertex.Y = Source.Y;
		Data->Vertex.Z = Source.Z;

		// pick a destination point
		if ( Data->DestVariance > 0 )
		{
			geXForm3d_GetLeft( &( Data->Xf ), &Left );
			geXForm3d_GetUp( &( Data->Xf ), &Up );
			geVec3d_Scale( &Left, (float)Data->DestVariance * EffectC_Frand( -1.0f, 1.0f ), &Left );
			geVec3d_Scale( &Up, (float)Data->DestVariance * EffectC_Frand( -1.0f, 1.0f ), &Up );
			geVec3d_Add( &Left, &Up, &Dest );
			geVec3d_Add( &( Data->Dest ), &Dest, &Dest );
		}
		else
		{
			geVec3d_Copy( &( Data->Dest ), &Dest );
		}

		// set velocity
		if ( Data->MinSpeed > 0.0f )
		{
			geVec3d_Subtract( &Dest, &Source, &Velocity );
			geVec3d_Normalize( &Velocity );
			geVec3d_Scale( &Velocity, EffectC_Frand( Data->MinSpeed, Data->MaxSpeed ), &Velocity );
		}
		else
		{
			geVec3d_Set( &Velocity, 0.0f, 0.0f, 0.0f );
		}

		// set scale
		Scale = EffectC_Frand( Data->MinScale, Data->MaxScale );

		// set life
		UnitLife = EffectC_Frand( Data->MinUnitLife, Data->MaxUnitLife );

		// setup color
		Data->Vertex.r = EffectC_Frand( Data->ColorMin.r, Data->ColorMax.r );
		Data->Vertex.g = EffectC_Frand( Data->ColorMin.g, Data->ColorMax.g );
		Data->Vertex.b = EffectC_Frand( Data->ColorMin.b, Data->ColorMax.b );
		Data->Vertex.a = EffectC_Frand( Data->ColorMin.a, Data->ColorMax.a );

		// add the new particle
		Particle_SystemAddParticle(	Resource->ExternalResource->PS,
									Data->Texture,
									&( Data->Vertex ),
									Data->AnchorPoint,
									UnitLife,
									&Velocity,
									Scale,
									Data->ParticleGravity );
	}

	// all done
	return GE_TRUE;

} // Spray_Process()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Spray_Frame()
//
//	Perform once-per-frame processing for all effects of this type.
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean Spray_Frame(
	SprayResource	*Resource,		// available resources
	float			TimeDelta )		// elapsed time
{

	// all done
	return GE_TRUE;

	// get rid of warnings
	Resource;
	TimeDelta;

} // Spray_Frame()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Spray_Modify()
//
//	Adjust the effect.
//
////////////////////////////////////////////////////////////////////////////////////////
geBoolean Spray_Modify(
	SprayResource	*Resource,	// available resources
	Spray			*Data,		// effect data
	Spray			*NewData,	// new data
	uint32			Flags )		// user flags
{

	// locals
	geBoolean	Result;
	geBoolean	RecalculateLeaf = GE_FALSE;

	// ensure valid data
	assert( Resource != NULL );
	assert( Data != NULL );
	assert( NewData != NULL );

	// adjust source and dest together
	if ( Flags & SPRAY_FOLLOWTAIL )
	{
		geVec3d_Copy( &( Data->Source ), &( Data->Dest ) );
		geVec3d_Copy( &( NewData->Source ), &( Data->Source ) );
		RecalculateLeaf = GE_TRUE;
	}

	// adjust source
	if ( Flags & SPRAY_SOURCE )
	{
		geVec3d_Copy( &( NewData->Source ), &( Data->Source ) );
		RecalculateLeaf = GE_TRUE;
	}

	// adjust source
	if ( Flags & SPRAY_DEST )
	{
		geVec3d_Copy( &( NewData->Dest ), &( Data->Dest ) );
		RecalculateLeaf = GE_TRUE;
	}

	// calculate leaf value
 	if ( RecalculateLeaf == GE_TRUE )
	{
		EffectC_XFormFromVector( &( Data->Source ), &( Data->Dest ), &( Data->Xf ) );
		Result = geWorld_GetLeaf( Resource->ExternalResource->World, &( Data->Source ), &( Data->Leaf ) );
		assert( Result == GE_TRUE );
	}

	// all done
	return GE_TRUE;

} // Spray_Modify()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Spray_Pause()
//
//	Pause/unpause effect.
//
////////////////////////////////////////////////////////////////////////////////////////
void Spray_Pause(
	SprayResource	*Resource,	// available resources
	Spray			*Data,		// effect data
	geBoolean		Pause )		// new pause state
{

	// ensure valid data
	assert( Resource != NULL );
	assert( Data != NULL );

	// set pause flag
	Data->Paused = Pause;

	// get rid of warnings
	Resource;

} // Spray_Pause()
