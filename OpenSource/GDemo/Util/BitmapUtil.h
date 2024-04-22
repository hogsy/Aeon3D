/****************************************************************************************/
/*  BitmapUtil.h	                                                                    */
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
#ifndef BITMAPUTIL_H
#define BITMAPUTIL_H

#include "bitmap.h"

#ifdef __cplusplus
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
//	Prototypes
////////////////////////////////////////////////////////////////////////////////////////

//	Create a bitmap from a file.
//
////////////////////////////////////////////////////////////////////////////////////////
geBitmap * BitmapUtil_CreateFromFileName(
	geVFile		*File,		// file system to use
	const char	*Name,		// name of the file
	const char	*AlphaName,	// name of the alpha file
	geBoolean	MipIt,		// whether or not to create mips for it
	geBoolean	Sync );		// load it synchronously

//	Lock a bitmap for writing.
//
////////////////////////////////////////////////////////////////////////////////////////
geBoolean BitmapUtil_LockBitmap(
	geBitmap		*Bmp,			// bitmap to lock
	gePixelFormat	PixelFormat,	// pixel format to use
	geBitmap		**LockedBitmap,	// where to store locked bitmap pointer
	uint8			**Bits,			// where to store bits pointer
	geBitmap_Info	*BmpInfo );		// where to store bitmap info

//	Unlock a bitmap.
//
////////////////////////////////////////////////////////////////////////////////////////
geBoolean BitmapUtil_UnLockBitmap(
	geBitmap	*Bmp );	// bitmap to unlock


#ifdef __cplusplus
	}
#endif

#endif
