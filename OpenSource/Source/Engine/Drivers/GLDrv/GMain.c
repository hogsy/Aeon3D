/****************************************************************************************/
/*  GMain.c                                                                             */
/*                                                                                      */
/*  Author: John Pollard                                                                */
/*  Description: Glide initialization code, etc                                         */
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
#include <Windows.h>
#include <StdIO.h>

#include "GlideDrv.h"
#include "GMain.h"
#include "GTHandle.h"

int WriteBMP( unsigned short *ScreenBuffer, const char *Name );

DRV_Window ClientWindow;

static RECT OldWindow;

GMain_BoardInfo g_BoardInfo;// Global board info for current hardware

geBoolean g_FogEnable = GE_TRUE;
float     g_FogR;
float     g_FogG;
float     g_FogB;

void GLMain_Log( const char *string, ... )
{
	char buffer[ 2048 ];
	wvsprintf( buffer, string, ( char * ) ( &string + 1 ) );

	OutputDebugString( buffer );

	FILE *file = fopen( "GLDrv.log", "a+t" );
	if ( file == NULL )
	{
		return;
	}

	fprintf( file, "%s", buffer );
	fclose( file );
}

//==============================================================================
//==============================================================================
geBoolean GMain_Startup( DRV_DriverHook *Hook )
{
	switch ( Hook->Mode )
	{
		case 0:
		{
			ClientWindow.Width = 800;
			ClientWindow.Height = 600;
			break;
		}
		case 1:
		{
			ClientWindow.Width = 1024;
			ClientWindow.Height = 480;
			break;
		}
		case 2:
		{
			ClientWindow.Width = 1280;
			ClientWindow.Height = 1024;
			break;
		}
		default:
		{
			SetLastDrvError( DRV_ERROR_NULL_WINDOW, "GLIDE_DrvInit:  Invalid display mode." );
			return FALSE;
		}
	}

	ClientWindow.hWnd = Hook->hWnd;

#if defined( _WIN32 )

	// Go full-screen so we won't lose the mouse
	{
		RECT DeskTop;

		// Save the old window size
		GetWindowRect( ClientWindow.hWnd, &OldWindow );

		// Get the size of the desktop
		GetWindowRect( GetDesktopWindow(), &DeskTop );

		// Resize the window to the size of the desktop
		MoveWindow( ClientWindow.hWnd, DeskTop.left - 4, DeskTop.top - 40, DeskTop.right + 20, DeskTop.bottom + 20, TRUE );

		// Center the mouse
		SetCursorPos( ClientWindow.Width / 2, ClientWindow.Height / 2 );
	}

	HDC dc = GetDC( ClientWindow.hWnd );

	PIXELFORMATDESCRIPTOR pfd = { 0 };
	pfd.nSize = sizeof( PIXELFORMATDESCRIPTOR );
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 24;
	pfd.cStencilBits = 8;
	pfd.iLayerType = PFD_MAIN_PLANE;

	int format = ChoosePixelFormat( dc, &pfd );
	SetPixelFormat( dc, format, &pfd );

	HGLRC hRC = wglCreateContext( dc );
	if ( !wglMakeCurrent( dc, hRC ) )
	{
		const char *msg = "Failed to make GL context current!\n";
		GLMain_Log( "%s", msg );
		SetLastDrvError( DRV_ERROR_INIT_ERROR, msg );
		return GE_FALSE;
	}

#endif

	// initialize glew
	GLenum err;
	if ( ( err = glewInit() ) != GLEW_OK )
	{
		char tmp[ 128 ];
		snprintf( tmp, sizeof( tmp ), "Failed to initialize GLEW: %s", glewGetErrorString( err ) );

		GLMain_Log( "%s\n", tmp );

		SetLastDrvError( DRV_ERROR_INIT_ERROR, tmp );
		return GE_FALSE;
	}

	// Get the info about this board
	if ( !GMain_GetBoardInfo( &g_BoardInfo ) )
		return FALSE;

	if ( g_BoardInfo.NumTMU <= 0 )
	{
		SetLastDrvError( DRV_ERROR_INIT_ERROR, "GLIDE_DrvInit:  Not enough texture mapping units." );
		return GE_FALSE;
	}

	// We know that GLIDE will be in 5-6-5 mode...
	ClientWindow.R_shift = 5 + 6;
	ClientWindow.G_shift = 5;
	ClientWindow.B_shift = 0;

	ClientWindow.R_mask = 0xf800;
	ClientWindow.G_mask = 0x07e0;
	ClientWindow.B_mask = 0x001f;

	ClientWindow.R_width = 5;
	ClientWindow.G_width = 6;
	ClientWindow.B_width = 5;

	SetLastDrvError( DRV_ERROR_NONE, "GMain_Startup:  No error." );

	if ( !GTHandle_Startup() )
	{
		SetLastDrvError( DRV_ERROR_GENERIC, "GMain_Startup:  GTHandle_Startup failed...\n" );
		return GE_FALSE;
	}

	if ( !GMain_InitGlideRegisters() )
	{
		SetLastDrvError( DRV_ERROR_GENERIC, "GMain_Startup:  GMain_InitGlideRegisters failed...\n" );
		return GE_FALSE;
	}

	return GE_TRUE;
}

