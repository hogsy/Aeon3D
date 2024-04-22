/****************************************************************************************/
/*  Gui.h                                                                               */
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
#ifndef GUI_H
#define GUI_H

#include "genesis.h"

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////////////////////////////////
//	Gui pointer
////////////////////////////////////////////////////////////////////////////////////////
typedef struct	Gui	Gui;


////////////////////////////////////////////////////////////////////////////////////////
//	Prototypes
////////////////////////////////////////////////////////////////////////////////////////

//	Destroy the demos gui.
//
////////////////////////////////////////////////////////////////////////////////////////
void Gui_Destroy(
	Gui	**DeadAppGui );	// gui to destroy

//	Create the demos gui.
//
////////////////////////////////////////////////////////////////////////////////////////
Gui * Gui_Create(
	geEngine	*Engine,		// engine to use
	int			WindowWidth,	// window width
	int			WindowHeight );	// window height

//	Process a frame for the gui.
//
////////////////////////////////////////////////////////////////////////////////////////
geBoolean Gui_Frame(
	Gui			*AppGui,	// gui to process
	geCamera	*Camera );	// current camera

//	Let gui know about the current cursor pos.
//
////////////////////////////////////////////////////////////////////////////////////////
void Gui_SetCursorPos(
	Gui	*AppGui,	// gui to update
	int	CursorX,	// cursor x
	int	CursorY );	// cursor y

//	Get currently active bg.
//
////////////////////////////////////////////////////////////////////////////////////////
int Gui_GetActiveBg(
	Gui	*AppGui );	// gui whose active bg we want

//	Is the gui currently active or not.
//
////////////////////////////////////////////////////////////////////////////////////////
geBoolean Gui_IsActive(
	Gui	*AppGui );	// gui whose status we want

//	Set gui status.
//
////////////////////////////////////////////////////////////////////////////////////////
void Gui_SetActiveStatus(
	Gui			*AppGui,		// gui whose active status we want to set
	geBoolean	NewStatus );	// new active status


//	Let gui know if the cursor needs to be drawn.
//
////////////////////////////////////////////////////////////////////////////////////////
void Gui_ShowCursor(
	Gui	*AppGui,		// gui to update
	geBoolean State);	// cursor state

//	Prepare for first frame
//
////////////////////////////////////////////////////////////////////////////////////////
geBoolean Gui_PrepareFrame(
	Gui			*AppGui,	// gui to process
	geCamera	*Camera );	// current camera


#ifdef __cplusplus
	}
#endif

#endif
