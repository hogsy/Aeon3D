/****************************************************************************************/
/*  Render.c                                                                            */
/*                                                                                      */
/*  Author: John Pollard                                                                */
/*  Description: Code to render polys in glide                                          */
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
/*  The Original Code is Genesis3D, released March 25, 1999.                            */
/*Genesis3D Version 1.1 released November 15, 1999                            */
/*  Copyright (C) 1999 WildTangent, Inc. All Rights Reserved           */
/*                                                                                      */
/****************************************************************************************/
#include <assert.h>
#include <Math.h>

#include "Render.h"
#include "GMain.h"
#include "GlideDrv.h"
#include "GTHandle.h"

#define ENABLE_WIREFRAME

#ifdef ENABLE_WIREFRAME
static int DoWireFrame = 0;
#endif

#define MAX_LMAP_SIZE 32

#define SNAP_VERT( v ) ( ( float ) ( ( long ) ( ( v ) * 16 ) ) * 0.0625f /* 1/16 */ )

#define RENDER_MAX_PNTS ( 64 )

typedef enum
{
	ColorCombine_Undefined,
	ColorCombine_Gouraud,
	ColorCombine_Texture,
	ColorCombine_TextureGouraud,
	ColorCombine_TextureGouraudWithFog,
} Render_ColorCombine;

typedef enum
{
	TexCombine_Undefined,
	TexCombine_SinglePassGouraud,
	TexCombine_SinglePassTexture,
	TexCombine_SimultaneousPass,
	TexCombine_PassThrough
} Render_TexCombine;

DRV_RENDER_MODE     RenderMode = RENDER_NONE;
Render_ColorCombine Render_OldColorCombine = ColorCombine_Undefined;
Render_TexCombine   Render_OldTexCombine = TexCombine_Undefined;
int32               Render_HardwareMode = RENDER_UNKNOWN_MODE;
uint32              Render_HardwareFlags = 0;

uint32 PolyMode = DRV_POLYMODE_NORMAL;

DRV_CacheInfo CacheInfo;

uint32 CurrentLRU;

extern geBoolean g_FogEnable;

//==========================================================================================
//	Render_SetColorCombine
//==========================================================================================
void Render_SetColorCombine( Render_ColorCombine ColorCombine )
{
	if ( Render_OldColorCombine == ColorCombine )
		return;// Nothing to change

	switch ( ColorCombine )
	{
		case ColorCombine_Gouraud:
		{
			glDisable( GL_TEXTURE_2D );
			break;
		}
		case ColorCombine_Texture:
		{
			glEnable( GL_TEXTURE_2D );
			glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );
			break;
		}
		case ColorCombine_TextureGouraud:
		{
			glEnable( GL_TEXTURE_2D );
			glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
			break;
		}
		case ColorCombine_TextureGouraudWithFog:
		{
			glEnable( GL_TEXTURE_2D );
			//TODO
			//guColorCombineFunction( GR_COLORCOMBINE_TEXTURE_TIMES_ITRGB_ADD_ALPHA );
			break;
		}

		default:
			assert( 0 );
	}

	Render_OldColorCombine = ColorCombine;
}

//==========================================================================================
//	Render_SetTexCombine
//==========================================================================================
void Render_SetTexCombine( Render_TexCombine TexCombine )
{
	if ( Render_OldTexCombine == TexCombine )
		return;// Nothing to change

	switch ( TexCombine )
	{
		case TexCombine_SinglePassGouraud:
		{
			glActiveTexture( GL_TEXTURE0 );
			glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

			if ( g_BoardInfo.NumTMU >= 2 )
			{
				glActiveTexture( GL_TEXTURE1 );
				glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ZERO );
			}
			break;
		}

		case TexCombine_SinglePassTexture:
		{
			glActiveTexture( GL_TEXTURE0 );
			glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );

			if ( g_BoardInfo.NumTMU >= 2 )
			{
				glActiveTexture( GL_TEXTURE1 );
				glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ZERO );
			}
			break;
		}

		case TexCombine_SimultaneousPass:
		{
			assert( g_BoardInfo.NumTMU >= 2 );

			glActiveTexture( GL_TEXTURE0 );
			glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

			glActiveTexture( GL_TEXTURE1 );
			glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );
			break;
		}

		case TexCombine_PassThrough:
		{
			assert( g_BoardInfo.NumTMU >= 2 );

			glActiveTexture( GL_TEXTURE0 );
			glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

			glActiveTexture( GL_TEXTURE1 );
			glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );
			break;
		}

		default:
			assert( 0 );
	}

	Render_OldTexCombine = TexCombine;
}