//==================================================================================
//	GMain_Shutdown
//==================================================================================
void GMain_Shutdown( void )
{
	GTHandle_Shutdown();

	// Resize the window to the size of the original size
	MoveWindow( ClientWindow.hWnd, OldWindow.left, OldWindow.top, OldWindow.right, OldWindow.bottom, TRUE );
}

//==================================================================================
//	GMain_GetBoardInfo
//	OpenGL is assumed to be initialized before this function is called...
//==================================================================================
geBoolean GMain_GetBoardInfo( GMain_BoardInfo *Info )
{
	GLint nTexUnits;

	memset( Info, 0, sizeof( *Info ) );

	// Query the OpenGL context for the number of texture units
	glGetIntegerv( GL_MAX_TEXTURE_UNITS, &nTexUnits );
	Info->NumTMU = nTexUnits;
	// TODO: OpenGL does not provide a direct method to obtain the amount of Video RAM
	Info->MainRam = 536870912;

	return GE_TRUE;
}

//==================================================================================
//	GMain_InitGlideRegisters
//==================================================================================
geBoolean GMain_InitGlideRegisters( void )
{
	//
	// Setup card register states
	//

	// fix up the z-buffer
	glDepthFunc( GL_GEQUAL );
	glDepthMask( GL_TRUE );

	// Fixup the transparent color - alpha test
	glDisable( GL_ALPHA_TEST );

	// Fixup the Mipmapping
	glActiveTexture( GL_TEXTURE0 );
	glTexEnvf( GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, -1.0f );
	// Tell it how we like it...
	glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE );
	// turn on gouraud shading
	glShadeModel( GL_SMOOTH );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glEnable( GL_BLEND );

	// The following corresponds to the usage of second TMU
	if ( g_BoardInfo.NumTMU >= 2 )
	{
		glActiveTexture( GL_TEXTURE1 );
		glTexEnvf( GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, 0.3f );
		glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE );
	}

	//GMain_SetFogEnable(GE_TRUE, 0.0f, 255.0f, 0.0f, 500.0f, 1500.0f);
	GMain_SetFogEnable( GE_FALSE, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f );

	return GE_TRUE;
}

extern uint32 CurrentLRU;// Shame!!!

//==================================================================================
//	Reset the 3dfx, and free all allocated ram
//==================================================================================
geBoolean GMain_ResetAll( void )
{
	GTHandle_Shutdown();

	if ( !GTHandle_Startup() )
		return GE_FALSE;

	if ( !GMain_InitGlideRegisters() )
		return GE_FALSE;

	CurrentLRU = 0;

	return GE_TRUE;
}

//==================================================================================
//==================================================================================
geBoolean DRIVERCC GMain_ScreenShot( const char *Name )
{
	uint16 *Buffer;
	Buffer = ( uint16 * ) malloc( sizeof( uint16 * ) * 640 * 480 );
	glReadPixels( 0, 0, 640, 480, GL_RGB, GL_UNSIGNED_SHORT, Buffer );

	GLenum error = glGetError();
	if ( error == GL_NO_ERROR )
	{
		WriteBMP( Buffer, Name );
	}

	free( Buffer );

	if ( error != GL_NO_ERROR )
	{
		SetLastDrvError( DRV_ERROR_GENERIC, "OpenGL: Could not save BMP." );
		return GE_FALSE;
	}

	return GE_TRUE;
}

//==================================================================================
//	GMain_SetFogEnable
//==================================================================================
geBoolean DRIVERCC GMain_SetFogEnable( geBoolean Enable, float r, float g, float b, float Start, float End )
{
	g_FogEnable = Enable;
	g_FogR = r;
	g_FogG = g;
	g_FogB = b;

	if ( g_FogEnable )
	{
		glEnable( GL_FOG );
		GLfloat fogColor[ 4 ] = { r, g, b, 1.0f };
		glFogfv( GL_FOG_COLOR, fogColor );
		glFogi( GL_FOG_MODE, GL_LINEAR );
		glFogf( GL_FOG_START, Start );
		glFogf( GL_FOG_END, End );
	}
	else
	{

		glDisable( GL_FOG );
	}
	return GE_TRUE;
}