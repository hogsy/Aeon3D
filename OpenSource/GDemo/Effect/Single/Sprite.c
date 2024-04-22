/****************************************************************************************/
/*  Sprite.c									                                        */
/*                                                                                      */
/*  Author: Peter Siamidis                                                              */
/*  Description:    Sprite effect - Cycles through a list of sprites					*/
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
#include <stdlib.h>
#include <assert.h>
#include "EffectI.h"
#include "EffectC.h"
#include "Ram.h"
#include "Quatern.h"
#include "Errorlog.h"
#include "Sprite.h"


////////////////////////////////////////////////////////////////////////////////////////
//	Resource struct
////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	EffectResource	*ExternalResource;	// external resource list
	int				TypeID;				// effect type id

} SpriteResource;


////////////////////////////////////////////////////////////////////////////////////////
//	Interface setup
////////////////////////////////////////////////////////////////////////////////////////
static char *		Sprite_GetName( void );
static void *		Sprite_Create( EffectResource *Resource, int TypeID );
static void			Sprite_Destroy( void ** ResourceToZap );
static void *		Sprite_Add( SpriteResource *Resource, Sprite *Data, Sprite *AttachData );
static void			Sprite_Remove( SpriteResource *Resource, Sprite *Data );
static geBoolean	Sprite_Process( SpriteResource *Resource, float TimeDelta, Sprite *Data );
static geBoolean	Sprite_Frame( SpriteResource *Resource, float TimeDelta );
static geBoolean	Sprite_Modify( SpriteResource *Resource, Sprite *Data, Sprite *NewData, uint32 Flags );
static void			Sprite_Pause( SpriteResource *Resource, Sprite *Data, geBoolean Pause );
Effect_Interface	Sprite_Interface =
{
	Sprite_GetName,
	Sprite_Create,
	Sprite_Destroy,
	Sprite_Add,
	Sprite_Remove,
	Sprite_Process,
	Sprite_Frame,
	Sprite_Modify,
	Sprite_Pause,
	sizeof( Sprite )
};