//==========================================================================================
//	Render_SetHardwareMode
//==========================================================================================
void Render_SetHardwareMode( int32 NewMode, uint32 NewFlags )
{
	if ( NewFlags != Render_HardwareFlags )// See if the flags gave changed,,,
	{
		if ( NewFlags & DRV_RENDER_NO_ZMASK )
			glDepthFunc( GL_ALWAYS );
		else if ( Render_HardwareFlags & DRV_RENDER_NO_ZMASK )
			glDepthFunc( GL_GEQUAL );

		if ( NewFlags & DRV_RENDER_NO_ZWRITE )
			glDepthMask( GL_FALSE );
		else if ( Render_HardwareFlags & DRV_RENDER_NO_ZWRITE )
			glDepthMask( GL_TRUE );

		if ( NewFlags & DRV_RENDER_CLAMP_UV )
		{
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		}
		else if ( Render_HardwareFlags & DRV_RENDER_CLAMP_UV )
		{
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		}
	}

	// Make these flags recent
	Render_HardwareFlags = NewFlags;

	if ( NewMode == Render_HardwareMode )// && NewFlags == Render_HardwareFlags)
		return;                          // Nothing to change

	if ( Render_HardwareMode == RENDER_DECAL_MODE )
	{
		glDisable( GL_ALPHA_TEST );
	}

	// sets up hardware mode
	switch ( NewMode )
	{
		case ( RENDER_MISC_TEX_POLY_MODE ):
		{
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

			Render_SetColorCombine( ColorCombine_TextureGouraud );
			Render_SetTexCombine( TexCombine_SinglePassTexture );

			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

			if ( g_FogEnable )
			{
				glFogi( GL_FOG_MODE, GL_EXP2 );
			}

			break;
		}

		case ( RENDER_MISC_GOURAD_POLY_MODE ):
		{
			Render_SetColorCombine( ColorCombine_Gouraud );
			Render_SetTexCombine( TexCombine_SinglePassGouraud );
			glBlendFuncSeparate( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE );

			if ( g_FogEnable )
			{
				glFogi( GL_FOG_MODE, GL_EXP2 );
			}

			break;
		}

		case ( RENDER_LINES_POLY_MODE ):
		{
			Render_SetColorCombine( ColorCombine_Gouraud );
			Render_SetTexCombine( TexCombine_SinglePassGouraud );
			glBlendFuncSeparate( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE );

			if ( g_FogEnable )
			{
				glFogi( GL_FOG_MODE, GL_EXP2 );
			}

			break;
		}

		case ( RENDER_WORLD_POLY_MODE_NO_LIGHTMAP ):
		{
			Render_SetColorCombine( ColorCombine_TextureGouraud );
			Render_SetTexCombine( TexCombine_SinglePassTexture );

			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			glBlendFunc( GL_ONE, GL_ZERO );

			if ( g_FogEnable )
			{
				glFogi( GL_FOG_MODE, GL_EXP2 );
			}

			break;
		}

		case ( RENDER_WORLD_POLY_MODE ):
		{
			Render_SetColorCombine( ColorCombine_TextureGouraud );
			Render_SetTexCombine( TexCombine_SinglePassTexture );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			glBlendFunc( GL_ONE, GL_ZERO );

			if ( g_FogEnable )
				glFogi( GL_FOG_MODE, GL_LINEAR );
			break;
		}

		case ( RENDER_WORLD_TRANSPARENT_POLY_MODE ):
		{
			Render_SetColorCombine( ColorCombine_TextureGouraud );
			Render_SetTexCombine( TexCombine_SinglePassTexture );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );// Bug fix thanks to Bobtree
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
			if ( g_FogEnable )
				glFogi( GL_FOG_MODE, GL_LINEAR );
			break;
		}

		// NOTE - IF this card has 2 TMU's, world polys AND lightmap polys will be piped through here
		//	Notice how Simultaneous mode is turned on when 2 TMU's are detected... -JP
		case ( RENDER_LIGHTMAP_POLY_MODE ):
		{
			glActiveTexture( GL_TEXTURE1 );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

			if ( g_BoardInfo.NumTMU >= 2 )
			{
				Render_SetColorCombine( ColorCombine_TextureGouraud );

				glBlendFuncSeparate( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
				                     GL_ONE, GL_ONE );

				if ( g_FogEnable )
					glEnable( GL_FOG );

				Render_SetTexCombine( TexCombine_SimultaneousPass );

				glActiveTexture( GL_TEXTURE0 );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			}
			else
			{
				Render_SetColorCombine( ColorCombine_TextureGouraud );
				// Singlepass mode if there is onle 1 TMU
				Render_SetTexCombine( TexCombine_SinglePassTexture );
				// Modulate the texture with the framebuffer
				if ( g_FogEnable )
				{
					//glFogi(GL_FOG_MODE, GL_EXP2);
					glEnable( GL_FOG );
					glFogi( GL_FOG_MODE, GL_EXP2 );
					glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
				}
				else
				{
					glBlendFunc( GL_ONE, GL_ZERO );
				}
				// Force clamping to be on if this card only has one TMU
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
				Render_HardwareFlags |= DRV_RENDER_CLAMP_UV;
			}

			break;
		}

		case ( RENDER_LIGHTMAP_FOG_POLY_MODE ):
		{
			glActiveTexture( GL_TEXTURE1 );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			Render_SetColorCombine( ColorCombine_TextureGouraud );
			if ( g_BoardInfo.NumTMU >= 2 )
			{
				Render_SetTexCombine( TexCombine_PassThrough );
				glActiveTexture( GL_TEXTURE0 );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			}
			else
			{
				Render_SetTexCombine( TexCombine_SinglePassTexture );
			}
			if ( g_FogEnable )
				//glFogi(GL_FOG_MODE,GL_FOG_COORD);
				glDisable( GL_FOG );
			glBlendFunc( GL_ONE, GL_ONE );
			break;
		}

		case ( RENDER_DECAL_MODE ):
		{
#if 0
			grLfbConstantDepth( 0xffff );
			grLfbConstantAlpha( 0xff );
			grChromakeyMode( GR_CHROMAKEY_ENABLE );
			grAlphaBlendFunction( GR_BLEND_ONE, GR_BLEND_ZERO,
			                      GR_BLEND_ONE, GR_BLEND_ZERO );
#endif
			break;
		}

		default:
		{
			assert( 0 );
		}
	}

	Render_HardwareMode = NewMode;
}

