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

#if defined( __unix__ )
#	include <dlfcn.h>
#endif

#include "BASETYPE.H"
#include "System.h"
#include "RAM.H"
#include "engine.h"
#include "list.h"
#include "geAssert.h"
#include "Core/System.h"

//#define SKY_HACK
//extern BOOL GlobalReset;

geSystemLibrary geSystem_LoadLibrary( const char *path )
{
#if defined( _WIN32 )

	return LoadLibrary( path );

#else

	return dlopen( path, RTLD_LAZY );

#endif
}

geBoolean geSystem_FreeLibrary( geSystemLibrary library )
{
#if defined( _WIN32 )

	return FreeLibrary( library );

#else

	return ( dlclose( library ) == 0 );

#endif
}

void *geSystem_GetProcAddress( geSystemLibrary library, const char *name )
{
#if defined( _WIN32 )

	return GetProcAddress( library, name );

#else

	return dlsym( library, name );

#endif
}

//=====================================================================================
//	Implementation of Win32 functions for other platforms
//=====================================================================================
#if !defined( _WIN32 )

#	include <dlfcn.h>
#	include <time.h>

geBoolean QueryPerformanceCounter( LARGE_INTEGER *performanceCount )
{
	struct timespec tp;
	if ( clock_gettime( CLOCK_MONOTONIC, &tp ) != -1 )
	{
		return GE_FALSE;
	}

	performanceCount->QuadPart = tp.tv_sec * 10000000 + tp.tv_nsec / 100;
	return GE_TRUE;
}

#endif

//=====================================================================================
//	Local static globals
//=====================================================================================
static const char DriverFileNames[][ 200 ] = {
        { "GLDrv.dll" },
        { "GlideDrv.dll" },
        { "D3DDrv.dll" },
        { "SoftDrv.dll" },
        { "SoftDrv2.dll" },
        { "" },
};

//=====================================================================================
//	local static function prototypes
//=====================================================================================

static geBoolean EnumSubDrivers( Sys_DriverInfo *DriverInfo, const char *DriverDirectory );

static BOOL EnumSubDriversCB( S32 DriverId, char *Name, void *Context );
static BOOL EnumModesCB( S32 ModeId, char *Name, S32 Width, S32 Height, void *Context );

//=====================================================================================
//	geDriver_SystemGetNextDriver
//=====================================================================================
GENESISAPI geDriver *geDriver_SystemGetNextDriver( geDriver_System *DriverSystem, geDriver *Start )
{
	Sys_DriverInfo *DriverInfo;
	geDriver       *Last;

	assert( DriverSystem != NULL );

	DriverInfo = ( Sys_DriverInfo * ) DriverSystem;

	if ( !DriverInfo->NumSubDrivers )
		return NULL;

	Last = &DriverInfo->SubDrivers[ DriverInfo->NumSubDrivers - 1 ];

	if ( Start )// If they have a driver, return the next one
		Start++;
	else
		Start = DriverInfo->SubDrivers;// Else, return the first one...

	if ( Start > Last )// No more drivers left
		return NULL;

	// This must be true!!!
	assert( Start >= DriverInfo->SubDrivers && Start <= Last );

	return Start;// This is it...
}

//=====================================================================================
//	geDriver_GetNextMode
//=====================================================================================
GENESISAPI geDriver_Mode *geDriver_GetNextMode( geDriver *Driver, geDriver_Mode *Start )
{
	geDriver_Mode *Last;

	Last = &Driver->Modes[ Driver->NumModes - 1 ];

	if ( Start )// If there is a start, return the next one
		Start++;
	else
		Start = Driver->Modes;// Else, return the first

	if ( Start > Last )// No more Modes left
		return NULL;

	// This must be true...
	assert( Start >= Driver->Modes && Start <= Last );

	return Start;
}

//=====================================================================================
//	geDriver_GetName
//=====================================================================================
GENESISAPI geBoolean geDriver_GetName( geDriver *Driver, const char **Name )
{
	assert( Driver );
	assert( Name );

	*Name = Driver->Name;

	return GE_TRUE;
}

//=====================================================================================
//	geDriver_ModeGetName
//=====================================================================================
GENESISAPI geBoolean geDriver_ModeGetName( geDriver_Mode *Mode, const char **Name )
{
	assert( Mode );
	assert( Name );

	*Name = Mode->Name;

	return GE_TRUE;
}

//=====================================================================================
//	geDriver_ModeGetWidthHeight
//=====================================================================================
GENESISAPI geBoolean geDriver_ModeGetWidthHeight( geDriver_Mode *Mode, int32 *Width, int32 *Height )
{
	assert( Mode );
	assert( Width );
	assert( Height );

	*Width  = Mode->Width;
	*Height = Mode->Height;

	return GE_TRUE;
}

//=====================================================================================
//	Sys_EngineCreate
//	<> geEngine_Create
//=====================================================================================

const uint32 geEngine_Version = GE_VERSION;
const uint32 geEngine_Version_OldestSupported =
        ( ( GE_VERSION_MAJOR << GE_VERSION_MAJOR_SHIFT ) + GE_VERSION_MINOR_MIN );