////////////////////////////////////////////////////////////////////////////////////////
//
//	Sprite_GetName()
//
////////////////////////////////////////////////////////////////////////////////////////
static char * Sprite_GetName(
	void )	// no parameters
{
	return "Sprite";

} // Sprite_GetName()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Sprite_Create()
//
////////////////////////////////////////////////////////////////////////////////////////
static void * Sprite_Create(
	EffectResource	*ExternalResource,	// external resources
	int				TypeID )			// effect type id
{

	// locals
	SpriteResource	*Resource;

	// ensure valid data
	assert( ExternalResource != NULL );
	assert( TypeID > 0 );

	// allocate the resource list struct
	Resource = geRam_AllocateClear( sizeof( *Resource ) );
	if ( Resource == NULL )
	{
		geErrorLog_AddString( GE_ERR_MEMORY_RESOURCE, "Sprite_Create: failed to create SpriteResource struct.", NULL );
		return NULL;
	}

	// save passed resource list
	Resource->ExternalResource = ExternalResource;

	// save type id
	Resource->TypeID = TypeID;

	// all done
	return Resource;

} // Sprite_Create()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Sprite_Destroy()
//
////////////////////////////////////////////////////////////////////////////////////////
static void Sprite_Destroy(
	void	**ResourceToZap )	// resource list 
{

	// locals
	SpriteResource	*Resource;

	// ensure valid data
	assert( ResourceToZap != NULL );
	assert( *ResourceToZap != NULL );

	// get resource list
	Resource = *ResourceToZap;

	// zap resource list
	geRam_Free( Resource );
	ResourceToZap = NULL;

} // Sprite_Destroy()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Sprite_Add()
//
////////////////////////////////////////////////////////////////////////////////////////
static void * Sprite_Add(
	SpriteResource	*Resource,		// available resources
	Sprite			*Data,			// data of effect
	Sprite			*AttachData )	// data of effect to attach to
{

	// locals
	Sprite		*NewData;
	int			i;
	geBoolean	Result;

	// ensure valid data
	assert( Resource != NULL );
	assert( Data != NULL );

	// fail if type ids don't match
	if ( Resource->TypeID != Data->TypeID )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Sprite_Add: effect struct and effect type don't match.", NULL );
		return NULL;
	}

	// allocate sprite data
	NewData = geRam_Allocate( sizeof( *NewData ) );
	if ( NewData == NULL )
	{
		geErrorLog_AddString( GE_ERR_MEMORY_RESOURCE, "Sprite_Add: failed to create Sprite struct.", NULL );
		return NULL;
	}

	// copy passed data
	memcpy( NewData, Data, sizeof( *NewData ) );

	// make sure textures are valid
	if ( NewData->TotalTextures <= 0 )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Sprite_Add: effect not added, TotalTextures number is bad.", NULL );
		return NULL;
	}
	if ( NewData->Texture == NULL )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Sprite_Add: effect not added, texture list is bad.", NULL );
		return NULL;
	}
	for ( i = 0; i < NewData->TotalTextures; i++ )
	{
		if ( NewData->Texture[i] == NULL )
		{
			geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Sprite_Add: effect not added, texture list is bad.", NULL );
			return NULL;
		}
	}

	// fail if we have any bad data
	if ( EffectC_IsColorGood( &( NewData->Color ) ) == GE_FALSE )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Sprite_Add: effect not added, Color is bad.", NULL );
		return NULL;
	}
	if ( NewData->Scale <= 0.0f )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Sprite_Add: effect not added, Scale is <= 0.", NULL );
		return NULL;
	}
	if( NewData->TextureRate < 0.0f )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Sprite_Add: effect not added, TextureRate is negative.", NULL );
		return NULL;
	}
	if ( NewData->ScaleRate < 0.0f )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Sprite_Add: effect not added, ScaleRate is negative.", NULL );
		return NULL;
	}
	if ( NewData->AlphaRate < 0.0f )
	{
		geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Sprite_Add: effect not added, AlphaRate is negative.", NULL );
		return NULL;
	}

	// setup defaults
	NewData->Rotation = 0.0f;
	for ( i = 0; i < 4; i++ )
	{
		NewData->Vertex[i].r = NewData->Color.r;
		NewData->Vertex[i].g = NewData->Color.g;
		NewData->Vertex[i].b = NewData->Color.b;
		NewData->Vertex[i].a = NewData->Color.a;
	}
	NewData->Vertex[0].u = 0.0f;
	NewData->Vertex[0].v = 0.0f;
	NewData->Vertex[1].u = 1.0f;
	NewData->Vertex[1].v = 0.0f;
	NewData->Vertex[2].u = 1.0f;
	NewData->Vertex[2].v = 1.0f;
	NewData->Vertex[3].u = 0.0f;
	NewData->Vertex[3].v = 1.0f;
	NewData->CurrentTexture = 0;
	NewData->ElapsedTime = 0.0f;
	NewData->Direction = 1;

	// calculate leaf value
	Result = geWorld_GetLeaf( Resource->ExternalResource->World, &( NewData->Pos ), &( NewData->Leaf ) );
	assert( Result == GE_TRUE );

	// all done
	return NewData;

	// get rid of warnings
	AttachData;

} // Sprite_Add()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Sprite_Remove()
//
////////////////////////////////////////////////////////////////////////////////////////
static void Sprite_Remove(
	SpriteResource	*Resource,	// available resources
	Sprite			*Data )		// effect data
{

	// ensure valid data
	assert( Resource != NULL );
	assert( Data != NULL );

	// free effect data
	geRam_Free( Data );

	// get rid of warnings
	Resource;

} // Sprite_Remove()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Sprite_Process()
//
//	Perform processing on an indivual effect. A return of GE_FALSE means that the
//	effect needs to be removed.
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean Sprite_Process(
	SpriteResource	*Resource,		// available resources
	float			TimeDelta,		// elapsed time
	Sprite			*Data )			// effect data
{

	// locals
	int	i;

	// ensure valid data
	assert( Resource != NULL );
	assert( TimeDelta > 0.0f );
	assert( Data != NULL );

	// do nothing else if its paused
	if ( Data->Paused == GE_TRUE )
	{
		return GE_TRUE;
	}

	// adjust scale
	if ( Data->ScaleRate > 0.0f )
	{

		// eliminate the effect if the scale has reached zero
		Data->Scale -= ( Data->ScaleRate * TimeDelta );
		if ( Data->Scale <= 0.0f )
		{
			Data->Scale = 0.0f;
			return GE_FALSE;
		}
	}

	// adjust alpha
	if ( Data->AlphaRate > 0.0f )
	{

		// eliminate the effect if the alpha has reached zero
		Data->Color.a -= ( Data->AlphaRate * TimeDelta );
		if ( Data->Color.a <= 0.0f )
		{
			Data->Color.a = 0.0f;
			return GE_FALSE;
		}

		// adjust all verts with new alpha
		for ( i = 0; i < 4; i++ )
		{
			Data->Vertex[i].a = Data->Color.a;
		}
	}

	// do nothing else if it isn't visible
	if ( EffectC_IsPointVisible(	Resource->ExternalResource->World,
									Resource->ExternalResource->Camera,
									&( Data->Pos ),
									Data->Leaf,
									EFFECTC_CLIP_LEAF | EFFECTC_CLIP_SEMICIRCLE ) == GE_FALSE )
	{
		return GE_TRUE;
	}

	// adjust art
	if ( Data->TotalTextures > 1 )
	{
		assert( Data->Style != SPRITE_CYCLE_NONE );
		Data->ElapsedTime += TimeDelta;
		assert( Data->TextureRate > 0.0f );
		while ( Data->ElapsedTime > Data->TextureRate )
		{

			// adjust elapsed time counter
			Data->ElapsedTime -= Data->TextureRate;

			// pick the next nexture based on the cycle style
			if ( Data->Style == SPRITE_CYCLE_RANDOM )
			{
				Data->CurrentTexture = ( rand() % Data->TotalTextures );
			}
			else
			{

				// pick new texture number
				Data->CurrentTexture += Data->Direction;

				// adjust texture number if it has exceeded limits
				if (	( Data->CurrentTexture < 0 ) ||
						( Data->CurrentTexture >= Data->TotalTextures ) )
				{
					if ( Data->Style == SPRITE_CYCLE_REVERSE )
					{
						Data->Direction = -Data->Direction;
						Data->CurrentTexture += Data->Direction;
						Data->CurrentTexture += Data->Direction;
					}
					else if ( Data->Style == SPRITE_CYCLE_RESET )
					{
						Data->CurrentTexture = 0;
					}
					else
					{
						return GE_FALSE;
					}
				}
			}
		}
	}
	assert( Data->CurrentTexture >= 0 );
	assert( Data->CurrentTexture < Data->TotalTextures );

	// adjust current rotation amount
	if ( Data->RotationRate != 0.0f )
	{
		Data->Rotation += ( Data->RotationRate * TimeDelta );
		if ( Data->Rotation > GE_PI )
		{
			Data->Rotation = -GE_PI;
		}
		else if ( Data->Rotation < -GE_PI )
		{
			Data->Rotation = GE_PI;
		}
	}

	// if there is no rotation, then process the sprite this way
	if ( Data->Rotation == 0.0f )
	{

		// setup vert
		Data->Vertex[0].X = Data->Pos.X;
		Data->Vertex[0].Y = Data->Pos.Y;
		Data->Vertex[0].Z = Data->Pos.Z;

		// update the art
		geWorld_AddPolyOnce( Resource->ExternalResource->World, Data->Vertex, 1, Data->Texture[Data->CurrentTexture], GE_TEXTURED_POINT, GE_RENDER_DEPTH_SORT_BF, Data->Scale );
	}
	// ...otherwise process it this way
	else
	{

		// locals
		const geXForm3d	*CameraXf;
		geXForm3d		NewCameraXf;
		geVec3d			Left, Up, In;
		geQuaternion	Quat;
		float			HalfWidth, HalfHeight;

		// determine half width and half height
		HalfWidth = geBitmap_Width( Data->Texture[Data->CurrentTexture] ) * Data->Scale * 0.5f;
		HalfHeight = geBitmap_Height( Data->Texture[Data->CurrentTexture] ) * Data->Scale * 0.5f;

		// get left and up vectors from camera transform
		assert( Resource->ExternalResource->Camera != NULL );
		CameraXf = geCamera_GetWorldSpaceXForm( Resource->ExternalResource->Camera );
		geXForm3d_GetIn( CameraXf, &In );
		geQuaternion_SetFromAxisAngle( &Quat, &In, Data->Rotation );
		geQuaternion_ToMatrix( &Quat, &NewCameraXf );
		geXForm3d_Multiply( &NewCameraXf, CameraXf, &NewCameraXf );
		geXForm3d_GetLeft( &NewCameraXf, &Left );
		geXForm3d_GetUp( &NewCameraXf, &Up );
		geVec3d_Scale( &Left, HalfWidth, &Left );
		geVec3d_Scale( &Up, HalfHeight, &Up );

		// setup verticies
		Data->Vertex[0].X = Data->Pos.X + Left.X + Up.X;
		Data->Vertex[0].Y = Data->Pos.Y + Left.Y + Up.Y;
		Data->Vertex[0].Z = Data->Pos.Z + Left.Z + Up.Z;
		Data->Vertex[1].X = Data->Pos.X - Left.X + Up.X;
		Data->Vertex[1].Y = Data->Pos.Y - Left.Y + Up.Y;
		Data->Vertex[1].Z = Data->Pos.Z - Left.Z + Up.Z;
		Data->Vertex[2].X = Data->Pos.X - Left.X - Up.X;
		Data->Vertex[2].Y = Data->Pos.Y - Left.Y - Up.Y;
		Data->Vertex[2].Z = Data->Pos.Z - Left.Z - Up.Z;
		Data->Vertex[3].X = Data->Pos.X + Left.X - Up.X;
		Data->Vertex[3].Y = Data->Pos.Y + Left.Y - Up.Y;
		Data->Vertex[3].Z = Data->Pos.Z + Left.Z - Up.Z;

		// update the art
		geWorld_AddPolyOnce( Resource->ExternalResource->World, Data->Vertex, 4, Data->Texture[Data->CurrentTexture], GE_TEXTURED_POLY, GE_RENDER_DEPTH_SORT_BF, 1.0f );
	}

	// all done
	return GE_TRUE;

} // Sprite_Process()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Sprite_Frame()
//
//	Perform once-per-frame processing for all effects of this type.
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean Sprite_Frame(
	SpriteResource	*Resource,		// available resources
	float			TimeDelta )		// elapsed time
{

	// ensure valid data
	assert( Resource != NULL );
	assert( TimeDelta > 0.0f );

	// all done
	return GE_TRUE;

	// get rid of warnings
	Resource;
	TimeDelta;

} // Sprite_Frame()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Sprite_Modify()
//
//	Adjust the effect.
//
////////////////////////////////////////////////////////////////////////////////////////
geBoolean Sprite_Modify(
	SpriteResource	*Resource,	// available resources
	Sprite			*Data,		// effect data
	Sprite			*NewData,	// new data
	uint32			Flags )		// user flags
{

	// locals
	geBoolean	Result;
	geBoolean	RecalculateLeaf = GE_FALSE;

	// ensure valid data
	assert( Resource != NULL );
	assert( Data != NULL );
	assert( NewData != NULL );

	// adjust location
	if ( Flags & SPRITE_POS )
	{
		geVec3d_Copy( &( NewData->Pos ), &( Data->Pos ) );
		RecalculateLeaf = GE_TRUE;
	}

	// adjust scale
	if ( Flags & SPRITE_SCALE )
	{
		if ( NewData->Scale <= 0.0f )
		{
			geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Sprite_Modify: effect not modified, scale is bad.", NULL );
			return GE_FALSE;
		}
		Data->Scale = NewData->Scale;
	}

	// adjust rotation
	if ( Flags & SPRITE_ROTATION )
	{
		Data->Rotation = NewData->Rotation;
	}

	// adjust color
	if ( Flags & SPRITE_COLOR )
	{

		// locals
		int	i;

		// fail if a bad color was provided
		if ( ( NewData->Color.a < 0.0f ) || ( NewData->Color.a > 255.0f ) )
		{
			geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Sprite_Modify: effect not modified, alpha value is bad.", NULL );
			return GE_FALSE;
		}
		if ( ( NewData->Color.r < 0.0f ) || ( NewData->Color.r > 255.0f ) )
		{
			geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Sprite_Modify: effect not modified, red value is bad.", NULL );
			return GE_FALSE;
		}
		if ( ( NewData->Color.g < 0.0f ) || ( NewData->Color.g > 255.0f ) )
		{
			geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Sprite_Modify: effect not modified, green value is bad.", NULL );
			return GE_FALSE;
		}
		if ( ( NewData->Color.b < 0.0f ) || ( NewData->Color.b > 255.0f ) )
		{
			geErrorLog_AddString( GE_ERR_INVALID_PARMS, "Sprite_Modify: effect not modified, blue value is bad.", NULL );
			return GE_FALSE;
		}

		// save new color value
		Data->Color.r = NewData->Color.r;
		Data->Color.g = NewData->Color.g;
		Data->Color.b = NewData->Color.b;
		Data->Color.a = NewData->Color.a;

		// apply it to all verts
		for ( i = 0; i < 4; i++ )
		{
			Data->Vertex[i].r = NewData->Color.r;
			Data->Vertex[i].g = NewData->Color.g;
			Data->Vertex[i].b = NewData->Color.b;
			Data->Vertex[i].a = NewData->Color.a;
		}
	}

	// recalculate leaf value if required
	if ( RecalculateLeaf == GE_TRUE )
	{
		Result = geWorld_GetLeaf( Resource->ExternalResource->World, &( Data->Pos ), &( Data->Leaf ) );
		assert( Result == GE_TRUE );
	}

	// all done
	return GE_TRUE;

} // Sprite_Modify()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Sprite_Pause()
//
//	Pause/unpause effect.
//
////////////////////////////////////////////////////////////////////////////////////////
void Sprite_Pause(
	SpriteResource	*Resource,	// available resources
	Sprite			*Data,		// effect data
	geBoolean		Pause )		// new pause state
{

	// ensure valid data
	assert( Resource != NULL );
	assert( Data != NULL );

	// save paused state
	Data->Paused = Pause;

	// all done
	return;

	// get rid of warnings
	Resource;

} // Sprite_Pause()