//****************************************************************************
//	Render a regualar gouraud shaded poly
//****************************************************************************
geBoolean DRIVERCC Render_GouraudPoly( DRV_TLVertex *Pnts, int32 NumPoints, uint32 Flags )
{
	int32   i;
	GLfloat vertex[ RENDER_MAX_PNTS ][ 6 ];

	for ( i = 0; i < NumPoints; i++ )
	{
		vertex[ i ][ 0 ] = Pnts->x;
		vertex[ i ][ 1 ] = Pnts->y;
		vertex[ i ][ 2 ] = Pnts->z;

		vertex[ i ][ 3 ] = Pnts->r;
		vertex[ i ][ 4 ] = Pnts->g;
		vertex[ i ][ 5 ] = Pnts->b;

		Pnts++;
	}

	Render_SetHardwareMode( RENDER_MISC_GOURAD_POLY_MODE, Flags );

	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_COLOR_ARRAY );

	glVertexPointer( 3, GL_FLOAT, 0, vertex );
	glColorPointer( 3, GL_FLOAT, 0, &vertex[ 0 ][ 3 ] );

	glDrawArrays( GL_POLYGON, 0, NumPoints );

	glDisableClientState( GL_COLOR_ARRAY );
	glDisableClientState( GL_VERTEX_ARRAY );

	GLIDEDRV.NumRenderedPolys++;

	return TRUE;
}