geEngine *Sys_EngineCreate( HWND hWnd, const char *AppName, const char *DriverDirectory, uint32 Version )
{
	int32     i;
	geEngine *NewEngine;

	if ( ( Version & GE_VERSION_MAJOR_MASK ) != ( geEngine_Version & GE_VERSION_MAJOR_MASK ) )
	{
		geErrorLog_AddString( -1, "Genesis Engine has wrong major version!", NULL );
		return NULL;
	}

	if ( Version > geEngine_Version )
	{
		char str[ 1024 ];
		sprintf( str, "%d - %d", Version, geEngine_Version );
		geErrorLog_AddString( -1, "Genesis Engine is older than application; aborting!", str );
		return NULL;
	}

	if ( Version < geEngine_Version_OldestSupported )
	{
		char str[ 1024 ];
		sprintf( str, "%d - %d", Version, geEngine_Version );
		geErrorLog_AddString( -1, "Genesis Engine does not support the old version!", str );
		return NULL;
	}

	//	Attempt to create a new engine object
	NewEngine = GE_RAM_ALLOCATE_STRUCT( geEngine );

	if ( !NewEngine )
	{
		geErrorLog_Add( GE_ERR_OUT_OF_MEMORY, NULL );
		goto ExitWithError;
	}

	// Clear the engine structure...
	memset( NewEngine, 0, sizeof( geEngine ) );

	if ( !List_Start() )
	{
		geErrorLog_Add( GE_ERR_OUT_OF_MEMORY, NULL );
		goto ExitWithError;
	}

	size_t Length              = strlen( DriverDirectory ) + 1;
	NewEngine->DriverDirectory = geRam_Allocate( Length );

	if ( !NewEngine->DriverDirectory )
		goto ExitWithError;

	memcpy( NewEngine->DriverDirectory, DriverDirectory, Length );

	NewEngine->hWnd = hWnd;
	strcpy( NewEngine->AppName, AppName );

	// Get cpu info
	if ( !Sys_GetCPUFreq( &NewEngine->CPUInfo ) )
		goto ExitWithError;

	// Build the wavetable
	for ( i = 0; i < 20; i++ )
		NewEngine->WaveTable[ i ] = ( ( i * 65 ) % 200 ) + 50;

	if ( !EnumSubDrivers( &NewEngine->DriverInfo, DriverDirectory ) )
		goto ExitWithError;

	if ( !geEngine_BitmapListInit( NewEngine ) )
		goto ExitWithError;

	if ( !Light_EngineInit( NewEngine ) )
		goto ExitWithError;

	if ( !User_EngineInit( NewEngine ) )
		goto ExitWithError;

	if ( !geEngine_InitFonts( NewEngine ) )// must be after BitmapList
		goto ExitWithError;

	NewEngine->Changed = GE_TRUE;// Force a first time driver upload

	NewEngine->DisplayFrameRateCounter = GE_TRUE;// Default to showing the FPS counter

	geAssert_SetCriticalShutdownCallback( ( geAssert_CriticalShutdownCallback ) geEngine_ShutdownDriver, NewEngine );

	NewEngine->CurrentGamma = 1.0f;

	//geEngine_SetFogEnable(NewEngine, GE_TRUE, 255.0f, 0.0f, 0.0f, 250.0f, 1000.0f);
	geEngine_SetFogEnable( NewEngine, GE_FALSE, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f );

	return NewEngine;

// Error cleanup
ExitWithError:
{
	if ( NewEngine )
	{
		if ( NewEngine->DriverDirectory )
			geRam_Free( NewEngine->DriverDirectory );

		geRam_Free( NewEngine );
	}

	return NULL;
}
}


//=====================================================================================
//	Sys_EngineFree
//	<> geEngine_Destroy()
//=====================================================================================
void Sys_EngineFree( geEngine *Engine )
{
	geBoolean Ret;

	assert( Engine != NULL );

	if ( !Engine )
		return;

	Ret = geEngine_RemoveAllWorlds( Engine );
	assert( Ret );

	// Call upon modules to free allocated data in the engine
	Light_EngineShutdown( Engine );
	User_EngineShutdown( Engine );

	Ret = geEngine_ShutdownFonts( Engine );
	assert( Ret == GE_TRUE );

	Ret = geEngine_ShutdownDriver( Engine );
	assert( Ret == GE_TRUE );

	Ret = geEngine_BitmapListShutdown( Engine );
	assert( Ret == GE_TRUE );

	geRam_Free( Engine->DriverDirectory );

	List_Stop();

	geRam_Free( Engine );
}

//=====================================================================================
//	SysGetCPUFreq
//=====================================================================================
geBoolean Sys_GetCPUFreq( Sys_CPUInfo *Info )
{
	assert( Info != NULL );

#if defined( _WIN32 )

	LARGE_INTEGER Freq;
	if ( !QueryPerformanceFrequency( &Freq ) )
	{
		geErrorLog_Add( GE_ERR_NO_PERF_FREQ, NULL );
		return GE_FALSE;
	}

	Info->Freq = Freq.LowPart;

#else

	Info->Freq = 10000000;

#endif

	return GE_TRUE;
}

