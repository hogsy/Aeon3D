/****************************************************************************************/
/*  Cursor.h                                                                            */
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

#ifndef CURSOR_H
#define CURSOR_H

#include "genesis.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Cursor Cursor;

void	Cursor_Destroy(Cursor *curs);
Cursor *Cursor_Create (geEngine *Engine, const int ResourceId);
void	Cursor_SetPosition(Cursor *curs, int X, int Y);
void	Cursor_GetPosition(Cursor *curs, int *X, int *Y);
void	Cursor_Render(Cursor *curs);

#ifdef __cplusplus
}
#endif

#endif