//==========================================================================================
//	Render_LinesPoly
//==========================================================================================
geBoolean DRIVERCC Render_LinesPoly( DRV_TLVertex *Pnts, int32 NumPoints )
{
	int32   i;
	GLfloat vertices[ RENDER_MAX_PNTS ][ 4 ];
	GLfloat colors[ RENDER_MAX_PNTS ][ 4 ];

	for ( i = 0; i < NumPoints; i++ )
	{
		colors[ i ][ 0 ] = 1.0f;
		colors[ i ][ 1 ] = 1.0f;
		colors[ i ][ 2 ] = 1.0f;
		colors[ i ][ 3 ] = 1.0f;

		vertices[ i ][ 0 ] = Pnts->x;
		vertices[ i ][ 1 ] = Pnts->y;
		vertices[ i ][ 2 ] = Pnts->z;
		vertices[ i ][ 3 ] = 1 / Pnts->z;

		Pnts++;
	}

	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_COLOR_ARRAY );

	glVertexPointer( 4, GL_FLOAT, 0, vertices );
	glColorPointer( 4, GL_FLOAT, 0, colors );

	for ( i = 0; i < NumPoints; i++ )
	{
		int32  i2 = ( ( i + 1 ) < NumPoints ) ? ( i + 1 ) : 0;
		GLuint indices[ 2 ] = { i, i2 };
		glDrawElements( GL_LINES, 2, GL_UNSIGNED_INT, indices );
	}

	glDisableClientState( GL_COLOR_ARRAY );
	glDisableClientState( GL_VERTEX_ARRAY );

	GLIDEDRV.NumRenderedPolys++;

	return TRUE;
}