#ifdef MESHES
//===================================================================================
//	Sys_WorldCreateMesh
//	Create a mesh definition object
//===================================================================================
Mesh_MeshDef *Sys_WorldCreateMesh( geWorld *World, const char *BitmapPath, const char *FileName )
{
	Mesh_MeshDef *MeshDef;

	assert( World != NULL );
	assert( BitmapPath != NULL );
	assert( FileName != NULL );

	MeshDef = Mesh_WorldCreateMesh( World, BitmapPath, FileName );

	return MeshDef;
}

//===================================================================================
//	Sys_WorldFreeMesh
//===================================================================================
void Sys_WorldFreeMesh( geWorld *World, Mesh_MeshDef *MeshDef )
{
	assert( World != NULL );
	assert( MeshDef != NULL );

	Mesh_WorldFreeMesh( World, MeshDef );
}
#endif

//===================================================================================
//	EnumSubDriversCB
//===================================================================================
static BOOL EnumSubDriversCB( S32 DriverId, char *Name, void *Context )
{
	Sys_DriverInfo *DriverInfo = ( Sys_DriverInfo * ) Context;
	DRV_Driver     *RDriver;
	geDriver       *Driver;

	if ( strlen( Name ) >= DRV_STR_SIZE )
		return TRUE;// Ignore

	if ( DriverInfo->NumSubDrivers + 1 >= MAX_SUB_DRIVERS )
		return FALSE;// Stop when no more driver slots available

	Driver = &DriverInfo->SubDrivers[ DriverInfo->NumSubDrivers ];

	Driver->Id = DriverId;
	strcpy( Driver->Name, Name );
	strcpy( Driver->FileName, DriverInfo->CurFileName );

	RDriver = DriverInfo->RDriver;

	// Store this, so enum modes know what driver we are working on...
	DriverInfo->CurDriver = Driver;

	if ( !RDriver->EnumModes( Driver->Id, Driver->Name, EnumModesCB, ( void * ) DriverInfo ) )
		return FALSE;

	DriverInfo->NumSubDrivers++;

	return TRUE;
}

//===================================================================================
//	EnumModesCB
//===================================================================================
static BOOL EnumModesCB( S32 ModeId, char *Name, S32 Width, S32 Height, void *Context )
{
	Sys_DriverInfo *DriverInfo;
	geDriver       *Driver;
	geDriver_Mode  *Mode;

	if ( strlen( Name ) >= DRV_MODE_STR_SIZE )
		return TRUE;// Ignore

	DriverInfo = ( Sys_DriverInfo * ) Context;

	Driver = DriverInfo->CurDriver;

	if ( Driver->NumModes + 1 >= MAX_DRIVER_MODES )
		return FALSE;

	Mode = &Driver->Modes[ Driver->NumModes ];

	Mode->Id = ModeId;
	strcpy( Mode->Name, Name );
	Mode->Width  = Width;
	Mode->Height = Height;

	Driver->NumModes++;

	return TRUE;
}

//===================================================================================
//	EnumSubDrivers
//===================================================================================
static geBoolean EnumSubDrivers( Sys_DriverInfo *DriverInfo, const char *DriverDirectory )
{
	int32       i;
	DRV_Hook   *DriverHook;
	HINSTANCE   Handle;
	DRV_Driver *RDriver;
	geBoolean   GlideFound;

	DriverInfo->NumSubDrivers = 0;

	GlideFound = GE_FALSE;

	for ( i = 0; DriverFileNames[ i ][ 0 ] != 0; i++ )
	{
		if ( !strcmp( DriverFileNames[ i ], "D3DDrv.dll" ) && GlideFound )
			continue;// Skip D3D if we found a glidedrv

		Handle = geEngine_LoadLibrary( DriverFileNames[ i ], DriverDirectory );

		if ( !Handle )
			continue;

		DriverInfo->CurFileName = DriverFileNames[ i ];

		DriverHook = ( DRV_Hook * ) geSystem_GetProcAddress( Handle, "DriverHook" );
		if ( !DriverHook )
		{
			geSystem_FreeLibrary( Handle );
			continue;
		}

		if ( !DriverHook( &RDriver ) )
		{
			geSystem_FreeLibrary( Handle );
			continue;
		}

		if ( RDriver->VersionMajor != DRV_VERSION_MAJOR || RDriver->VersionMinor != DRV_VERSION_MINOR )
		{
			geSystem_FreeLibrary( Handle );
			geErrorLog_AddString( -1, "EnumSubDrivers : found driver of different version; ignoring; non-fatal", DriverFileNames[ i ] );
			continue;
		}

		DriverInfo->RDriver = RDriver;

		if ( !RDriver->EnumSubDrivers( EnumSubDriversCB, ( void * ) DriverInfo ) )
		{
			geSystem_FreeLibrary( Handle );
			continue;// Should we return FALSE, or just continue?
		}

		geSystem_FreeLibrary( Handle );

		if ( !strcmp( DriverFileNames[ i ], "GlideDrv.dll" ) )
			GlideFound = GE_TRUE;
	}

	DriverInfo->RDriver = NULL;// Reset this

	return GE_TRUE;
}
