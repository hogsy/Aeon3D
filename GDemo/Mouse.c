/****************************************************************************************/
/*  Mouse.c                                                                             */
/*                                                                                      */
/*  Author: Frank Maddin                                                                */
/*  Description:    Mouse manipulation routines          				                */
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

#include "windows.h"
#include "mouse.h"

typedef struct MouseInfo
{
	geBoolean InAWindow;
	geBoolean ResetPosition;
	HWND hWnd;
	int	Width;
	int Height;
}MouseInfo;

static MouseInfo Mouse;

//////////////////////////////////////////////////////

void Mouse_GetWindowRect(RECT *rect)
{
	if (Mouse.InAWindow)
		{
		GetWindowRect(Mouse.hWnd, rect);
		}
	else
		{
		rect->top = 0;
		rect->left = 0;
		rect->right = Mouse.Width;
		rect->bottom = Mouse.Height;
		}
}


void Mouse_Set(HWND hWnd, int Width, int Height, geBoolean InAWindow)
{
	Mouse.hWnd = hWnd;
	Mouse.InAWindow = InAWindow;
	Mouse.Width = Width;
	Mouse.Height = Height;
	Mouse.ResetPosition = GE_TRUE;
}

void Mouse_ShowWinCursor(void)
{
	while (ShowCursor(TRUE) < 0);
}

void Mouse_HideWinCursor(void)
{
	int count;

	while (1)
	{
		count = ShowCursor(FALSE);

		if (count < -1)
			{
			// add 2 because next loop will subtract 1
			ShowCursor(TRUE);
			ShowCursor(TRUE);
			continue;
			}

		if (count == -1)
			return;
	}
}


void Mouse_SetForPlayback(void)
{
	// dont reset the mouse
	// only show the cursor if you are in a window

	if (Mouse.InAWindow)
		{
		Mouse_ShowWinCursor();
		}
	else
		{
		Mouse_HideWinCursor();
		}

	Mouse.ResetPosition = GE_FALSE;
}

void Mouse_SetForControl(void)
{
	RECT	rect;
	// reset the mouse
	// hide the cursor

	Mouse_HideWinCursor();
	Mouse.ResetPosition = GE_TRUE;

	//GetWindowRect(Mouse.hWnd, &rect);
	Mouse_GetWindowRect(&rect);
	SetCursorPos(rect.left + (Mouse.Width/2), rect.top + (Mouse.Height/2));
}

void Mouse_SetForEdit(void)
{
	// resetting the mouse doesn't matter because the mouse isn't called for editing
	// hide the cursor

	Mouse_ShowWinCursor();
}

void Mouse_SetCenter(void)
{
	RECT	rect;

	//GetWindowRect(Mouse.hWnd, &rect);
	Mouse_GetWindowRect(&rect);

	if(Mouse.ResetPosition)
	{
		SetCursorPos(rect.left + (Mouse.Width/2), rect.top + (Mouse.Height/2));
	}
}

void Mouse_GetInput(float *DeltaX, float *DeltaY)
{
	POINT	Point;
	float	dx, dy;
	int32	x, y;
	RECT	rect;

	Mouse_GetWindowRect(&rect);

	GetCursorPos(&Point);

	Point.x -= rect.left;
	Point.y -= rect.top;
  
	x = Point.x;
	y = Point.y;

	dx = ((float) (((float)Mouse.Width/2.0f) - Point.x) / 350.0f);
	dy = ((float) (((float)Mouse.Height/2.0f) - Point.y) / 350.0f);

	if(Mouse.ResetPosition)
	{
		SetCursorPos(rect.left + (Mouse.Width/2), rect.top + (Mouse.Height/2));
	}

	*DeltaX = dx;
	*DeltaY = dy;
}