//****************************************************************************
//	Render a world texture / lightmap
//****************************************************************************
geBoolean DRIVERCC Render_WorldPoly( DRV_TLVertex *Pnts, int32 NumPoints, geRDriver_THandle *THandle, DRV_TexInfo *TexInfo, DRV_LInfo *LInfo, uint32 Flags )
{
#if 0//TODO
	GrVertex      Vrtx[ RENDER_MAX_PNTS ], *pVrtx;
	float         OneOverSize_255;
	float         ShiftU, ShiftV, ScaleU, ScaleV;
	DRV_TLVertex *pPnts;
	int32         i;
	float         Alpha;

	assert( Pnts );
	assert( TexInfo );

#	ifdef ENABLE_WIREFRAME
	if ( DoWireFrame )
		return ( Render_LinesPoly( Pnts, NumPoints ) );
#	endif

#	if 0
	switch (PolyMode)
	{
		case DRV_POLYMODE_NORMAL:
			break;							// Use this function
		case DRV_POLYMODE_GOURAUD:
			return (Render_GouraudPoly(Pnts, NumPoints, 0));
		case DRV_POLYMODE_LINES:
			return (Render_LinesPoly(Pnts, NumPoints));
	}
#	endif

	GLIDEDRV.NumRenderedPolys++;

	OneOverSize_255 = THandle->OneOverLogSize_255;

	// Get how much to shift U, and V for the Texture
	ShiftU = TexInfo->ShiftU;
	ShiftV = TexInfo->ShiftV;
	ScaleU = 1.0f / TexInfo->DrawScaleU;
	ScaleV = 1.0f / TexInfo->DrawScaleV;

	pPnts = Pnts;

	pVrtx = Vrtx;

#	if 0
	// Fix the uv's to be as close to the origin as possible, without affecting their appearance...
	//if (pPnts->u > 1000.0f || pPnts->v > 1000.0f)
	{
		float		OneOverLogSize;

 		OneOverLogSize = 1.0f / (float)THandle->LogSize;
		
		ShiftU -= (float)(((int32)(pPnts->u*ScaleU/THandle->Width))*THandle->Width);
		ShiftV -= (float)(((int32)(pPnts->v*ScaleV/THandle->Height))*THandle->Height);
	}
#	endif

	Alpha = Pnts->a;

	for ( i = 0; i < NumPoints; i++ )
	{
		float ZRecip;

		pVrtx->a = Alpha;

		pVrtx->r = pPnts->r;
		pVrtx->g = pPnts->g;
		pVrtx->b = pPnts->b;

		ZRecip = ( 1.0f / ( pPnts->z ) );
		pVrtx->ooz = 65535.0f * ZRecip;
		pVrtx->oow = ZRecip;

		pVrtx->tmuvtx[ 0 ].oow = ZRecip;
		pVrtx->tmuvtx[ 1 ].oow = ZRecip;

		ZRecip *= OneOverSize_255;

		pVrtx->tmuvtx[ TMU[ 0 ] ].sow = ( pPnts->u * ScaleU + ShiftU ) * ZRecip;
		pVrtx->tmuvtx[ TMU[ 0 ] ].tow = ( pPnts->v * ScaleV + ShiftV ) * ZRecip;

		pVrtx->x = SNAP_VERT( pPnts->x );
		pVrtx->y = SNAP_VERT( pPnts->y );

		pPnts++;
		pVrtx++;
	}


	// Set the source texture for TMU 0
	SetupTexture( THandle );

	// If only 1 TMU (or no lightmap), then draw first pass poly now.
	if ( g_BoardInfo.NumTMU == 1 || !LInfo )
	{// The lightmap will blend over it
		if ( Flags & DRV_RENDER_ALPHA )
			Render_SetHardwareMode( RENDER_WORLD_TRANSPARENT_POLY_MODE, Flags );
		else
		{
			if ( LInfo )
				Render_SetHardwareMode( RENDER_WORLD_POLY_MODE, Flags );
			else
				Render_SetHardwareMode( RENDER_WORLD_POLY_MODE_NO_LIGHTMAP, Flags );
		}

		grDrawPolygonVertexList( NumPoints, Vrtx );
	}

#	if 0//todo
	if ( LInfo )// If there is a lightmap, render it now, on top of the first pass poly
	{
		geBoolean Dynamic;

		// How much to shift u'vs back into lightmap space
		ShiftU = ( float ) LInfo->MinU - 8.0f;
		ShiftV = ( float ) LInfo->MinV - 8.0f;

		pPnts = Pnts;

		// Call the engine to set this sucker up, because it's visible...
		GLIDEDRV.SetupLightmap( LInfo, &Dynamic );

		OneOverSize_255 = LInfo->THandle->OneOverLogSize_255;

		pVrtx = Vrtx;

		for ( i = 0; i < NumPoints; i++ )
		{
			float u = pPnts->u - ShiftU;
			float v = pPnts->v - ShiftV;
			float ZRecip = pVrtx->oow * OneOverSize_255;

			pVrtx->tmuvtx[ TMU[ 1 ] ].sow = u * ZRecip;
			pVrtx->tmuvtx[ TMU[ 1 ] ].tow = v * ZRecip;
			pPnts++;
			pVrtx++;
		}

		RenderLightmapPoly( Vrtx, NumPoints, LInfo, ( geBoolean ) Dynamic, Flags );
	}
#	endif

	return TRUE;
#endif
}

