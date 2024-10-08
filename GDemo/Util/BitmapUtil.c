/****************************************************************************************/
/*  BitmapUtil.c                                                                        */
/*                                                                                      */
/*  Description:    Functions for dealing with bitmaps. 				                */
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
#include <assert.h>
#include "BitmapUtil.h"
#include "ErrorLog.h"



////////////////////////////////////////////////////////////////////////////////////////
//
//	BitmapUtil_CreateFromFileName()
//
//	Create a bitmap from a file.
//
////////////////////////////////////////////////////////////////////////////////////////
geBitmap * BitmapUtil_CreateFromFileName(
	geVFile		*File,		// file system to use
	const char	*Name,		// name of the file
	const char	*AlphaName,	// name of the alpha file
	geBoolean	MipIt,		// whether or not to create mips for it
	geBoolean	Sync )		// load it synchronously
{

	// locals
	geVFile		*BmpFile;
	geBitmap	*Bmp;
	geBoolean	Result;

	// ensure valid data
	assert( Name != NULL );

	// open the bitmap
	if ( File == NULL )
	{
		BmpFile = geVFile_OpenNewSystem( NULL, GE_VFILE_TYPE_DOS, Name, NULL, GE_VFILE_OPEN_READONLY );
	}
	else
	{
		BmpFile = geVFile_Open( File, Name, GE_VFILE_OPEN_READONLY );
	}
	if ( BmpFile == NULL )
	{
		geErrorLog_AddString(-1,"BitmapUtil_CreateFromFileName: Unable to open file.\n",Name);
		return NULL;
	}

	// create the bitmap
	Bmp = geBitmap_CreateFromFile( BmpFile );
	geVFile_Close( BmpFile );
	if ( Bmp == NULL )
	{
		geErrorLog_AddString(-1,"BitmapUtil_CreateFromFileName: Unable to create bitmap.\n",Name);
		return NULL;
	}

	// add alpha if required...
	if ( AlphaName != NULL )
	{

		// locals
		geBitmap	*AlphaBmp;
		geVFile		*AlphaFile;

		// open alpha file
		if ( File == NULL )
		{
			AlphaFile = geVFile_OpenNewSystem( NULL, GE_VFILE_TYPE_DOS, AlphaName, NULL, GE_VFILE_OPEN_READONLY );
		}
		else
		{
			AlphaFile = geVFile_Open( File, AlphaName, GE_VFILE_OPEN_READONLY );
		}
		if( AlphaFile == NULL )
		{
			geBitmap_Destroy( &Bmp );
			geErrorLog_AddString(-1,"BitmapUtil_CreateFromFileName: Unable to open file.\n",AlphaName);
			return NULL;
		}

		// create alpha bitmap
		AlphaBmp = geBitmap_CreateFromFile( AlphaFile );
		geVFile_Close( AlphaFile );
		if ( AlphaBmp == NULL )
		{
			geBitmap_Destroy( &Bmp );
			geErrorLog_AddString(-1,"BitmapUtil_CreateFromFileName: Unable to create bitmap.\n",AlphaName);
			return NULL;
		}

		// set its alpha
		Result = geBitmap_SetAlpha( Bmp, AlphaBmp );
		if ( Result == GE_FALSE )
		{
			geBitmap_Destroy( &Bmp );
			geErrorLog_AddString(-1,"BitmapUtil_CreateFromFileName: Unable to set alpha.\n",AlphaName);
			return NULL;
		}

		// don't need the alpha anymore
		geBitmap_Destroy( &AlphaBmp );
	}
	// ...or just set the color key
	else
	{
		Result = geBitmap_SetColorKey( Bmp, GE_TRUE, 255, GE_FALSE );
		assert( Result );
	}

	// create mipmaps for it
	if ( MipIt == GE_TRUE )
	{
		//geBitmap_SetMipCount( Bmp, 4 ); undone
	}

	// all done
	return Bmp;

} // BitmapUtil_CreateFromFileName()



////////////////////////////////////////////////////////////////////////////////////////
//
//	BitmapUtil_LockBitmap()
//
//	Lock a bitmap for writing.
//
////////////////////////////////////////////////////////////////////////////////////////
geBoolean BitmapUtil_LockBitmap(
	geBitmap		*Bmp,			// bitmap to lock
	gePixelFormat	PixelFormat,	// pixel format to use
	geBitmap		**LockedBitmap,	// where to store locked bitmap pointer
	uint8			**Bits,			// where to store bits pointer
	geBitmap_Info	*BmpInfo )		// where to store bitmap info
{

	// locals
	geBoolean	Success = GE_TRUE;
	geBitmap	*DestLocked;

	// ensure valid data
	assert( Bmp != NULL );
	assert( LockedBitmap != NULL );
	assert( Bits != NULL );
	assert( BmpInfo != NULL );

	// lock the hud bitmap for writing
	if ( geBitmap_LockForWriteFormat( Bmp, &DestLocked, 0, 0, PixelFormat ) == GE_FALSE )
	{
		Success = GE_FALSE;
		geErrorLog_AddString(-1,"BitmapUtil_LockBitmap: geBitmap_LockForWriteFormat failed.\n",NULL);
		goto ALLDONE;
	}
	*LockedBitmap = DestLocked;
	if ( geBitmap_GetInfo( DestLocked, BmpInfo, NULL ) == GE_FALSE )
	{
		Success = GE_FALSE;
		geErrorLog_AddString(-1,"BitmapUtil_LockBitmap: geBitmap_GetInfo failed.\n",NULL);
		goto ALLDONE;
	}
	*Bits = geBitmap_GetBits( DestLocked );
	if ( *Bits == NULL )
	{
		Success = GE_FALSE;
		geErrorLog_AddString(-1,"BitmapUtil_LockBitmap: geBitmap_GetBits failed.\n",NULL);
		goto ALLDONE;
	}

	// all done
	ALLDONE:
	return Success;

} // BitmapUtil_LockBitmap()



////////////////////////////////////////////////////////////////////////////////////////
//
//	BitmapUtil_UnLockBitmap()
//
//	Unlock a bitmap.
//
////////////////////////////////////////////////////////////////////////////////////////
geBoolean BitmapUtil_UnLockBitmap(
	geBitmap	*Bmp )	// bitmap to unlock
{

	// ensure valid data
	assert( Bmp != NULL );

	// unlock and return
	return geBitmap_UnLock( Bmp );

} // BitmapUtil_UnLockBitmap()
