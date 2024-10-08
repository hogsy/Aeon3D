/****************************************************************************************/
/*  TimeEdit.c                                                                          */
/*                                                                                      */
/*  Author: Frank Maddin                                                                */
/*  Description:    Timeline editor for motions								            */
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
#define	WIN32_LEAN_AND_MEAN
#include	<windows.h>
#include	<assert.h>
#include	<stdio.h>

#include "errorlog.h"
#include "ram.h"
#include "TimeEdit.h"

/*
												| BORDERY %
					*							+ _MARK %			^
		K		K	|		K		K			+ _KEY %			|
								|				+ _TIC %			|
		+----------------------------			0					|SizeY	
								|				- _TIC %			|
			N		|	N						- _KEY %			|
					*							- _MARK %			v  
												| BORDERY %
<		> BORDERX %

		^ + is PositionX,PositionY
		<        SizeX				>
*/

#ifndef GENESIS_20
#define GE_ERR_WINDOWS_API_FAILURE 99
#define GE_ERR_SUBSYSTEM_FAILURE   98
#define geErrorLog_IntToString(xx) NULL
#endif

#define TIMEEDIT_TIC		10
#define TIMEEDIT_KEY		30
#define TIMEEDIT_MARK		15
#define TIMEEDIT_BORDERX	5
#define TIMEEDIT_BORDERY	5

static int TimeEdit_RegisterCount = 0;

#define MAX_KEYS 512
#define MAX_EVENTS 512

#define ELLIPSE_SIZE 4
#define ELLIPSE_SEL_SIZE 8

#define TIMEKEY_ELLIPSE_YOFF -12
#define EVENT_ELLIPSE_YOFF 24

#define TIMELINE_TEXT_YOFF 4

#define TIMEBAR_HEIGHT 90

#define DRAG_TIMER 1
#define DRAG_DEBOUCE 80

typedef struct TimeEdit
{
	HWND		hwnd;
	HINSTANCE	hinst;
	int			PositionX,PositionY;
	int			SizeX,SizeY;
	int			WindowAcross,WindowDown;

	float		TimeStart;			// start time of time line
	float		TimeEnd;			// end time of time line


	// keys and events
	float		KeyList[MAX_KEYS];		// list of key times
	float		EventList[MAX_EVENTS];	// list of event times
	int			KeyCount;
	int			EventCount;
	float		*KeysSelected[MAX_KEYS];	// need to allow for selection of muliple keys
	float		*EventsSelected[MAX_KEYS];	// need to allow for selection of muliple keys

	// dragging vars
	int			DragStartX;
	int			DragStartY;
	int			DragX;
	int			DragY;
	geBoolean   Dragging;

	// time bar
	float		CurrTime;
	geBoolean	TimeBarSelected;

	// callback vars
	TE_CB 		ParentCB;
	void *		CBdata;

	geBoolean	Looping;

} TimeEdit;

static geBoolean	TimeEdit_Paint(TimeEdit *TE);
					
static geBoolean	TimeEdit_AnimateSelected(TimeEdit *TE);
static geBoolean	TimeEdit_Select(TimeEdit *TE, int MouseX, int MouseY);
static geBoolean	TimeEdit_SelectTest(TimeEdit *TE, float TimeKey, int MouseX, int MouseY, int Yoff);
static float		TimeEdit_XToTime(TimeEdit* TE, int MouseX);
static int			TimeEdit_TimeToX(TimeEdit* TE, float Time);

static geBoolean	TimeEdit_MoveSelectedKeys(TimeEdit *TE, int MouseX, int MouseY);
static geBoolean	TimeEdit_MoveSelectedKeys(TimeEdit *TE, int MouseX, int MouseY);
static geBoolean	TimeEdit_SelectMultiTest(TimeEdit *TE, int MouseX, int MouseY);
static geBoolean	TimeEdit_AddKey(TimeEdit* TE, int MouseX);
static geBoolean	TimeEdit_DelKey(TimeEdit* TE);
static geBoolean	TimeEdit_AddEvent(TimeEdit* TE, int MouseX);
static geBoolean	TimeEdit_DelEvent(TimeEdit* TE);
static geBoolean	TimeEdit_IsEventArea(TimeEdit *TE, int MouseX, int MouseY);
static int			TimeEdit_TimeToX(TimeEdit* TE, float Time);
static geBoolean	TimeEdit_InsertTime(TimeEdit *TE);


#define TIMEEDIT_WINDOW_CLASS "EclipseTimeEditor"


/////////////////////////////////////////////////////////////////////////////////////////////
// Windows Procedure for TimeEdit

