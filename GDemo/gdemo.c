/****************************************************************************************/
/*  GDemo.c                                                                             */
/*                                                                                      */
/*  Author: Frank Maddin                                                                */
/*  Description:    Windows startup code and main program loop  		                */
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
#include <windows.h>
#pragma warning(disable : 4201 4214 4115)
#include <mmsystem.h>
#pragma warning(default : 4201 4214 4115)

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>

#include "App.h"
#include "errorlog.h"
#include "resrc1.h"


#pragma warning (disable:4514)	// unreferenced inline function (caused by Windows)


//=====================================================================================
//	WndProc
//=====================================================================================
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	return (App_WndProc(hWnd, iMessage, wParam, lParam));
}

static HWND CreateMainWindow(HANDLE hInstance, char *AppName, int32 Width, int32 Height)
{
	WNDCLASS		wc;
	HWND			hWnd;
	RECT			WindowRect;
	int				Style;

	//
	// Set up and register application window class
	//

	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	//wc.hIcon         = IDI_ICON1;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName  = (const char*)NULL;
	wc.lpszClassName = AppName;

	RegisterClass(&wc);

	Style = WS_OVERLAPPEDWINDOW;
	//
	// Create application's main window
	//

	hWnd = CreateWindowEx(
		0,
		AppName,
		AppName,
		Style,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		hInstance,
		NULL);

	if (!hWnd)
	{
		MessageBox(0, "Could not create window.", "** ERROR **", MB_OK);
		_exit(1);
	}	


	UpdateWindow(hWnd);
	SetFocus(hWnd);

	// Adjust window size so that the client area is Width x Height
	WindowRect.left = 0;
	WindowRect.top = 0;
	WindowRect.right = Width - 1;
	WindowRect.bottom = Height - 1;
	AdjustWindowRect (&WindowRect, Style, FALSE);
	
	{
		int WindowWidth = WindowRect.right - WindowRect.left + 1;
		int WindowHeight = WindowRect.bottom - WindowRect.top + 1;
 
		SetWindowPos 
		(
			hWnd, 0,
			0, 0, 
			WindowWidth, WindowHeight,
			SWP_NOCOPYBITS | SWP_NOZORDER
		);
	}

	//
	// Make window visible
	//
	ShowWindow(hWnd, SW_SHOWNORMAL);

	return hWnd;

}

//=====================================================================================
//	WinMain
//=====================================================================================
#pragma warning (disable:4100)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
				   LPSTR lpszCmdParam, int nCmdShow)
{
	int Width, Height, Running;
	HWND hWnd;
	MSG				Msg;
	App *App;

	// Create the app's main window		
	Width = 640;
	Height = 480;
	hWnd = CreateMainWindow(hInstance, "GDemo", Width, Height);

	// Create the application
	App = App_Create(hInstance, hWnd, lpszCmdParam);

	Running = TRUE;

	while (Running)
	{

		// Update the application
		if (App_Update(App) == GE_FALSE)
			break;

		// message pump
		while (PeekMessage( &Msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if (!GetMessage(&Msg, NULL, 0, 0 ))
			{
				PostQuitMessage(0);
				Running=0;
				break;
			}
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}
	}

	// Application clean exit
	App_Shutdown(App);

	return 0;
}
#pragma warning (default:4100)
