/****************************************************************************************/
/*  VidMode.c                                                                           */
/*                                                                                      */
/*  Author: Mike Sandige                                                                */
/*  Description:                                                                        */
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

#include "VidMode.h"

#define WIDTH(V)  ((V)>>16)
#define HEIGHT(V) ((V)&0xFFFF)
#define PACK(W,H)  (((W)<<16) + (H))

void VidMode_GetResolution (const VidMode V, int *Width, int *Height)
{
	int W,H;
	assert( Width  != NULL );
	assert( Height != NULL );
	W = WIDTH(V);
	H = HEIGHT(V);
	
	assert( W >= 320  );
	assert( W <  8000 );
	assert( H >= 200  );
	assert( H <  8000 ); 

	*Width = W;
	*Height = H;
}
	
geBoolean VidMode_SetResolution (VidMode *V, int Width, int Height)
{
	assert( V != NULL );
	
	if	(	 	( Width  < 320  ) 
			||  ( Width  > 8000 ) 
			||  ( Height < 200  ) 
			||  ( Height > 8000 )
		)
		return GE_FALSE;
	
	*V = PACK(Width,Height);
	return GE_TRUE;
}

