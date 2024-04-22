/****************************************************************************************/
/*  Glow.c	                                                                            */
/*                                                                                      */
/*  Author: Peter Siamidis                                                              */
/*  Description:    Creates a light which follows a passed vector.						*/
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
#include <memory.h>
#include <assert.h>
#include "EffectI.h"
#include "Ram.h"
#include "Glow.h"
#include "EffectC.h"
#include "Errorlog.h"


////////////////////////////////////////////////////////////////////////////////////////
//	Resource struct
////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	EffectResource	*ExternalResource;	// external resource list
	int				TypeID;				// effect type id

} GlowResource;


////////////////////////////////////////////////////////////////////////////////////////
//	Interface setup
////////////////////////////////////////////////////////////////////////////////////////
static char *		Glow_GetName( void );
static void *		Glow_Create( EffectResource *Resource, int TypeID );
static void			Glow_Destroy( void ** ResourceToZap );
static void *		Glow_Add( GlowResource *Resource, Glow *Data, Glow *AttachData );
static void			Glow_Remove( GlowResource *Resource, Glow *Data );
static geBoolean	Glow_Process( GlowResource *Resource, float TimeDelta, Glow *Data );
static geBoolean	Glow_Frame( GlowResource *Resource, float TimeDelta );
static geBoolean	Glow_Modify( GlowResource *Resource, Glow *Data, Glow *NewData, uint32 Flags );
static void			Glow_Pause( GlowResource *Resource, Glow *Data, geBoolean Pause );
Effect_Interface	Glow_Interface =
{
	Glow_GetName,
	Glow_Create,
	Glow_Destroy,
	Glow_Add,
	Glow_Remove,
	Glow_Process,
	Glow_Frame,
	Glow_Modify,
	Glow_Pause,
	sizeof( Glow )
};