#if 0
void RenderLightmapPoly( GrVertex *vrtx, int32 NumPoints, DRV_LInfo *LInfo, geBoolean Dynamic, uint32 Flags )
{
	geRDriver_THandle *THandle;
	int32              l;
	GCache_Slot       *Slot;

	THandle = LInfo->THandle;

	Slot = SetupLMapTexture( THandle, LInfo, Dynamic, 0 );

	GCache_SlotSetLRU( Slot, CurrentLRU );
	TextureSource( TMU[ 1 ], GCache_SlotGetMemAddress( Slot ), GR_MIPMAPLEVELMASK_BOTH, GCache_SlotGetInfo( Slot ) );

	Render_SetHardwareMode( RENDER_LIGHTMAP_POLY_MODE, Flags );

	grDrawPolygonVertexList( NumPoints, vrtx );

	// Render special maps
	for ( l = 1; l < 2; l++ )
	{
		if ( !LInfo->RGBLight[ l ] )
			continue;

		switch ( l )
		{
			case LMAP_TYPE_LIGHT:
				DownloadLightmap( LInfo, THandle->LogSize, Slot, 0 );
				TextureSource( TMU[ 1 ], GCache_SlotGetMemAddress( Slot ), GR_MIPMAPLEVELMASK_BOTH, GCache_SlotGetInfo( Slot ) );
				Render_SetHardwareMode( RENDER_LIGHTMAP_POLY_MODE, Flags );
				break;

			case LMAP_TYPE_FOG:
				DownloadLightmap( LInfo, THandle->LogSize, Slot, l );
				TextureSource( TMU[ 1 ], GCache_SlotGetMemAddress( Slot ), GR_MIPMAPLEVELMASK_BOTH, GCache_SlotGetInfo( Slot ) );
				Render_SetHardwareMode( RENDER_LIGHTMAP_FOG_POLY_MODE, Flags );
				break;
		}

		grDrawPolygonVertexList( NumPoints, vrtx );
	}
}

//**********************************************************************************
//	Downloads a lightmap to the card
//**********************************************************************************
void DownloadLightmap( DRV_LInfo *LInfo, int32 Wh, GCache_Slot *Slot, int32 LMapNum )
{
	uint16     TempL[ MAX_LMAP_SIZE * MAX_LMAP_SIZE ];// Temp to hold converted 565 lightmap
	int32      w, h;
	uint16    *pTempP = TempL;
	uint8      r, g, b;
	uint8     *Bits;
	int32      W = LInfo->Width;
	int32      H = LInfo->Height;
	GrTexInfo *Info;

	//memset(TempL, 0, sizeof(uint6)*32*32);
	Bits = ( uint8 * ) LInfo->RGBLight[ LMapNum ];

	for ( h = 0; h < H; h++ )
	{
		for ( w = 0; w < W; w++ )
		{
			r = *( Bits++ );
			g = *( Bits++ );
			b = *( Bits++ );

			r >>= 3;
			g >>= 2;
			b >>= 3;

			*pTempP++ = ( uint16 ) ( ( r << ( 11 ) ) + ( g << 5 ) + b );
		}
		pTempP += ( Wh - w );
	}

	Info = GCache_SlotGetInfo( Slot );
	Info->data = TempL;

	GCache_UpdateSlot( LMapCache, Slot, Info );
}
#endif

//************************************************************************************
//	Render a misc texture poly....
//************************************************************************************
geBoolean DRIVERCC Render_MiscTexturePoly( DRV_TLVertex *Pnts, int32 NumPoints, geRDriver_THandle *THandle, uint32 Flags )
{
	int32         i;
	GLfloat       vertices[ NumPoints ][ 4 ];
	GLfloat       colors[ NumPoints ][ 4 ];
	GLfloat       texCoords[ NumPoints ][ 3 ];
	DRV_TLVertex *pPnt = Pnts;

#ifdef ENABLE_WIREFRAME
	if ( DoWireFrame )
	{
		return ( Render_LinesPoly( Pnts, NumPoints ) );
	}
#endif

	assert( Pnts != NULL );
	assert( NumPoints < RENDER_MAX_PNTS );
	assert( THandle != NULL );

	for ( i = 0; i < NumPoints; i++ )
	{
		vertices[ i ][ 0 ] = pPnt->x;
		vertices[ i ][ 1 ] = pPnt->y;
		vertices[ i ][ 2 ] = pPnt->z;
		vertices[ i ][ 3 ] = 1.0f / pPnt->z;

		colors[ i ][ 0 ] = pPnt->r;
		colors[ i ][ 1 ] = pPnt->g;
		colors[ i ][ 2 ] = pPnt->b;
		colors[ i ][ 3 ] = pPnt->a;

		texCoords[ i ][ 0 ] = pPnt->u;
		texCoords[ i ][ 1 ] = pPnt->v;
		texCoords[ i ][ 2 ] = 1.0f;

		pPnt++;
	}

	SetupTexture( THandle );

	Render_SetHardwareMode( RENDER_MISC_TEX_POLY_MODE, Flags );

	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_COLOR_ARRAY );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );

	glVertexPointer( 4, GL_FLOAT, 0, vertices );
	glColorPointer( 4, GL_FLOAT, 0, colors );
	glTexCoordPointer( 3, GL_FLOAT, 0, texCoords );

	glEnable( GL_TEXTURE_2D );
	//TODO
	//glBindTexture(GL_TEXTURE_2D, THandle->TextureId);

	glDrawArrays( GL_POLYGON, 0, NumPoints );

	glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	glDisableClientState( GL_COLOR_ARRAY );
	glDisableClientState( GL_VERTEX_ARRAY );

	return TRUE;
}

