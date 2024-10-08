/****************************************************************************************/
/*  AutoSelect.c                                                                        */
/*                                                                                      */
/*  Author: Mike Sandige                                                                */
/*  Description:    Attempts to automatically choose a good video mode                  */
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

#include    <stdlib.h>  // qsort 
#include	<assert.h>
#include    <Windows.h>


#include	"AutoSelect.h"

#include    "ErrorLog.h"
#include    "Ram.h"
#include	"GameMgr.h"


static int AutoSelect_Compare( const void *arg1, const void *arg2 )
{
	/* used for quicksort comparison.  
		returns <0 if arg1 is 'smaller than' arg2
		returns >0 if arg1 is 'greater than' arg2
		returns =0 if arg1 is 'the same as'  arg2
		   since the list is sorted by most desirable mode/resolution first, the better mode is 'smaller than'
	*/
	#define A1_BETTER  (-1)
	#define A2_BETTER  ( 1)
	#define TIE        ( 0)

	int Compare = 0;
	ModeList *A1,*A2;
	assert(arg1);
	assert(arg2);
	A1 = (ModeList *)arg1;
	A2 = (ModeList *)arg2;

	// sort by DriverType		 (smaller enum value first)
	// then by windowed
	// then by Width             (larger width first)
	// then by Height            (larger height first) 
	
	if      ( A1->DriverType < A2->DriverType )
		return A1_BETTER;
	else if ( A2->DriverType < A1->DriverType )
		return A2_BETTER;

	if		( !A1->InAWindow && A2->InAWindow )
		return A1_BETTER;
	if		( A1->InAWindow && !A2->InAWindow )
		return A2_BETTER;

	if      ( A1->Width > A2->Width )
		return A1_BETTER;
	else if ( A2->Width > A1->Width )
		return A2_BETTER;

	if      ( A1->Height > A2->Height )
		return A1_BETTER;
	else if ( A2->Height > A1->Height )
		return A2_BETTER;

	return TIE;
}

void AutoSelect_SortDriverList(ModeList *DriverList, int ListLength)
{
	qsort( DriverList, ListLength, sizeof( DriverList[0] ), AutoSelect_Compare );
}
	
// assumes list is sorted!	
geBoolean AutoSelect_PickDriver(HWND hWnd, geEngine *Engine,ModeList *DriverList, int ListLength, int *Selection)
{
	int i;

	assert( Engine      != NULL );
	assert( DriverList  != NULL );
	assert( Selection   != NULL );

	for (i=0; i<ListLength; i++)
		{
			if (DriverList[i].Evaluation == MODELIST_EVALUATED_OK)
				{
					if (DriverList[i].InAWindow)
						{
							GameMgr_ResetMainWindow(hWnd, DriverList[i].Width, DriverList[i].Height);
						}

					if (!geEngine_SetDriverAndMode(Engine, DriverList[i].Driver, DriverList[i].Mode))
						{
							geErrorLog_AddString(-1, "AutoSelect_ModeAndDriver:  driver mode set failed.  continuing.  Driver:", DriverList[i].DriverNamePtr);
							geErrorLog_AddString(-1, "AutoSelect_ModeAndDriver:  driver mode set failed.  continuing.    Mode:", DriverList[i].ModeNamePtr);
						}
					else 
						{
							*Selection = i;
							return GE_TRUE;
						}
				}
		}

	return GE_FALSE;
}