////////////////////////////////////////////////////////////////////////////////////////
//
//	Glow_GetName()
//
////////////////////////////////////////////////////////////////////////////////////////
static char * Glow_GetName(
	void )	// no parameters
{
	return "Glow";

} // Glow_GetName()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Glow_Create()
//
////////////////////////////////////////////////////////////////////////////////////////
static void * Glow_Create(
	EffectResource	*ExternalResource,	// external resources
	int				TypeID )			// effect type id
{

	// locals
	GlowResource	*Resource;

	// ensure valid data
	assert( ExternalResource != NULL );
	assert( TypeID > 0 );

	// allocate the resource list struct
	Resource = geRam_AllocateClear( sizeof( *Resource ) );
	if ( Resource == NULL )
	{
		geErrorLog_AddString( GE_ERR_MEMORY_RESOURCE, "Glow_Create: failed to create GlowResource struct.", NULL );
		return NULL;
	}

	// save passed resource list
	Resource->ExternalResource = ExternalResource;

	// save type id
	Resource->TypeID = TypeID;

	// all done
	return Resource;

} // Glow_Create()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Glow_Destroy()
//
////////////////////////////////////////////////////////////////////////////////////////
static void Glow_Destroy(
	void	**ResourceToZap )	// resource list 
{

	// locals
	GlowResource	*Resource;

	// ensure valid data
	assert( ResourceToZap != NULL );
	assert( *ResourceToZap != NULL );

	// get resource list
	Resource = *ResourceToZap;

	// zap resource list
	geRam_Free( Resource );
	ResourceToZap = NULL;

} // Glow_Destroy()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Glow_Add()
//
////////////////////////////////////////////////////////////////////////////////////////
static void * Glow_Add(
	GlowResource	*Resource,		// available resources
	Glow			*Data,			// data of effect
	Glow			*AttachData )	// data of effect to attach to
{

	// locals
	Glow		*NewData;
	geBoolean	Result;

	// ensure valid data
	assert( Resource != NULL );
	assert( Data != NULL );

	// fail if type ids don't match
	if ( Resource->TypeID != Data->TypeID )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Glow_Add: effect struct and effect type don't match.", NULL );
		return NULL;
	}

	// allocate glow data
	NewData = geRam_Allocate( sizeof( *NewData ) );
	if ( NewData == NULL )
	{
		geErrorLog_AddString( GE_ERR_MEMORY_RESOURCE, "Glow_Add: failed to create Glow struct.", NULL );
		return NULL;
	}

	// copy passed data
	memcpy( NewData, Data, sizeof( *NewData ) );

	// fail if we have any bad data
	if ( EffectC_IsColorGood( &( NewData->ColorMin ) ) == GE_FALSE )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Glow_Add: effect not added, ColorMin is bad.", NULL );
		return NULL;
	}
	if ( EffectC_IsColorGood( &( NewData->ColorMax ) ) == GE_FALSE )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Glow_Add: effect not added, ColorMax is bad.", NULL );
		return NULL;
	}
	if ( NewData->RadiusMin < 0.0f )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Glow_Add: effect not added, RadiusMin is negative.", NULL );
		return NULL;
	}
	if ( ( NewData->RadiusMax <= 0.0f ) || ( NewData->RadiusMax < NewData->RadiusMin ) )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Glow_Add: effect not added, RadiusMax value is bad.", NULL );
		return NULL;
	}
	if ( ( NewData->Intensity < 0.0f ) || ( NewData->Intensity > 1.0f ) )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Glow_Add: effect not added, Intensity value is bad.", NULL );
		return NULL;
	}

	// calculate leaf value
	Result = geWorld_GetLeaf( Resource->ExternalResource->World, &( NewData->Pos ), &( NewData->Leaf ) );
	assert( Result == GE_TRUE );

	// all done
	return NewData;

	// get rid of warnings
	AttachData;

} // Glow_Add()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Glow_Remove()
//
////////////////////////////////////////////////////////////////////////////////////////
static void Glow_Remove(
	GlowResource	*Resource,	// available resources
	Glow			*Data )		// effect data
{

	// ensure valid data
	assert( Resource != NULL );
	assert( Data != NULL );

	// remove its light
	if ( Data->Light != NULL )
	{
		geWorld_RemoveLight( Resource->ExternalResource->World, Data->Light );
		Data->Light = NULL;
	}

	// free effect data
	geRam_Free( Data );

} // Glow_Remove()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Glow_Process()
//
//	Perform processing on an indivual effect. A return of GE_FALSE means that the
//	effect needs to be removed.
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean Glow_Process(
	GlowResource	*Resource,		// available resources
	float			TimeDelta,		// elapsed time
	Glow			*Data )			// effect data
{

	// locals
	float	Radius;
	GE_RGBA	Color;

	// ensure valid data
	assert( Resource != NULL );
	assert( TimeDelta > 0.0f );
	assert( Data != NULL );

	// do nothing if it isn't visible
	if ( Data->DoNotClip == GE_FALSE )
	{
		if ( EffectC_IsPointVisible(	Resource->ExternalResource->World,
										Resource->ExternalResource->Camera,
										&( Data->Pos ),
										Data->Leaf,
										EFFECTC_CLIP_LEAF | EFFECTC_CLIP_LINEOFSIGHT | EFFECTC_CLIP_SEMICIRCLE ) == GE_FALSE )
		{
			if ( Data->Light != NULL )
			{
				geWorld_RemoveLight( Resource->ExternalResource->World, Data->Light );
				Data->Light = NULL;
			}
			return GE_TRUE;
		}
	}

	// recreate the light if required
	if ( Data->Light == NULL )
	{
		Data->Light = geWorld_AddLight( Resource->ExternalResource->World );
		if ( Data->Light == NULL )
		{
			geErrorLog_AddString( GE_ERR_SYSTEM_RESOURCE, "Glow_Process: failed to recreate light.", NULL );
			return GE_TRUE;
		}
	}

	// set color
	assert( Data->ColorMin.r >= -255.0f );
	assert( Data->ColorMax.r <= 255.0f );
	assert( Data->ColorMin.r <= Data->ColorMax.r );
	Color.r = EffectC_Frand( Data->ColorMin.r, Data->ColorMax.r ) * Data->Intensity;
	assert( Data->ColorMin.g >= -255.0f );
	assert( Data->ColorMax.g <= 255.0f );
	assert( Data->ColorMin.g <= Data->ColorMax.g );
	Color.g = EffectC_Frand( Data->ColorMin.g, Data->ColorMax.g ) * Data->Intensity;
	assert( Data->ColorMin.b >= -255.0f );
	assert( Data->ColorMax.b <= 255.0f );
	assert( Data->ColorMin.b <= Data->ColorMax.b );
	Color.b = EffectC_Frand( Data->ColorMin.b, Data->ColorMax.b ) * Data->Intensity;
	assert( Data->ColorMin.a >= -255.0f );
	assert( Data->ColorMax.a <= 255.0f );
	assert( Data->ColorMin.a <= Data->ColorMax.a );
	Color.a = EffectC_Frand( Data->ColorMin.a, Data->ColorMax.a ) * Data->Intensity;

	// set radius
	assert( Data->RadiusMin >= 0.0f );
	assert( Data->RadiusMax > 0.0f );
	assert( Data->RadiusMin <= Data->RadiusMax );
	Radius = EffectC_Frand( Data->RadiusMin, Data->RadiusMax );

	// adjust the lights parameters
	if ( geWorld_SetLightAttributes(	Resource->ExternalResource->World,
										Data->Light,
										&( Data->Pos ),
										&Color,
										Radius,
										Data->CastShadows ) == GE_FALSE )
	{
		geErrorLog_AddString( GE_ERR_SYSTEM_RESOURCE, "Glow_Process: failed to modify light.", NULL );
	}

	// all done
	return GE_TRUE;

	// get rid of warnings
	TimeDelta;

} // Glow_Process()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Glow_Frame()
//
//	Perform once-per-frame processing for all effects of this type.
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean Glow_Frame(
	GlowResource	*Resource,		// available resources
	float			TimeDelta )		// elapsed time
{

	// all done
	return GE_TRUE;

	// get rid of warnings
	Resource;
	TimeDelta;

} // Glow_Frame()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Glow_Modify()
//
//	Adjust the effect.
//
////////////////////////////////////////////////////////////////////////////////////////
geBoolean Glow_Modify(
	GlowResource	*Resource,	// available resources
	Glow			*Data,		// effect data
	Glow			*NewData,	// new data
	uint32			Flags )		// user flags
{

	// locals
	geBoolean	Result;
	geBoolean	RecalculateLeaf = GE_FALSE;

	// ensure valid data
	assert( Resource != NULL );
	assert( Data != NULL );
	assert( NewData != NULL );

	// adjust the source
	if ( Flags & GLOW_POS )
	{
		geVec3d_Copy( &( NewData->Pos ), &( Data->Pos ) );
		RecalculateLeaf = GE_TRUE;
	}

	// adjust the min radius
	if ( Flags & GLOW_RADIUSMIN )
	{
		if ( ( NewData->RadiusMin < 0.0f ) || ( NewData->RadiusMin > NewData->RadiusMax ) )
		{
			geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Glow_Modify: light min radius not modified, data is bad.", NULL );
			return GE_FALSE;
		}
		Data->RadiusMin = NewData->RadiusMin;
		if ( Data->RadiusMin > Data->RadiusMax )
		{
			Data->RadiusMax = Data->RadiusMin;
		}
	}

	// adjust the max radius
	if ( Flags & GLOW_RADIUSMAX )
	{
		if ( ( NewData->RadiusMax <= 0.0f ) || ( NewData->RadiusMax < NewData->RadiusMin ) )
		{
			geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Glow_Modify: light max radius not modified, data is bad.", NULL );
			return GE_FALSE;
		}
		Data->RadiusMax = NewData->RadiusMax;
		if ( Data->RadiusMax < Data->RadiusMin )
		{
			Data->RadiusMin = Data->RadiusMax;
		}
	}

	// adjust intensity
	if ( Flags & GLOW_INTENSITY )
	{
		if ( ( NewData->Intensity < 0.0f ) || ( NewData->Intensity > 1.0f ) )
		{
			geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Glow_Modify: light intensity not modified, data is bad.", NULL );
			return GE_FALSE;
		}
		Data->Intensity = NewData->Intensity;
	}

	// calculate leaf value
	if ( RecalculateLeaf == GE_TRUE )
	{
		Result = geWorld_GetLeaf( Resource->ExternalResource->World, &( Data->Pos ), &( Data->Leaf ) );
		assert( Result == GE_TRUE );
	}

	// all done
	return GE_TRUE;

} // Glow_Modify()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Glow_Pause()
//
//	Pause/unpause effect.
//
////////////////////////////////////////////////////////////////////////////////////////
void Glow_Pause(
	GlowResource	*Resource,	// available resources
	Glow			*Data,		// effect data
	geBoolean		Pause )		// new pause state
{

	// ensure valid data
	assert( Resource != NULL );
	assert( Data != NULL );

	// remove its light if required
	if ( Pause == GE_TRUE )
	{
		if ( Data->Light != NULL )
		{
			geWorld_RemoveLight( Resource->ExternalResource->World, Data->Light );
			Data->Light = NULL;
		}
	}

} // Glow_Pause()
