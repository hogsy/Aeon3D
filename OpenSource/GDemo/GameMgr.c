/****************************************************************************************/
/*  GameMgr.c                                                                           */
/*                                                                                      */
/*  Description:    Misc routines mode and window related  				                */
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
#include "GameMgr.h"
#include "VidMode.h"
#include "errorlog.h"
#include "autoselect.h"
#include "drvlist.h"

static geBoolean Set = GE_FALSE;

static VidMode	CurVidMode;
static geCamera *CurCamera;
static int		CurWidth;
static int		CurHeight;

// video mode
#define STARTING_WIDTH  (500)
#define STARTING_HEIGHT (400)

geBoolean GameMgr_GetModeData(VidMode *VidMode, geCamera **Camera, int *Width, int *Height)
{
	if (Set == GE_FALSE)
	{
		geErrorLog_AddString(-1, "GameMgr_GetModeData:  Data not set by GameMgr_PickMode.", NULL);
		return GE_FALSE;
	}
		
	*VidMode = CurVidMode;
	*Camera = CurCamera;
	*Width = CurWidth;
	*Height = CurHeight;

	return GE_TRUE;
}

void GameMgr_ResetMainWindow(HWND hWnd, int32 Width, int32 Height)
{
	RECT ScreenRect;

	GetWindowRect(GetDesktopWindow(),&ScreenRect);

	SetWindowLong(hWnd, 
                 GWL_STYLE, 
                 GetWindowLong(hWnd, GWL_STYLE) & ~WS_POPUP);

	SetWindowLong(hWnd, 
                 GWL_STYLE, 
                 GetWindowLong(hWnd, GWL_STYLE) | (WS_OVERLAPPED  | 
                                                   WS_CAPTION     | 
                                                   WS_SYSMENU     | 
                                                   WS_MINIMIZEBOX));

	SetWindowLong(hWnd, 
                  GWL_STYLE, 
                  GetWindowLong(hWnd, GWL_STYLE) | WS_THICKFRAME |
                                                     WS_MAXIMIZEBOX);

	//SetWindowLong(hWnd, 
    //              GWL_EXSTYLE, 
    //              GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_TOPMOST);

	{
		RECT ClientRect;
		int Style = GetWindowLong (hWnd, GWL_STYLE);
		
		ClientRect.left = 0;
		ClientRect.top = 0;
		ClientRect.right = Width - 1;
		ClientRect.bottom = Height - 1;
		AdjustWindowRect (&ClientRect, Style, FALSE);	// FALSE == No menu	
		
		{
			int WindowWidth = ClientRect.right - ClientRect.left + 1;
			int WindowHeight = ClientRect.bottom - ClientRect.top + 1;
			SetWindowPos 
			(
				hWnd, HWND_TOP,
				(ScreenRect.right+ScreenRect.left)/2 - WindowWidth/2,
				(ScreenRect.bottom+ScreenRect.top)/2 - WindowHeight/2, 
				WindowWidth, WindowHeight,
				SWP_NOCOPYBITS | SWP_NOZORDER
			);
		}
	}

	ShowWindow(hWnd, SW_SHOWNORMAL);
	UpdateWindow(hWnd);
	SetFocus(hWnd);
}

//====================================================================================
//	GameMgr_SetDriverAndMode
//====================================================================================
geBoolean GameMgr_SetDriverAndMode(geDriver *Driver, geDriver_Mode *DriverMode, 
								   int Width, int Height)
{
	geRect		CameraRect = {0, 0, 0, 0};
	
	if (VidMode_SetResolution(&CurVidMode,Width,Height)==GE_FALSE)
	{
		geErrorLog_AddString(-1, "GameMgr_SetDriverAndMode:  Invalid Width or Height.", NULL);
		return GE_FALSE;
	}
	
	// Make the camera be a good default value...
	CameraRect.Left = 0;
	CameraRect.Right = Width-1;
	CameraRect.Top = 0;
	CameraRect.Bottom = Height-1;
	
	CurWidth = Width;
	CurHeight = Height;

	//
	// Then create a camera for client, etc to use for rendering, etc...
	//
	CurCamera = geCamera_Create(2.0f, &CameraRect);
	
	if (!CurCamera)
	{
		geErrorLog_AddString(-1, "GameMgr_SetDriverAndMode:  geCamera_Create failed.", NULL);
		return GE_FALSE;
	}
	
	// Set the camera ZScale
	geCamera_SetZScale(CurCamera, 1.0f);
	
	return GE_TRUE;
}

geBoolean GameMgr_PickMode(HWND hwnd, HANDLE hInstance, geEngine *Engine, 
					 geBoolean NoSelection, geBoolean ManualSelection, 
					 ModeList *List, int ListLength, int *ListSelection, geBoolean *WindowMode)
{
	
	assert( hwnd != 0 );
	assert( hInstance != 0 );
	assert( List != NULL );
	assert( ListSelection != NULL );
	
	Set = GE_FALSE;

	if (!NoSelection && !ManualSelection)
	{
		if (AutoSelect_PickDriver(hwnd, Engine, List, ListLength, ListSelection)==GE_FALSE)
		{
			geErrorLog_AddString(-1,"Automatic video mode selection failed to find good mode.  Trying manual selection.",NULL);
			ManualSelection = GE_TRUE;
		}
	}
	
	
	if (NoSelection || ManualSelection)
	{
		while (1)
		{
			if (!NoSelection)
			{
				if (DrvList_PickDriver(hInstance, hwnd, List, ListLength, ListSelection)==GE_FALSE)
				{
					geErrorLog_AddString(-1,"Driver pick dialog failed",NULL);
					//App_ShutdownAll();
					exit(1);
				}
			}
			NoSelection = GE_FALSE;
			
			if ( *ListSelection < 0 )	// no selection made
			{
				//App_ShutdownAll();
				exit(1);
			}
			
			geEngine_ShutdownDriver(Engine);
			
			if(List[*ListSelection].InAWindow)
			{
				*WindowMode = GE_TRUE;
				GameMgr_ResetMainWindow(hwnd,List[*ListSelection].Width,List[*ListSelection].Height);
			}
			
			if (!geEngine_SetDriverAndMode(Engine, List[*ListSelection].Driver, List[*ListSelection].Mode))
			{
				GameMgr_ResetMainWindow(hwnd,STARTING_WIDTH,STARTING_HEIGHT);
				geErrorLog_AddString(-1, "geEngine_SetDriverAndMode failed. (continuing)", NULL);
				MessageBox(hwnd, "Driver failed to properly set the selected mode.","Error:",MB_OK);
			}
			else
			{ 
				break;
			}
		}
	}
	
	
	// Set the driver and the mode through the game manager...
	if (!GameMgr_SetDriverAndMode(List[*ListSelection].Driver, List[*ListSelection].Mode, 
		List[*ListSelection].Width,  List[*ListSelection].Height))
	{
		geErrorLog_AddString(-1, "GameMgr_SetDriverAndMode failed. (continuing)", NULL);
		return GE_FALSE;
	}

	Set = GE_TRUE;

	return GE_TRUE;
}