geRDriver_THandle *OldPalHandle;

//============================================================================================
//	SetupTexture
//============================================================================================
void SetupTexture( geRDriver_THandle *THandle )
{
#if 0
	GTHandle_CheckTextures();

	// Setup the palette
	if ( THandle->PixelFormat.PixelFormat == GE_PIXELFORMAT_8BIT )
	{
		assert( THandle->PalHandle );
		assert( THandle->PalHandle->Data );

		// CB <> one shared palette in glide; added _UPDATE check
		if ( ( OldPalHandle != THandle->PalHandle ) || ( THandle->PalHandle->Flags & THANDLE_UPDATE ) )
		{
			grTexDownloadTable( TMU[ 0 ], GR_TEXTABLE_PALETTE, THandle->PalHandle->Data );
			OldPalHandle = THandle->PalHandle;
			THandle->PalHandle->Flags &= ~THANDLE_UPDATE;
		}
	}

	if ( !THandle->Slot || GCache_SlotGetUserData( THandle->Slot ) != THandle )
	{
		THandle->Slot = GCache_TypeFindSlot( THandle->CacheType );
		assert( THandle->Slot );

		GCache_SlotSetUserData( THandle->Slot, THandle );
		THandle->Flags |= THANDLE_UPDATE;

		CacheInfo.TexMisses++;
	}

	if ( THandle->Flags & THANDLE_UPDATE )
	{
		GrTexInfo *Info;

		Info = GCache_SlotGetInfo( THandle->Slot );

		// Set the data to the correct bits
		Info->data = THandle->Data;

		// We must make sure the textures formats and the caches format match (formats can change on the fly)
		GlideFormatFromGenesisFormat( THandle->PixelFormat.PixelFormat, &Info->format );

		GCache_UpdateSlot( TextureCache, THandle->Slot, Info );

		THandle->Flags &= ~THANDLE_UPDATE;
	}

	GCache_SlotSetLRU( THandle->Slot, CurrentLRU );
	TextureSource( TMU[ 0 ], GCache_SlotGetMemAddress( THandle->Slot ), GR_MIPMAPLEVELMASK_BOTH, GCache_SlotGetInfo( THandle->Slot ) );
#endif
}

//==================================================================================
//	Render_DrawDecal
//==================================================================================
geBoolean DRIVERCC Render_DrawDecal( geRDriver_THandle *THandle, RECT *SRect, int32 x, int32 y )
{
	if ( x >= ClientWindow.Width || y >= ClientWindow.Height )
	{
		return TRUE;
	}

	int32 width, height;
	if ( SRect != NULL )
	{
		width = SRect->right - SRect->left;
		height = SRect->bottom - SRect->top;
	}
	else
	{
		width = THandle->Width;
		height = THandle->Height;
	}

	glBegin( GL_QUADS );

	glVertex3i( x, y, 0 );
	glColor3f( 1.0f, 0.0f, 0.0f );

	glVertex3i( x + width, y, 0 );
	glColor3f( 0.0f, 1.0f, 0.0f );

	glVertex3i( x + width, y + height, 0 );
	glColor3f( 1.0f, 0.0f, 0.0f );

	glVertex3i( x, y + height, 0 );
	glColor3f( 0.0f, 1.0f, 0.0f );

	glEnd();

	return GE_TRUE;
}

