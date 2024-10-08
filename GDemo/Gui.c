/****************************************************************************************/
/*  Gui.c                                                                               */
/*                                                                                      */
/*  Author: Peter Siamidis                                                              */
/*  Description:    Simple gui for gdemo1               				                */
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
#include "genesis.h"
#include "ram.h"
#include "Gui.h"
#include "Cursor.h"
#include "resrc1.h"


////////////////////////////////////////////////////////////////////////////////////////
//	Gui struct
////////////////////////////////////////////////////////////////////////////////////////
typedef	struct Gui
{
	geEngine	*Engine;
	geBitmap	*Bg[7];
	int			TopCoor, BottomCoor;
	int			Left[7], Right[7];
	int			TopLeftCornerX, TopLeftCornerY;
	int			TotalBitmaps;
	int			WindowWidth, WindowHeight;
	geBoolean	Active;
	Cursor		*Cursor;
	int			CursorX, CursorY;
	int			CurrentBg;
	geBoolean   ShowCursor;

} Gui;



////////////////////////////////////////////////////////////////////////////////////////
//
//	Gui_Destroy()
//
//	Destroy the demos gui.
//
////////////////////////////////////////////////////////////////////////////////////////
void Gui_Destroy(
	Gui	**DeadAppGui )	// gui to destroy
{

	// locals
	Gui	*AppGui;
	int	i;

	// ensure valid data
	assert( DeadAppGui != NULL );
	assert( *DeadAppGui != NULL );

	// get gui pointer
	AppGui = *DeadAppGui;

	// destroy bitmaps
	for ( i = 0; i < AppGui->TotalBitmaps; i++ )
	{
		if ( AppGui->Bg[i] != NULL )
		{
			geEngine_RemoveBitmap( AppGui->Engine, AppGui->Bg[i] );
			geBitmap_Destroy( &( AppGui->Bg[i] ) );
		}
	}

	Cursor_Destroy(AppGui->Cursor);

	// free the gui struct
	geRam_Free( AppGui );
	*DeadAppGui = NULL;

} // Gui_Destroy()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Gui_Create()
//
//	Create the demos gui.
//
////////////////////////////////////////////////////////////////////////////////////////
Gui * Gui_Create(
	geEngine	*Engine,		// engine to use
	int			WindowWidth,	// window width
	int			WindowHeight )	// window height
{

	// locals
	Gui				*AppGui;
	geBoolean		Result;
	geBitmap_Info	BmpInfo;
	int				i;

	// ensure valid data
	assert( Engine != NULL );
	assert( WindowWidth > 0 );
	assert( WindowHeight > 0 );

	// allocate gui struct
	AppGui = geRam_AllocateClear( sizeof( *AppGui ) );
	if ( AppGui == NULL )
	{
		return NULL;
	}

	AppGui->Cursor = Cursor_Create(Engine, IDR_DATA1);
	if ( AppGui->Cursor == NULL )
	{
		return NULL;
	}

	// load up gui bitmaps
	AppGui->TotalBitmaps = 7;
	AppGui->Bg[0] = geBitmap_CreateFromFileName( NULL, "bmp\\gui\\template.bmp" );
	AppGui->Bg[1] = geBitmap_CreateFromFileName( NULL, "bmp\\gui\\sequence.bmp" );
	AppGui->Bg[2] = geBitmap_CreateFromFileName( NULL, "bmp\\gui\\hellfire.bmp" );
	AppGui->Bg[3] = geBitmap_CreateFromFileName( NULL, "bmp\\gui\\reactor.bmp" );
	AppGui->Bg[4] = geBitmap_CreateFromFileName( NULL, "bmp\\gui\\blood.bmp" );
	AppGui->Bg[5] = geBitmap_CreateFromFileName( NULL, "bmp\\gui\\seafloor.bmp" );
	AppGui->Bg[6] = geBitmap_CreateFromFileName( NULL, "bmp\\gui\\exit.bmp" );
	for ( i = 0; i < AppGui->TotalBitmaps; i++ )
	{
		if ( AppGui->Bg[i] == NULL )
		{
			Gui_Destroy( &AppGui );
			return NULL;
		}
		geEngine_AddBitmap( Engine, AppGui->Bg[i] );
	}

	// get some coorinate info
	Result = geBitmap_GetInfo( AppGui->Bg[0], &BmpInfo, NULL );
	if ( Result == GE_FALSE )
	{
		Gui_Destroy( &AppGui );
		return NULL;
	}
	AppGui->WindowWidth = WindowWidth;
	AppGui->WindowHeight = WindowHeight;
	AppGui->TopLeftCornerX = ( WindowWidth - BmpInfo.Width ) / 2;
	AppGui->TopLeftCornerY = ( WindowHeight - BmpInfo.Height ) / 2;

	// setup hotspots
	AppGui->TopCoor = 310 + AppGui->TopLeftCornerY;
	AppGui->BottomCoor = 480 + AppGui->TopLeftCornerY;
	AppGui->Left[1] = 0 + AppGui->TopLeftCornerX;
	AppGui->Right[1] = 110 + AppGui->TopLeftCornerX;
	AppGui->Left[2] = 111 + AppGui->TopLeftCornerX;
	AppGui->Right[2] = 215 + AppGui->TopLeftCornerX;
	AppGui->Left[3] = 216 + AppGui->TopLeftCornerX;
	AppGui->Right[3] = 320 + AppGui->TopLeftCornerX;
	AppGui->Left[4] = 321 + AppGui->TopLeftCornerX;
	AppGui->Right[4] = 425 + AppGui->TopLeftCornerX;
	AppGui->Left[5] = 426 + AppGui->TopLeftCornerX;
	AppGui->Right[5] = 530 + AppGui->TopLeftCornerX;
	AppGui->Left[6] = 531 + AppGui->TopLeftCornerX;
	AppGui->Right[6] = 640 + AppGui->TopLeftCornerX;

	// all done
	AppGui->ShowCursor = GE_TRUE;
	AppGui->CurrentBg = 0;
	AppGui->Active = GE_TRUE;
	AppGui->Engine = Engine;

	return AppGui;

} // Gui_Create()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Gui_PrepareFrame()
//
//	Prepare for first frame
//
////////////////////////////////////////////////////////////////////////////////////////
geBoolean Gui_PrepareFrame(
	Gui			*AppGui,	// gui to process
	geCamera	*Camera )	// current camera
{
	int i;

	// ensure valid data
	assert( AppGui != NULL );
	assert( Camera != NULL );

	// do nothing if gui isnt active
	if ( AppGui->Active == GE_FALSE )
	{
		// draw with the bitmaps to cache them
		return GE_FALSE;
	}

	geEngine_BeginFrame( AppGui->Engine, Camera, GE_TRUE );
	for (i = 0; i < AppGui->TotalBitmaps; i++)
	{
		geEngine_DrawBitmap( AppGui->Engine, AppGui->Bg[i], NULL, AppGui->TopLeftCornerX, AppGui->TopLeftCornerY );
	}
	geEngine_DrawBitmap( AppGui->Engine, AppGui->Bg[0], NULL, AppGui->TopLeftCornerX, AppGui->TopLeftCornerY );
	geEngine_EndFrame( AppGui->Engine );

	return GE_TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////
//
//	Gui_Frame()
//
//	Process a frame for the gui.
//
////////////////////////////////////////////////////////////////////////////////////////
geBoolean Gui_Frame(
	Gui			*AppGui,	// gui to process
	geCamera	*Camera )	// current camera
{

	// ensure valid data
	assert( AppGui != NULL );
	assert( Camera != NULL );

	// do nothing if gui isnt active
	if ( AppGui->Active == GE_FALSE )
	{
		return GE_FALSE;
	}

	// draw current bg
	geEngine_BeginFrame( AppGui->Engine, Camera, GE_TRUE );
	geEngine_DrawBitmap( AppGui->Engine, AppGui->Bg[AppGui->CurrentBg], NULL, AppGui->TopLeftCornerX, AppGui->TopLeftCornerY );
	if (AppGui->ShowCursor)
		Cursor_Render(AppGui->Cursor);
	geEngine_EndFrame( AppGui->Engine );

	// all done
	return GE_TRUE;

} // Gui_Frame()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Gui_SetCursorPos()
//
//	Let gui know about the current cursor pos.
//
////////////////////////////////////////////////////////////////////////////////////////
void Gui_SetCursorPos(
	Gui	*AppGui,	// gui to update
	int	CursorX,	// cursor x
	int	CursorY )	// cursor y
{

	// locals
	int	i;

	// ensure valid data
	assert( AppGui != NULL );

	if (CursorX > AppGui->TopLeftCornerX + AppGui->WindowWidth)
		CursorX = AppGui->TopLeftCornerX + AppGui->WindowWidth;

	if (CursorY > AppGui->TopLeftCornerY + AppGui->WindowHeight)
		CursorY = AppGui->TopLeftCornerY + AppGui->WindowHeight;

	// save cursor position
	AppGui->CursorX = CursorX;
	AppGui->CursorY = CursorY;
	Cursor_SetPosition(AppGui->Cursor, CursorX, CursorY);

	// determine currently active bg
	if ( ( CursorY < AppGui->TopCoor ) || ( CursorY > AppGui->BottomCoor ) )
	{
		AppGui->CurrentBg = 0;
	}
	else
	{
		for ( i = 1; i < AppGui->TotalBitmaps; i++ )
		{
			if ( ( CursorX >= AppGui->Left[i] ) && ( CursorX <= AppGui->Right[i] ) )
			{
				AppGui->CurrentBg = i;
				return;
			}
		}
	}

	// if we got to here then default to empty template
	AppGui->CurrentBg = 0;

} // Gui_SetCursorPos()


////////////////////////////////////////////////////////////////////////////////////////
//
//	Gui_ShowCursor()
//
//	Let gui know if the cursor needs to be drawn.
//
////////////////////////////////////////////////////////////////////////////////////////
void Gui_ShowCursor(
	Gui	*AppGui,	 // gui to update
	geBoolean State)// cursor state
{

	// ensure valid data
	assert( AppGui != NULL );

	AppGui->ShowCursor = State;

} // Gui_ShowCursor()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Gui_GetActiveBg()
//
//	Get currently active bg.
//
////////////////////////////////////////////////////////////////////////////////////////
int Gui_GetActiveBg(
	Gui	*AppGui )	// gui whose active bg we want
{

	// ensure valid data
	assert( AppGui != NULL );

	// return active bg
	return AppGui->CurrentBg;

} // Gui_GetActiveBg()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Gui_IsActive()
//
//	Is the gui currently active or not.
//
////////////////////////////////////////////////////////////////////////////////////////
geBoolean Gui_IsActive(
	Gui	*AppGui )	// gui whose status we want
{

	// ensure valid data
	assert( AppGui != NULL );

	// return active status
	return AppGui->Active;

} // Gui_IsActive()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Gui_SetActiveStatus()
//
//	Set gui status.
//
////////////////////////////////////////////////////////////////////////////////////////
void Gui_SetActiveStatus(
	Gui			*AppGui,	// gui whose active status we want to set
	geBoolean	NewStatus )	// new active status
{

	// ensure valid data
	assert( AppGui != NULL );

	// set active status
	AppGui->Active = NewStatus;

} // Gui_SetActiveStatus()

