/****************************************************************************************/
/*  Cursor.c                                                                            */
/*                                                                                      */
/*  Author: Frank Maddin                                                                */
/*  Description:    Cursor manipulation routines        				                */
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

#include "errorlog.h"
#include "resrc1.h"
#include "cursor.h"
#include "ram.h"

typedef	struct	Cursor
{
	geEngine *	Engine;
	geBitmap *	Decal;
	int			ScreenX;
	int			ScreenY;
}	Cursor;

void	Cursor_Destroy(Cursor *curs)
{
	assert(curs != NULL);

	if (curs->Decal != NULL)
	{
		geEngine_RemoveBitmap(curs->Engine, curs->Decal);
		geBitmap_Destroy (&curs->Decal);
	}
	geRam_Free (curs);
}

Cursor *	Cursor_Create (geEngine *Engine, const int ResourceId)
{
	Cursor *curs;
	geVFile *CursorFile;

	assert(Engine != NULL);

	curs = GE_RAM_ALLOCATE_STRUCT (Cursor);
	if( curs == NULL )
	{
		geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE,"Cursor_Create: Allocate failed.\n",NULL);
		goto Exit;
	}

	// open Cursor file
	CursorFile = geVFile_OpenNewSystem( NULL, GE_VFILE_TYPE_DOS, "bmp\\cursor.bmp", NULL, GE_VFILE_OPEN_READONLY );
	if( CursorFile == NULL )
	{
		geErrorLog_AddString(-1,"Cursor_Create: Unable to open bitmap file.\n",NULL);
		goto Exit;
	}

	// create Cursor bitmap
	curs->Decal = geBitmap_CreateFromFile( CursorFile );
	geVFile_Close( CursorFile );

	if ( curs->Decal == NULL )
	{
		geErrorLog_AddString(-1,"Cursor_Create: Unable to create bitmap.\n",NULL);
		goto Exit;
	}

	geBitmap_SetColorKey( curs->Decal, GE_TRUE, 255, GE_FALSE );

	curs->Engine = Engine;
	if (geEngine_AddBitmap(Engine, curs->Decal) == GE_FALSE)
	{
		geErrorLog_AddString(-1,"Cursor_Create: Unable to add bitmap to the engine.\n",NULL);
		goto Exit;
	}

	return curs;

Exit:

	if (curs)
	{
		geBitmap_Destroy(&curs->Decal);
	}

	return NULL;
}

void	Cursor_SetPosition(Cursor *curs, int X, int Y)
{
	assert(curs);
	curs->ScreenX = X;
	curs->ScreenY = Y;
}

void	Cursor_GetPosition(Cursor *curs, int *X, int *Y)
{
	assert(curs);
	*X = curs->ScreenX;
	*Y = curs->ScreenY;
}

void	Cursor_Render(Cursor *curs)
{
	GE_Rect	Rect;

	assert(curs);
	assert(curs->Engine);
	assert(curs->Decal);

	Rect.Left = 0;
	Rect.Top = 0;
	Rect.Right = 16;
	Rect.Bottom = 16;

	geEngine_DrawBitmap (curs->Engine, curs->Decal, &Rect, curs->ScreenX, curs->ScreenY);
}