//=====================================================================================
//	Scene managment functions
//=====================================================================================
geBoolean DRIVERCC BeginScene( geBoolean Clear, geBoolean ClearZ, RECT *WorldRect )
{
	memset( &CacheInfo, 0, sizeof( DRV_CacheInfo ) );

#ifdef ENABLE_WIREFRAME
	{
		uint32 KeyState1, KeyState2;

#	pragma message( "Glide : WireFrame enabled!" )
		KeyState1 = GetAsyncKeyState( VK_CONTROL ) & 0x8001;
		KeyState2 = GetAsyncKeyState( VK_F8 ) & 0x8001;
		if ( KeyState1 && KeyState2 )
		{
			DoWireFrame ^= 1;
		}
	}

	if ( DoWireFrame )
	{
		Clear = GE_TRUE;
		ClearZ = GE_TRUE;
	}
#endif

	if ( !GTHandle_CheckTextures() )
	{
		return GE_FALSE;
	}

	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE );// FIXME: Make clear zbuffer and frame buffer separate
	if ( Clear )
	{
		if ( g_FogEnable )
		{
			uint32  fogColorInt = ( ( ( uint32 ) g_FogB << 16 ) |
                                   ( ( uint32 ) g_FogG << 8 ) |
                                   ( uint32 ) g_FogR );
			GLfloat fogColor[ 4 ] = { ( GLfloat ) ( ( fogColorInt >> 16 ) & 0xFF ) / 255.0f,
			                          ( GLfloat ) ( ( fogColorInt >> 8 ) & 0xFF ) / 255.0f,
			                          ( GLfloat ) ( fogColorInt & 0xFF ) / 255.0f, 0.0f };

			glClearColor( fogColor[ 0 ], fogColor[ 1 ], fogColor[ 2 ], fogColor[ 3 ] );
			glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		}
		else
		{
#if !defined( NDEBUG )
			glClearColor( 0.5f, 0.5f, 0.5f, 1.0f );
#else
			glClearColour( 0.0f, 0.0f, 0.0f, 1.0f );
#endif
			glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		}
	}
	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );

	GLIDEDRV.NumRenderedPolys = 0;

	CurrentLRU++;

	return TRUE;
}

geBoolean DRIVERCC EndScene( void )
{
	// Mike: force z buffer on
	glDepthMask( GL_TRUE );
	Render_HardwareFlags &= ~DRV_RENDER_NO_ZWRITE;

	glDepthFunc( GL_GEQUAL );
	Render_HardwareFlags &= ~DRV_RENDER_NO_ZMASK;

	// swapping the front and back buffer on the next vertical retrace
	//TODO: urgh, window manager needs to deal with this...
#if defined( _WIN32 )

	HDC hdc = GetDC( ClientWindow.hWnd );
	assert( hdc != NULL );
	SwapBuffers( hdc );

#endif

	return TRUE;
}

geBoolean DRIVERCC BeginWorld( void )
{
	//ResetSpans( ClientWindow.Height );
	//NumWorldPixels = 0;

	GLIDEDRV.NumWorldPixels = 0;
	GLIDEDRV.NumWorldSpans = 0;

	RenderMode = RENDER_WORLD;

	return TRUE;
}

geBoolean DRIVERCC EndWorld( void )
{
	GLIDEDRV.NumWorldPixels = 0;//NumWorldPixels;
	GLIDEDRV.NumWorldSpans = 0; //NumSpans;

	RenderMode = RENDER_NONE;

	return TRUE;
}

geBoolean DRIVERCC BeginMeshes( void )
{
	RenderMode = RENDER_MESHES;
	return TRUE;
}

geBoolean DRIVERCC EndMeshes( void )
{
	RenderMode = RENDER_NONE;
	return TRUE;
}

geBoolean DRIVERCC BeginModels( void )
{
	RenderMode = RENDER_MODELS;
	return TRUE;
}

geBoolean DRIVERCC EndModels( void )
{
	RenderMode = RENDER_NONE;
	return TRUE;
}