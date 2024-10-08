/****************************************************************************************/
/*  Procedural.h	                                                                    */
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
#ifndef PROCEDURAL_H
#define PROCEDURAL_H

#include "Bitmap.h"

#ifdef __cplusplus
extern "C" {
#endif

//====================================================================================
//====================================================================================
//====================================================================================
//====================================================================================
typedef struct Procedural	Procedural;

typedef Procedural *PROC_CREATE(geBitmap **Bitmap, const char *ParmStart);
typedef void PROC_DESTROY(Procedural *Proc);
typedef geBoolean PROC_ANIMATE(Procedural *Proc, float ElpapsedTime); // ElapsedTime in Millisecs

#define Cur_Procedurals_Version	(0)

typedef struct
{
	uint32			Procedurals_Version;

	const char		Name[256];

	// Init/Destroy funcs
	PROC_CREATE		*Create;
	PROC_DESTROY	*Destroy;

	// Access funcs
	PROC_ANIMATE	*Animate;

} Procedural_Table;

#ifdef __cplusplus
}
#endif

#endif