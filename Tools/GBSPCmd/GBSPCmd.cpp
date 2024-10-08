/*******************************************************************************
Copyright © 1999 WildTangent, Inc. All Rights Reserved
Copyright © 2024 Mark E. Sowden <hogsy@oldtimes-software.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*******************************************************************************/

#include <cstdio>
#include <cassert>
#include <string>

#if defined( _WIN32 )
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#endif

#include "../GBSPLib/Gbsplib.h"

HMODULE library;

static void error_callback( const char *string, ... )
{
	va_list argptr;
	char    buf[ 2048 ];
	va_start( argptr, string );
	vsnprintf( buf, sizeof( buf ), string, argptr );
	va_end( argptr );

	printf( "\x1b[31m%s\x1b[0m", buf );

	// errors from GBSPLib are more akin to warnings, apparently...
}

static void print_callback( const char *string, ... )
{
	va_list argptr;
	char    buf[ 2048 ];
	va_start( argptr, string );
	vsnprintf( buf, sizeof( buf ), string, argptr );
	va_end( argptr );

	printf( "%s", buf );
}

static GBSP_RETVAL process_bsp( GBSP_FuncHook *hookFunction, const std::string &filename, bool verbose, bool light, bool vis, bool entitiesOnly )
{
	std::string bspFilename = filename.substr( 0, filename.find_last_of( '.' ) ) + ".bsp";

	GBSP_RETVAL result;
	if ( entitiesOnly )
	{
		return hookFunction->GBSP_UpdateEntities( filename.c_str(), bspFilename.c_str() ) ? GBSP_OK : GBSP_ERROR;
	}
	else
	{
		BspParms bspParms      = {};
		bspParms.EntityVerbose = verbose;
		bspParms.Verbose       = verbose;

		result = hookFunction->GBSP_CreateBSP( filename.c_str(), &bspParms );
		if ( result != GBSP_OK )
		{
			return result;
		}

		result = hookFunction->GBSP_SaveGBSPFile( bspFilename.c_str() );
		if ( result != GBSP_OK )
		{
			return result;
		}

		hookFunction->GBSP_FreeBSP();

		if ( vis )
		{
			VisParms visParms    = {};
			visParms.FullVis     = true;
			visParms.SortPortals = true;

			result = hookFunction->GBSP_VisGBSPFile( bspFilename.c_str(), &visParms );
			if ( result != GBSP_OK )
			{
				return result;
			}
		}

		if ( light )
		{
			LightParms lightParms = {};
			lightParms.Radiosity  = true;
			lightParms.Verbose    = verbose;

			result = hookFunction->GBSP_LightGBSPFile( bspFilename.c_str(), &lightParms );
			if ( result != GBSP_OK )
			{
				return result;
			}
		}
	}

	return result;
}

int main( int argc, char **argv )
{
	if ( argc <= 1 )
	{
		printf( "Usage: GBSPCmd.exe <path>\n" );
		return EXIT_SUCCESS;
	}

	LightParms lightParms = {};

	bool entitiesOnly = false;
	bool verbose      = false;
	bool vis          = false;
	bool light        = false;
	for ( unsigned int i = 2; i < argc; ++i )
	{
		if ( strcmp( argv[ i ], "/ent" ) == 0 )
		{
			entitiesOnly = true;
			continue;
		}
		else if ( strcmp( argv[ i ], "/verbose" ) == 0 )
		{
			verbose = true;
			continue;
		}
		else if ( strcmp( argv[ i ], "/vis" ) == 0 )
		{
			vis = true;
			continue;
		}
		else if ( strcmp( argv[ i ], "/light" ) == 0 )
		{
			light = true;
			continue;
		}
		else if ( strcmp( argv[ i ], "/minlight" ) == 0 )
		{
			if ( i + 3 >= argc )
			{
				printf( "Invalid number of arguments for /minlight!\n" );
				return EXIT_FAILURE;
			}
			lightParms.MinLight.X = strtof( argv[ i + 1 ], nullptr );
			lightParms.MinLight.Y = strtof( argv[ i + 2 ], nullptr );
			lightParms.MinLight.Z = strtof( argv[ i + 3 ], nullptr );
			continue;
		}

		printf( "Unknown argument (%s)!\n", argv[ i ] );
	}

	library = LoadLibrary( "GBSPLib.dll" );
	if ( library == nullptr )
	{
		printf( "Failed to fetch GBSPLib library!\n" );
		return EXIT_FAILURE;
	}

	GBSP_INIT *initFunction = ( GBSP_INIT * ) GetProcAddress( library, "GBSP_Init" );
	if ( initFunction == nullptr )
	{
		printf( "Failed to fetch GBSPLib init function!\n" );
		return EXIT_FAILURE;
	}

	GBSP_Hook hook = {};
	hook.Error     = error_callback;
	hook.Printf    = print_callback;

	GBSP_FuncHook *hookFunction = initFunction( &hook );
	if ( hookFunction == nullptr )
	{
		printf( "Failed to fetch GBSPLib hook function!\n" );
		return EXIT_FAILURE;
	}

	if ( hookFunction->VersionMajor > GBSP_VERSION_MAJOR )
	{
		printf( "Incompatible GBSP version (%u > %u)!\n", hookFunction->VersionMajor, GBSP_VERSION_MAJOR );
		return EXIT_FAILURE;
	}

	std::string filename = argv[ 1 ];

	GBSP_RETVAL result = process_bsp( hookFunction, argv[ 1 ], verbose, light, vis, entitiesOnly );
	switch ( result )
	{
		default:
			assert( 0 );
		case GBSP_ERROR:
			fprintf( stderr, "Failed to create a BSP!\n" );
			return EXIT_FAILURE;
		case GBSP_OK:
			printf( "Compile succeeded!\n" );
			break;
		case GBSP_CANCEL:
			printf( "Compiled cancelled!\n" );
			return EXIT_SUCCESS;
	}

	return EXIT_SUCCESS;
}