LRESULT CALLBACK TimeEdit_WndProc(HWND hWnd, UINT iMessage,
						 WPARAM wParam, LPARAM lParam)
{
	TimeEdit *TE;
	TE = (TimeEdit *)GetWindowLong(hWnd,0);
	if (TE == NULL)
		return DefWindowProc(hWnd, iMessage, wParam, lParam);

	switch	(iMessage)
	{
		case WM_KEYDOWN:
		{
			switch (wParam)
			{
				case VK_HOME:
					{
					InvalidateRect(hWnd, NULL, TRUE);

					// reset all vars
					TE->TimeStart = 0.0f;
					TE->TimeEnd	= 10.0f;
					TE->CurrTime = 0.0f;
					break;
					}

				case VK_RIGHT:
					{
					#define	SCROLL_SCALE 0.5f
					float len = (TE->TimeEnd - TE->TimeStart);

					InvalidateRect(hWnd, NULL, TRUE);

					TE->TimeStart	+= len * SCROLL_SCALE;
					TE->TimeEnd		+= len * SCROLL_SCALE;

					break;
					}

				case VK_LEFT:
					{
					float len = (TE->TimeEnd - TE->TimeStart);

					InvalidateRect(hWnd, NULL, TRUE);

					if (TE->TimeStart - (len * SCROLL_SCALE) < 0)
						{
						TE->TimeStart = 0.0f;
						TE->TimeEnd = len;
						}
					else
						{
						TE->TimeStart	-= len * SCROLL_SCALE;
						TE->TimeEnd		-= len * SCROLL_SCALE;
						}

					break;
					}

				case VK_ADD:
					{
					float len = TE->TimeEnd - TE->TimeStart;
					TE->TimeEnd = TE->TimeStart + len*2;
					InvalidateRect(hWnd, NULL, TRUE);
					break;
					}

				case VK_SUBTRACT:
					{
					float len = TE->TimeEnd - TE->TimeStart;
					TE->TimeEnd = TE->TimeStart + len/2;
					InvalidateRect(hWnd, NULL, TRUE);
					break;
					}

				case VK_INSERT :
					TimeEdit_InsertTime(TE);
					InvalidateRect(hWnd, NULL, TRUE);
					break;

				case VK_DELETE :

					if (TimeEdit_DelKey(TE) == GE_FALSE)
						{
						geErrorLog_AddString(-1,"TimeEdit_WndProc: TimeEdit_DelKey() failed.", NULL);
						break; // error occured
						}
					if (TimeEdit_DelEvent(TE) == GE_FALSE)
						{
						geErrorLog_AddString(-1,"TimeEdit_WndProc: TimeEdit_DelEvent() failed.", NULL);
						break; // error occured
						}	

					InvalidateRect(hWnd, NULL, TRUE);
					break;
			}
			break;
		}

		case WM_PAINT:
			{
				PAINTSTRUCT PaintStruct;
				BeginPaint(hWnd,&PaintStruct);

				TimeEdit_Paint(TE);

				EndPaint(hWnd,&PaintStruct);
				break;
			}

		case WM_MOUSEMOVE:
			{
			if (wParam & MK_LBUTTON)
				{
				// set x,y for drag
				// the paint function will use these
				TE->DragX = LOWORD(lParam);  
				TE->DragY = HIWORD(lParam);  

				if (TimeEdit_AnimateSelected(TE) == GE_FALSE)
					{
					geErrorLog_AddString(-1,"TimeEdit_WndProc: TimeEdit_AnimateSelected() failed.", NULL);
					break; //error
					}

				// let the paint function go
				InvalidateRect(hWnd, NULL, TRUE);
				}
			break;
			}

		case WM_RBUTTONDOWN:
			{
				int MouseX;
				int MouseY;

				InvalidateRect(hWnd, NULL, TRUE);

				MouseX = LOWORD(lParam);  // horizontal position of cursor 
				MouseY = HIWORD(lParam);  // vertical position 

				if (TimeEdit_IsEventArea(TE, MouseX, MouseY) == GE_TRUE)
					{
					if (TimeEdit_AddEvent(TE, MouseX) == GE_FALSE)
						{
						geErrorLog_AddString(-1,"TimeEdit_WndProc: TimeEdit_AddEvent() failed.", NULL);
						break; //error
						}
					}
				else
					{
					if (TimeEdit_AddKey(TE, MouseX) == GE_FALSE)
						{
						geErrorLog_AddString(-1,"TimeEdit_WndProc: TimeEdit_AddKey() failed.", NULL);
						break; //error
						}
					}

				break;
			}
		
		case WM_LBUTTONUP:
			{
			int MouseX;
			int MouseY;

			MouseX = LOWORD(lParam);  // horizontal position of cursor 
			MouseY = HIWORD(lParam);  // vertical position 

			// only move if we were dragging
			if (TE->Dragging)
				{
				if (TimeEdit_MoveSelectedKeys(TE, MouseX, MouseY) == GE_FALSE)
					{
					geErrorLog_AddString(-1,"TimeEdit_WndProc: TimeEdit_MoveSelectedKeys() failed.", NULL);
					break; //error
					}
				}

			// Turn Dragging off and kill the DragTimer
			TE->Dragging = GE_FALSE;
			KillTimer(hWnd, DRAG_TIMER);

			InvalidateRect(hWnd, NULL, TRUE);
			break;
			}

		case WM_TIMER:
			{
			switch (wParam)
				{
				// only allow dragging after small amount of time
				// otherwise its just a "selection" and not a drag
				case DRAG_TIMER:
					TE->Dragging = GE_TRUE;
					KillTimer(hWnd, DRAG_TIMER);
					break;
				}
			break;
			}

		case WM_LBUTTONDOWN:
			{
			int MouseX;
			int MouseY;

			TE->DragStartX = MouseX = LOWORD(lParam);  // horizontal position of cursor 
			TE->DragStartY = MouseY = HIWORD(lParam);  // vertical position 

			// start drag timer determine whether this click will be a selection or a drag
			SetTimer(hWnd, DRAG_TIMER, DRAG_DEBOUCE, NULL);

			if (wParam & MK_CONTROL)
				{
				if (TimeEdit_SelectMultiTest(TE, MouseX, MouseY) == GE_FALSE)
					break;
				}
			else
				{
				if (TimeEdit_Select(TE, MouseX, MouseY) == GE_FALSE)
					break;
				}

			InvalidateRect(hWnd, NULL, TRUE);
			break;
			}

		case WM_COMMAND:
			{
			switch (HIWORD(wParam))
				{
				case BN_CLICKED:
					{
					}
				}
			break;
			}

		default:
			return DefWindowProc(hWnd, iMessage, wParam, lParam);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Local Functions

static float TimeEdit_XToTime(TimeEdit* TE, int MouseX)
{
	int NewX;
	float NewTime;
	float TimeDiff;
	
	assert(TE);

	TimeDiff = TE->TimeEnd - TE->TimeStart;

	NewX = MouseX - TE->PositionX;
	NewTime = TE->TimeStart + (NewX * (TimeDiff/TE->SizeX));

	if (NewTime < 0)
		NewTime = 0;
	if (NewTime > TE->TimeEnd)
		NewTime = TE->TimeEnd;

	return NewTime;
}

static int TimeEdit_TimeToX(TimeEdit* TE, float Time)
{
	int PosX;
	float TimeDiff;
	
	assert(TE);

	TimeDiff = TE->TimeEnd - TE->TimeStart;

	PosX = TE->PositionX + (int)(TE->SizeX * ((Time - TE->TimeStart)/TimeDiff));

	return PosX;
}

static geBoolean TimeEdit_SetPosition(TimeEdit *TE)
{
	RECT Rect;
	assert( TE != NULL );
	if (GetClientRect(TE->hwnd,&Rect)==0)
		{
			geErrorLog_Add(GE_ERR_WINDOWS_API_FAILURE,"TimeEdit_SetPosition:  GetWindowRect failed");
			return GE_FALSE;
		}
	TE->WindowAcross = (Rect.right -Rect.left);
	TE->WindowDown   = (Rect.bottom-Rect.top);
	TE->SizeX = TE->WindowAcross - ((TE->WindowAcross)*TIMEEDIT_BORDERX*2)/100;
	TE->SizeY = TE->WindowDown   - ((TE->WindowDown  )*TIMEEDIT_BORDERY*2)/100;
	TE->PositionX = (TE->WindowAcross - TE->SizeX) / 2;
	TE->PositionY = (TE->WindowDown) / 2;
	return GE_TRUE;
}

static geBoolean TimeEdit_IsEventArea(TimeEdit *TE, int MouseX, int MouseY)
{
	assert(TE);

	return (MouseY > TE->PositionY);
}

static geBoolean TimeEdit_SelectTest(TimeEdit *TE, float TimeKey, int MouseX, int MouseY, int Yoff)
{
	int x_ul, y_ul, x_lr, y_lr;
	int PosX;

	assert(TE);

	PosX = TimeEdit_TimeToX(TE, TimeKey);

	x_ul = PosX - ELLIPSE_SEL_SIZE;
	y_ul = TE->PositionY - ELLIPSE_SEL_SIZE + Yoff;

	x_lr = PosX + ELLIPSE_SEL_SIZE;
	y_lr = TE->PositionY + ELLIPSE_SEL_SIZE + Yoff;

	if (MouseX < x_ul)
		return GE_FALSE;
	if (MouseY < y_ul)
		return GE_FALSE;
	if (MouseX > x_lr)
		return GE_FALSE;
	if (MouseY > y_lr)
		return GE_FALSE;

	return GE_TRUE;
}

static geBoolean TimeEdit_SelectTimeBarTest(TimeEdit *TE, int MouseX, int MouseY)
{
	int x_ul, y_ul, x_lr, y_lr;
	int PosX;

	assert(TE);

	PosX = TimeEdit_TimeToX(TE, TE->CurrTime);

	x_ul = PosX - 4;
	y_ul = TE->PositionY - TIMEBAR_HEIGHT;

	x_lr = PosX + 4;
	y_lr = TE->PositionY + TIMEBAR_HEIGHT;

	if (MouseX < x_ul)
		return GE_FALSE;
	if (MouseY < y_ul)
		return GE_FALSE;
	if (MouseX > x_lr)
		return GE_FALSE;
	if (MouseY > y_lr)
		return GE_FALSE;

	return GE_TRUE;
}

static geBoolean TimeEdit_SelectTimeLineTest(TimeEdit *TE, int MouseX, int MouseY)
{
	int x_ul, y_ul, x_lr, y_lr;
	int PosX;

	assert(TE);

	PosX = TimeEdit_TimeToX(TE, TE->CurrTime);

	x_ul = TE->PositionX;
	y_ul = TE->PositionY - 4;

	x_lr = TE->PositionX + TE->SizeX;
	y_lr = TE->PositionY + 4;

	if (MouseX < x_ul)
		return GE_FALSE;
	if (MouseY < y_ul)
		return GE_FALSE;
	if (MouseX > x_lr)
		return GE_FALSE;
	if (MouseY > y_lr)
		return GE_FALSE;

	return GE_TRUE;
}

static geBoolean TimeEdit_AnimateSelected(TimeEdit *TE)
{
	float Time;

	assert(TE);

	if (TE->TimeBarSelected)
		{
		Time = TimeEdit_XToTime(TE, TE->DragX);
		if (TE->ParentCB(TE->CBdata, TE_TIME_CHG, Time, NULL) == GE_FALSE)
			return GE_FALSE;
		}

	return GE_TRUE;
}

static geBoolean TimeEdit_Select(TimeEdit *TE, int MouseX, int MouseY)
{
	int i;

	assert(TE);

	// clear keys selected
	TE->KeysSelected[0] = NULL;

	for (i = 0; i < TE->KeyCount; i++)
	{	
		if (TimeEdit_SelectTest(TE, TE->KeyList[i], MouseX, MouseY, TIMEKEY_ELLIPSE_YOFF) == GE_TRUE)
			{
			// select single key
			TE->KeysSelected[0] = &TE->KeyList[i];
			// pass the first key only
			TE->ParentCB(TE->CBdata, TE_SEL_KEY, TE->KeyList[i], NULL);
			}
		}

	TE->EventsSelected[0] = NULL;

	for (i = 0; i < TE->EventCount; i++)
		{
		if (TimeEdit_SelectTest(TE, TE->EventList[i], MouseX, MouseY, EVENT_ELLIPSE_YOFF) == GE_TRUE)
			{
			// select single key
			TE->EventsSelected[0] = &TE->EventList[i];
			TE->ParentCB(TE->CBdata, TE_SEL_EVENT, TE->EventList[i], NULL);
			}
		}

	TE->TimeBarSelected = GE_FALSE;

	// only try selecting the timebar or timeline if we aren't selecting a key/event
	if (TE->KeysSelected[0] == NULL && TE->EventsSelected[0] == NULL)
		{	
		if (TimeEdit_SelectTimeBarTest(TE, MouseX, MouseY))
			{
			TE->TimeBarSelected = GE_TRUE;
			TE->ParentCB(TE->CBdata, TE_TIME_CHG, TE->CurrTime, NULL);
			}
		else
		if (TimeEdit_SelectTimeLineTest(TE, MouseX, MouseY))
			{
			}
		else
			{
			// move the current time
			TE->CurrTime = TimeEdit_XToTime(TE, MouseX);
			TE->ParentCB(TE->CBdata, TE_TIME_CHG, TE->CurrTime, NULL);
			}
		}

	return GE_TRUE;
}

static geBoolean TimeEdit_SelectMultiTest(TimeEdit *TE, int MouseX, int MouseY)
{
	int i;
	int s;
	geBoolean Found;

	assert(TE);

	for (i = 0; i < TE->KeyCount; i++)
	{
		if (TimeEdit_SelectTest(TE, TE->KeyList[i], MouseX, MouseY, TIMEKEY_ELLIPSE_YOFF) == GE_TRUE)
		{
		Found = GE_FALSE;
		for (s = 0; TE->KeysSelected[s] != NULL; s++)
			{
			if (TE->KeysSelected[s] == &TE->KeyList[i])
				Found = GE_TRUE;
			}
	
		if (!Found)
			TE->KeysSelected[s] = &TE->KeyList[i];
		}
	}

	return GE_TRUE;
}

static geBoolean TimeEdit_AddKey(TimeEdit* TE, int MouseX)
{
	float Time, ReturnTime;

	assert(TE);

	// add a new key
	Time = TimeEdit_XToTime(TE, MouseX);

	if (TE->ParentCB(TE->CBdata, TE_INS_KEY, Time, &ReturnTime) == GE_FALSE)
		{
		geErrorLog_AddString(-1,"TimeEdit_AddKey: TE_INS_KEY failed.", NULL);
		return GE_FALSE;
		}

	TE->KeyList[TE->KeyCount] = ReturnTime;
	if (TE->KeyCount++ >= MAX_KEYS)
		{
		geErrorLog_AddString(-1,"TimeEdit_AddKey: Key limit hit.", NULL);
		// send error
		TE->ParentCB(TE->CBdata, TE_INTERNAL_ERROR, -1.0f, NULL);
		return GE_FALSE;
		}

	return GE_TRUE;
}

static geBoolean TimeEdit_DelKey(TimeEdit* TE)
{
	int i;

	assert(TE);

	// need to extend this to delete multiple keys
	for (i = 0; i < TE->KeyCount; i++)
	{
		if (TE->KeysSelected[0] == &TE->KeyList[i])
			{
			if (TE->ParentCB(TE->CBdata, TE_DEL_KEY, TE->KeyList[i], NULL) == GE_FALSE)
				{
				geErrorLog_AddString(-1,"TimeEdit_DelKey: TE_DEL_KEY failed.", NULL);
				return GE_FALSE;
				}

			TE->KeyList[i] = TE->KeyList[TE->KeyCount-1];
			TE->KeyCount--;
			assert(TE->KeyCount >= 0);
			i--;
			break;
			}
	}

	// erase selection list
	TE->KeysSelected[0] = NULL;

	return GE_TRUE;
}

static geBoolean TimeEdit_AddEvent(TimeEdit* TE, int MouseX)
{
	float Time, ReturnTime;

	assert(TE);

	// add a new Event
	Time = TimeEdit_XToTime(TE, MouseX);

	if (TE->ParentCB(TE->CBdata, TE_INS_EVENT, Time, &ReturnTime) == GE_FALSE)
		{
		geErrorLog_AddString(-1,"TimeEdit_InitAddEvent: TE_INS_EVENT failed.", NULL);
		return GE_FALSE;
		}

	TE->EventList[TE->EventCount] = ReturnTime;
	if (TE->EventCount++ >= MAX_EVENTS)
		{
		geErrorLog_AddString(-1,"TimeEdit_InitAddEvent: Event limit hit.", NULL);
		// send error
		TE->ParentCB(TE->CBdata, TE_INTERNAL_ERROR, -1.0f, NULL);
		return GE_FALSE;
		}

	return GE_TRUE;
}

static geBoolean TimeEdit_DelEvent(TimeEdit* TE)
{
	int i;

	assert(TE);

	// need to extend this to delete multiple Events
	for (i = 0; i < TE->EventCount; i++)
	{
		if (TE->EventsSelected[0] == &TE->EventList[i])
			{
			if (TE->ParentCB(TE->CBdata, TE_DEL_EVENT, TE->EventList[i], NULL) == GE_FALSE)
				{
				geErrorLog_AddString(-1,"TimeEdit_DelKey: TE_DEL_EVENT failed.", NULL);
				return GE_FALSE;
				}

			TE->EventList[i] = TE->EventList[TE->EventCount-1];
			TE->EventCount--;
			i--;
			break;
			}
	}

	// erase selection list
	TE->EventsSelected[0] = NULL;

	return GE_TRUE;
}


static geBoolean TimeEdit_MoveSelectedKeys(TimeEdit *TE, int MouseX, int MouseY)
{
	int i;
	float Time,MouseAtTime;

	assert(TE);

	for (i = 0; TE->KeysSelected[i] != NULL; i++)
		{
		Time = *TE->KeysSelected[i];
		MouseAtTime = TimeEdit_XToTime(TE, MouseX);

		if (MouseAtTime < 0.0f)
			MouseAtTime = 0.0f;

		if (MouseAtTime > TE->TimeEnd)
			MouseAtTime = TE->TimeEnd;

		if (TE->ParentCB(TE->CBdata, TE_MOVE_KEY, Time, &MouseAtTime) == GE_FALSE)
			{
			geErrorLog_AddString(-1,"TimeEdit_MoveSelectedKeys: TE_MOVE_KEY failed.", NULL);
			return GE_FALSE;
			}
		*TE->KeysSelected[i] = MouseAtTime;
		}

	for (i = 0; TE->EventsSelected[i] != NULL; i++)
		{
		Time = *TE->EventsSelected[i];
		MouseAtTime = TimeEdit_XToTime(TE, MouseX);

		// events seem to have a problem being a 0.0f
		if (MouseAtTime <= 0.0001f)
			MouseAtTime = 0.0001f;

		if (MouseAtTime > TE->TimeEnd)
			MouseAtTime = TE->TimeEnd;

		if (TE->ParentCB(TE->CBdata, TE_MOVE_EVENT, Time, &MouseAtTime) == GE_FALSE)
			{
			geErrorLog_AddString(-1,"TimeEdit_MoveSelectedKeys: TE_MOVE_EVENT failed.", NULL);
			return GE_FALSE;
			}
		*TE->EventsSelected[i] = MouseAtTime;
		}

	if (TE->TimeBarSelected)
		{
		Time = TE->CurrTime;
		MouseAtTime = TimeEdit_XToTime(TE, MouseX);

		if (MouseAtTime < 0.0f)
			MouseAtTime = 0.0f;

		if (MouseAtTime > TE->TimeEnd)
			MouseAtTime = TE->TimeEnd;

		TE->CurrTime = MouseAtTime;
		}

	return GE_TRUE;
}

static geBoolean TimeEdit_InsertTime(TimeEdit *TE)
{
	float TimeOffset,Time;
	int i;

	assert(TE);

	if (TE->TimeBarSelected)
		{
		for (i = 0; i < TE->KeyCount; i++)
			{
			Time = TE->KeyList[i];
			TimeOffset = Time + ((TE->TimeEnd - TE->TimeStart)/10);

			if (Time >= TE->CurrTime)
				{
				if (TE->ParentCB(TE->CBdata, TE_MOVE_KEY, Time, &TimeOffset) == GE_FALSE)
					{
					geErrorLog_AddString(-1,"TimeEdit_InsertTime: TE_MOVE_KEY failed.", NULL);
					return GE_FALSE;
					}

				TE->KeyList[i] = TimeOffset;
				}
			}

		for (i = 0; i < TE->EventCount; i++)
			{
			Time = TE->EventList[i];
			TimeOffset = Time + ((TE->TimeEnd - TE->TimeStart)/10);

			if (Time >= TE->CurrTime)
				{
				if (TE->ParentCB(TE->CBdata, TE_MOVE_EVENT, Time, &TimeOffset) == GE_FALSE)
					{
					geErrorLog_AddString(-1,"TimeEdit_InsertTime: TE_MOVE_EVENT failed.", NULL);
					return GE_FALSE;
					}

				TE->EventList[i] = TimeOffset;
				}
			}
		}


	return GE_TRUE;
}


/////////////////////////////////////////////////////////////////////////////////////////////
// Painting functions

static geBoolean TimeEdit_TimeLinePaint(TimeEdit *TE,	HDC hdc)
{
	int i;
	int x_ul, y_ul, x_lr, y_lr;
	int PosX;
	float Time;
	HPEN Pen, OldPen;

	assert(TE);

	// draw time keys
	Pen = GetStockObject(BLACK_BRUSH);
	if (Pen == NULL)
		{	
		geErrorLog_AddString(GE_ERR_WINDOWS_API_FAILURE, "TimeEdit_TimeLinePaint: GetStockObject for pen failed", NULL);
		return GE_FALSE;
		}

	OldPen = SelectObject(hdc,Pen);
	if (OldPen == NULL)
		{	
		geErrorLog_AddString(GE_ERR_WINDOWS_API_FAILURE, "TimeEdit_TimeLinePaint: SelectObject for pen failed", NULL);
		return GE_FALSE;
		}

	for (i = 0; i < TE->KeyCount; i++)
	{
		Time = TE->KeyList[i];
		PosX = TimeEdit_TimeToX(TE, Time);

		if ((PosX < TE->PositionX) || (PosX > (TE->PositionX + TE->SizeX)))
			continue;

		x_ul = PosX - ELLIPSE_SIZE;
		y_ul = TE->PositionY - ELLIPSE_SIZE + TIMEKEY_ELLIPSE_YOFF;

		x_lr = PosX + ELLIPSE_SIZE;
		y_lr = TE->PositionY + ELLIPSE_SIZE + TIMEKEY_ELLIPSE_YOFF;

		Ellipse(hdc, x_ul, y_ul, x_lr, y_lr);

		MoveToEx(hdc, PosX, TE->PositionY + TIMEKEY_ELLIPSE_YOFF, NULL);
		LineTo(hdc, PosX, TE->PositionY);
	}

	// highlight selected keys
	Pen = GetStockObject(GRAY_BRUSH);
	if (Pen == NULL)
		{	
		geErrorLog_AddString(GE_ERR_WINDOWS_API_FAILURE, "TimeEdit_TimeLinePaint: GetStockObject for pen failed", NULL);
		return GE_FALSE;
		}
	OldPen = SelectObject(hdc,Pen);
	if (OldPen == NULL)
		{	
		geErrorLog_AddString(GE_ERR_WINDOWS_API_FAILURE, "TimeEdit_TimeLinePaint: SelectObject for pen failed", NULL);
		return GE_FALSE;
		}

	for (i = 0; TE->KeysSelected[i] != NULL; i++)
		{
		Time = *TE->KeysSelected[i];
		PosX = TimeEdit_TimeToX(TE, Time);

		if ((PosX < TE->PositionX) || (PosX > (TE->PositionX + TE->SizeX)))
			continue;

		x_ul = PosX - ELLIPSE_SIZE;
		y_ul = TE->PositionY - ELLIPSE_SIZE + TIMEKEY_ELLIPSE_YOFF;

		x_lr = PosX + ELLIPSE_SIZE;
		y_lr = TE->PositionY + ELLIPSE_SIZE + TIMEKEY_ELLIPSE_YOFF;

		Ellipse(hdc, x_ul, y_ul, x_lr, y_lr);

		if (TE->Dragging)
			{
			int Offset;

			Offset = TE->DragX - TE->DragStartX;
			PosX += Offset;

			if (PosX < TE->PositionX)
				PosX = TE->PositionX;
			if (PosX > TE->PositionX + TE->SizeX - 1)
				PosX = TE->PositionX + TE->SizeX - 1;

			x_ul = PosX - ELLIPSE_SIZE;
			y_ul = TE->PositionY - ELLIPSE_SIZE + TIMEKEY_ELLIPSE_YOFF;

			x_lr = PosX + ELLIPSE_SIZE;
			y_lr = TE->PositionY + ELLIPSE_SIZE + TIMEKEY_ELLIPSE_YOFF;

			Ellipse(hdc, x_ul, y_ul, x_lr, y_lr);
			}
		}

	// draw time Events
	Pen = GetStockObject(BLACK_BRUSH);
	if (Pen == NULL)
		{	
		geErrorLog_AddString(GE_ERR_WINDOWS_API_FAILURE, "TimeEdit_TimeLinePaint: GetStockObject for pen failed", NULL);
		return GE_FALSE;
		}
	OldPen = SelectObject(hdc,Pen);
	if (OldPen == NULL)
		{	
		geErrorLog_AddString(GE_ERR_WINDOWS_API_FAILURE, "TimeEdit_TimeLinePaint: SelectObject for pen failed", NULL);
		return GE_FALSE;
		}

	for (i = 0; i < TE->EventCount; i++)
	{
		Time = TE->EventList[i];
		PosX = TimeEdit_TimeToX(TE, Time);

		if ((PosX < TE->PositionX) || (PosX > (TE->PositionX + TE->SizeX)))
			continue;

		x_ul = PosX - ELLIPSE_SIZE;
		y_ul = TE->PositionY - ELLIPSE_SIZE + EVENT_ELLIPSE_YOFF;

		x_lr = PosX + ELLIPSE_SIZE;
		y_lr = TE->PositionY + ELLIPSE_SIZE + EVENT_ELLIPSE_YOFF;

		Ellipse(hdc, x_ul, y_ul, x_lr, y_lr);

		MoveToEx(hdc, PosX, TE->PositionY + EVENT_ELLIPSE_YOFF, NULL);
		LineTo(hdc, PosX, TE->PositionY);

	}

	// highlight selected Events
	Pen = GetStockObject(GRAY_BRUSH);
	if (Pen == NULL)
		{	
		geErrorLog_AddString(GE_ERR_WINDOWS_API_FAILURE, "TimeEdit_TimeLinePaint: GetStockObject for pen failed", NULL);
		return GE_FALSE;
		}
	OldPen = SelectObject(hdc,Pen);
	if (OldPen == NULL)
		{	
		geErrorLog_AddString(GE_ERR_WINDOWS_API_FAILURE, "TimeEdit_TimeLinePaint: SelectObject for pen failed", NULL);
		return GE_FALSE;
		}

	for (i = 0; TE->EventsSelected[i] != NULL; i++)
		{
		Time = *TE->EventsSelected[i];
		PosX = TimeEdit_TimeToX(TE, Time);

		if ((PosX < TE->PositionX) || (PosX > (TE->PositionX + TE->SizeX)))
			continue;
		
		x_ul = PosX - ELLIPSE_SIZE;
		y_ul = TE->PositionY - ELLIPSE_SIZE + EVENT_ELLIPSE_YOFF;

		x_lr = PosX + ELLIPSE_SIZE;
		y_lr = TE->PositionY + ELLIPSE_SIZE + EVENT_ELLIPSE_YOFF;

		Ellipse(hdc, x_ul, y_ul, x_lr, y_lr);

		if (TE->Dragging)
			{
			int Offset;

			Offset = TE->DragX - TE->DragStartX;
			PosX += Offset;

			if (PosX < TE->PositionX)
				PosX = TE->PositionX;
			if (PosX > TE->PositionX + TE->SizeX - 1)
				PosX = TE->PositionX + TE->SizeX - 1;

			x_ul = PosX - ELLIPSE_SIZE;
			y_ul = TE->PositionY - ELLIPSE_SIZE + EVENT_ELLIPSE_YOFF;

			x_lr = PosX + ELLIPSE_SIZE;
			y_lr = TE->PositionY + ELLIPSE_SIZE + EVENT_ELLIPSE_YOFF;

			Ellipse(hdc, x_ul, y_ul, x_lr, y_lr);
			}
		}

	Pen = GetStockObject(TE->TimeBarSelected ? GRAY_BRUSH : BLACK_BRUSH);
	if (Pen == NULL)
		{	
		geErrorLog_AddString(GE_ERR_WINDOWS_API_FAILURE, "TimeEdit_TimeLinePaint: GetStockObject for pen failed", NULL);
		return GE_FALSE;
		}
	OldPen = SelectObject(hdc,Pen);
	if (OldPen == NULL)
		{	
		geErrorLog_AddString(GE_ERR_WINDOWS_API_FAILURE, "TimeEdit_TimeLinePaint: SelectObject for pen failed", NULL);
		return GE_FALSE;
		}

	if (TE->TimeBarSelected && TE->Dragging)
		{
		PosX = TimeEdit_TimeToX(TE, TE->CurrTime) + (TE->DragX - TE->DragStartX);
		if (PosX < TE->PositionX)
			PosX = TE->PositionX;
		if (PosX > TE->PositionX + TE->SizeX - 1)
			PosX = TE->PositionX + TE->SizeX - 1;
		}
	else
		PosX = TimeEdit_TimeToX(TE, TE->CurrTime);

	x_ul = PosX - ELLIPSE_SIZE;
	y_ul = TE->PositionY - ELLIPSE_SIZE - TIMEBAR_HEIGHT/2;

	x_lr = PosX + ELLIPSE_SIZE;
	y_lr = TE->PositionY + ELLIPSE_SIZE - TIMEBAR_HEIGHT/2;

	Ellipse(hdc, x_ul, y_ul, x_lr, y_lr);
	y_ul += TIMEBAR_HEIGHT;
	y_lr += TIMEBAR_HEIGHT;
	Ellipse(hdc, x_ul, y_ul, x_lr, y_lr);

	MoveToEx(hdc, PosX, TE->PositionY + TIMEBAR_HEIGHT/2, NULL);
	LineTo(hdc, PosX, TE->PositionY - TIMEBAR_HEIGHT/2);

	return GE_TRUE;
}


static geBoolean TimeEdit_Paint(TimeEdit *TE)
{
	HDC hdc;
	HPEN Pen;
	HPEN OldPen;
	int i;

	assert( TE ) ;

	if (TimeEdit_SetPosition(TE)==GE_FALSE)
		{
		geErrorLog_Add(GE_ERR_SUBSYSTEM_FAILURE,"TimeEdit_Paint:  TimeEdit_SetPosition failed");
		return GE_FALSE;
		}

	hdc = GetDC(TE->hwnd);
	if (hdc == NULL)
		{
		geErrorLog_Add(GE_ERR_WINDOWS_API_FAILURE,"TimeEdit_Paint:  GetDC for drawing failed");
		return GE_FALSE;
		}
	
	Pen = GetStockObject(BLACK_BRUSH);
	if (Pen == NULL)
		{
		geErrorLog_Add(GE_ERR_WINDOWS_API_FAILURE,"TimeEdit_Paint:  GetStockObject for pen failed");
		ReleaseDC(TE->hwnd,hdc);
		return GE_FALSE;
		}
				
	OldPen = SelectObject(hdc,Pen);
	if (OldPen == NULL)
		{
		geErrorLog_Add(GE_ERR_WINDOWS_API_FAILURE,"TimeEdit_Paint:  SelectObject for pen failed");
		ReleaseDC(TE->hwnd,hdc);
		return GE_FALSE;
		}
			
	MoveToEx(hdc,TE->PositionX,TE->PositionY,NULL);
	LineTo(hdc,TE->PositionX + TE->SizeX,TE->PositionY);

	// draw tic marks
	for (i = 0; i <= 10; i++)
		{
		char str[64];
		float ThisTime,TimeIncrement;
		int PosX;

		TimeIncrement = (TE->TimeEnd - TE->TimeStart) / 10;

		ThisTime = TE->TimeStart + (i * TimeIncrement);

		PosX = TimeEdit_TimeToX(TE, ThisTime);

		if (TE->TimeEnd - TE->TimeStart <= 0.01f)
			sprintf(str,"%.4f",ThisTime);
		else
		if (TE->TimeEnd - TE->TimeStart <= 0.02f)
			sprintf(str,"%.3f",ThisTime);
		else
		if (TE->TimeEnd - TE->TimeStart < 2.0f)
			sprintf(str,"%.2f",ThisTime);
		else
			sprintf(str,"%.1f",ThisTime);

		TextOut(hdc, PosX - (strlen(str) * 3), TE->PositionY + TIMELINE_TEXT_YOFF, str, strlen(str));

		MoveToEx(hdc,PosX,TE->PositionY-3,NULL);
		LineTo(hdc,PosX,TE->PositionY+3);
		}

	// draw time line data
	if (TimeEdit_TimeLinePaint(TE, hdc) == GE_FALSE)
		{	
		geErrorLog_AddString(GE_ERR_WINDOWS_API_FAILURE, "TimeEdit_Paint: TimeLinePaint failed", NULL);
		ReleaseDC(TE->hwnd,hdc);
		return GE_FALSE;
		}

	SelectObject(hdc,OldPen);
	ReleaseDC(TE->hwnd,hdc);
	return GE_TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Public Functions

void TimeEdit_Clear(TimeEdit* TE)
{
	assert(TE);

	TE->KeyCount = 0;
	TE->EventCount = 0;
	TE->CurrTime = 0.0f;
	TE->KeysSelected[0] = NULL;
	TE->EventsSelected[0] = NULL;
	TE->Dragging = GE_FALSE;
	//TE->TimeStart = 0.0f;
	//TE->TimeEnd = 10.0f;
	InvalidateRect(TE->hwnd, NULL, TRUE);
}

geBoolean TimeEdit_InitAddKey(TimeEdit* TE, float Time)
{
	assert(TE);

	TE->KeyList[TE->KeyCount] = Time;
	if (TE->KeyCount++ >= MAX_KEYS)
		{
		geErrorLog_AddString(-1,"TimeEdit_InitAddKey: Key limit hit.", NULL);
		return GE_FALSE;
		}
	return GE_TRUE;
}

geBoolean TimeEdit_InitAddEvent(TimeEdit* TE, float Time)
{
	assert(TE);

	TE->EventList[TE->EventCount] = Time;
	if (TE->EventCount++ >= MAX_EVENTS)
		{
		geErrorLog_AddString(-1,"TimeEdit_InitAddEvent: Event limit hit.", NULL);
		return GE_FALSE;
		}
	return GE_TRUE;
}


void TimeEdit_Destroy(TimeEdit *TE)
{
	assert(TE);

	if (TE->hwnd)
		{
		DestroyWindow(TE->hwnd);
		}

	TimeEdit_RegisterCount--;
	if (TimeEdit_RegisterCount==0)
		{
		UnregisterClass(TIMEEDIT_WINDOW_CLASS,TE->hinst);
		}

	if (TE)
		{
		geRam_Free(TE);
		}
}

HWND TimeEdit_CreateWindow(HANDLE hInstance, HWND hwndParent, TE_CB CallbackFunc, void *CallbackData, 
						   char *AppName, int32 Width, int32 Height)
{
	WNDCLASS		wc;
	HWND			hWnd=0,bn=0;
	TimeEdit		*TE=0;

	assert(CallbackFunc);
	assert(CallbackData);

	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = TimeEdit_WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = sizeof(TimeEdit *);
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(hInstance, IDI_APPLICATION);
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName  = (const char*)NULL;
	wc.lpszClassName = TIMEEDIT_WINDOW_CLASS;

	if (TimeEdit_RegisterCount==0)
		{
			if (RegisterClass(&wc)==0)
				{
					geErrorLog_AddString(GE_ERR_WINDOWS_API_FAILURE,"TimeEdit_CreateWindow: RegisterClass for TimeEdit window",geErrorLog_IntToString(GetLastError));
					goto Error;
				}
		}
	TimeEdit_RegisterCount++;

	hWnd = CreateWindowEx(
		0,
		wc.lpszClassName,
		AppName,
		WS_BORDER | WS_CAPTION | WS_SIZEBOX,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		Width,
		Height,
		hwndParent,
		NULL,
		hInstance,
		NULL);

	if (!hWnd)
		{
			geErrorLog_AddString(GE_ERR_WINDOWS_API_FAILURE,"TimeEdit_CreateWindow: CreateWindowEx failed for TimeEdit window",geErrorLog_IntToString(GetLastError));
			goto Error;
		}


	TE = GE_RAM_ALLOCATE_STRUCT(TimeEdit);

	if (TE == NULL)
		{
			geErrorLog_AddString(GE_ERR_MEMORY_RESOURCE,"TimeEdit_CreateWindow:  Couldn't create TimeEdit object",NULL);
			goto Error;
		}

	TE->ParentCB = CallbackFunc;
	TE->TimeEnd = 10.0f;
	TE->TimeStart = 0.0f;
	TE->CurrTime = 0.0f;
	TE->CBdata = CallbackData;
	TE->Dragging = GE_FALSE;
	memset(TE->KeysSelected, 0, sizeof(TE->KeysSelected));
	memset(TE->EventsSelected, 0, sizeof(TE->EventsSelected));
	TE->KeyCount = 0;
	TE->EventCount = 0;
	TE->TimeBarSelected = GE_FALSE;

	TE->hwnd = hWnd;
	TE->hinst = hInstance;
	SetLastError(0);
	if (SetWindowLong(hWnd,0,(long)TE)==0)
		{
		if (GetLastError()!=0)
			{
			geErrorLog_AddString(GE_ERR_WINDOWS_API_FAILURE,"TimeEdit_CreateWindow:  SetWindowLong failed",geErrorLog_IntToString(GetLastError));
			goto Error;
			}
		}

	UpdateWindow(hWnd);
	ShowWindow(hWnd, SW_SHOWNORMAL);

	return hWnd;

	Error:
	if (hWnd)
		{
		DestroyWindow(hWnd);
		}

	TimeEdit_RegisterCount--;
	if (TimeEdit_RegisterCount==0)
		{
		UnregisterClass(TIMEEDIT_WINDOW_CLASS,hInstance);
		}

	if (TE)
		{
		geRam_Free(TE);
		}
	return NULL;
}
