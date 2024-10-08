/****************************************************************************************/
/*  ProcEng.c                                                                           */
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
#include <Assert.h>
#include <string.h>
#include "Genesis.h"
#include "Ram.h"
#include "Errorlog.h"

#include "ProcEng.h"
#include "Procedural.h"


// at the end of this file :
typedef Procedural_Table * (*GetProceduralFunc)(void);

extern GetProceduralFunc GetProceduralFunctions[];
extern int NumGetProceduralFunctions;

#define	PROCENG_MAX_PTABLES		32			// Pre-Loaded tables that procs attach to
#define	PROCENG_MAX_PROCS		256			// Actual procs (created from table data)

//====================================================================================
//====================================================================================
typedef struct ProcEng_PTable
{
	Procedural_Table	*Table;
	int					Uses;
} ProcEng_PTable;

typedef struct ProcEng_Proc
{
	Procedural_Table	*Table;				// Table used to create this proc (So we can destroy it)
	Procedural			*Proc;
	geBitmap			*Bitmap;
} ProcEng_Proc;

typedef struct ProcEng
{
	geWorld				*World;

	int32				NumPTables;
	ProcEng_PTable		PTables[PROCENG_MAX_PTABLES];

	int32				NumProcs;
	ProcEng_Proc		Procs[PROCENG_MAX_PROCS];
} ProcEng;



//====================================================================================
//	ProcEng_Create
//====================================================================================
ProcEng *ProcEng_Create(geVFile *CfgFile, geWorld *World)
{
	ProcEng		*PEng;
	int32		i;

	// For now, hard code in the procedures (but later, load *.dll)...
	PEng = geRam_Allocate(sizeof(*PEng));

	if (!PEng)
		return GE_FALSE;

	memset(PEng, 0, sizeof(*PEng));

	// init all compiled-in procedurals:

	for(i=0;i<NumGetProceduralFunctions;i++)
	{	
		if ( GetProceduralFunctions[i] )
		{
		Procedural_Table * pTable;
			pTable = (*GetProceduralFunctions[i]) ();
			if ( pTable->Procedurals_Version == Cur_Procedurals_Version )
			{
				PEng->PTables[PEng->NumPTables].Table = pTable;
				PEng->PTables[PEng->NumPTables].Uses  = 0;
				PEng->NumPTables ++;
			}
			else
			{
				//char ErrStr[1024];

				//sprintf(ErrStr,"ProcEng_Create : found procedural : %s : but ignored because of version mismatch",pTable->Name);
				//geErrorLog_AddString(-1,ErrStr, NULL);
			}
		}
	}

	PEng->World = World;

	return PEng;												 
	CfgFile;
}

//====================================================================================
//	ProcEng_Destroy
//====================================================================================
void ProcEng_Destroy(ProcEng **pPEng)
{
	int32			i;
	ProcEng_Proc	*pProc;
	ProcEng_PTable	*pTable;
	ProcEng			*PEng;

	assert(pPEng);

	PEng = *pPEng;
	if ( ! PEng )
		return;
	
	// Free all the allocated procs
	pProc = PEng->Procs;
	for (i=0; i< PEng->NumProcs; i++, pProc++)
	{
		assert(pProc->Table);
		assert(pProc->Proc);

		pProc->Table->Destroy(pProc->Proc);

		geBitmap_Destroy(&(pProc->Bitmap)); // we did creatref

		memset(pProc, 0, sizeof(*pProc));	
	}
	PEng->NumProcs = 0;

	// Free all the table dlls...
	pTable = PEng->PTables;
	for (i=0; i< PEng->NumPTables; i++, pTable++)
	{
		memset(pTable, 0, sizeof(*pTable));
	}
	PEng->NumPTables = 0;

	//if (PEng)
	//	geWorld_Free(PEng->World);
	PEng->World = NULL;

	geRam_Free(PEng);

	*pPEng = NULL;
}

//====================================================================================
//	ProcEng_Destroy
//====================================================================================
geBoolean ProcEng_Minimize(ProcEng *PEng)
{
ProcEng_PTable	*pTable;
ProcEng_PTable	*pTableLast;

	assert(PEng);

	// Free all the table dlls...
	pTable = PEng->PTables;
	pTableLast = pTable + (PEng->NumPTables - 1);
	while( pTable <= pTableLast )
	{
		if ( pTable->Uses < 1 )
		{
			*pTable = *pTableLast;
			pTableLast--;
			PEng->NumPTables --;
		}
		else
		{
			pTable ++;
		}
	}
	return GE_TRUE;
}

//====================================================================================
//	ProcEng_AddProcedural
//	NOTE - The only way to remove procs, is to destroy the ProcEng object, and start over for now
//====================================================================================
geBoolean ProcEng_AddProcedural(ProcEng *PEng, const char *ProcName, geBitmap **pBitmap, const char * Params)
{
	int32			i;
	ProcEng_PTable	*pTable;
	int				Length;

	assert(PEng);
	assert(ProcName);
	assert(pBitmap);

	if (PEng->NumProcs+1 >= PROCENG_MAX_PROCS)
	{
		geErrorLog_AddString(-1, "ProcEng_AddProcedural:  Max procs.", NULL);
		return GE_FALSE;
	}

	// Find Procedural_Table by Name
	pTable = PEng->PTables;
	Length = strlen( ProcName );
	for (i=0; i< PEng->NumPTables; i++, pTable++)
	{
		// Should we ignore case? ! YES!
		if ( strnicmp(pTable->Table->Name, ProcName, Length) == 0)
			break;
	}

	if (i == PEng->NumPTables)		// Didn't find it
	{
		geErrorLog_AddString(-1, "ProcEng_AddProcedural:  Table not found for procedural.", NULL);
		return GE_FALSE;
	}

	assert(pTable->Table);

	// Create the proc (each bitmap should have it's own)
	PEng->Procs[PEng->NumProcs].Proc = pTable->Table->Create(pBitmap, Params);

	if (!PEng->Procs[PEng->NumProcs].Proc)
	{
	/*char ErrStr[1024];
		sprintf(ErrStr,"ProcEng_AddProcedural:  (%s)->Create failed",pTable->Table->Name);
		geErrorLog_AddString(-1,ErrStr, NULL);*/
		return GE_FALSE;
	}
	if (! *pBitmap)
	{
	/*char ErrStr[1024];
		sprintf(ErrStr,"ProcEng_AddProcedural: (%s) : no Bitmap !",pTable->Table->Name);
		geErrorLog_AddString(-1,ErrStr, NULL);*/
		return GE_FALSE;
	}

	PEng->Procs[PEng->NumProcs].Bitmap = *pBitmap;
	PEng->Procs[PEng->NumProcs].Table = pTable->Table;

	geBitmap_CreateRef(*pBitmap);
	// make sure the bitmap isn't destroyed before our procedural

	PEng->NumProcs++;

	pTable->Uses ++;

	return GE_TRUE;
}

//====================================================================================
//	ProcEng_Animate
//====================================================================================
geBoolean ProcEng_Animate(ProcEng *PEng, float ElapsedTime)
{
	int32			i;
	ProcEng_Proc	*pProc;

	pProc = PEng->Procs;
	for (i=0; i< PEng->NumProcs; i++, pProc++)
	{
		assert(pProc->Table);
		assert(pProc->Proc);

		if (!geWorld_BitmapIsVisible(PEng->World, pProc->Bitmap))
			continue;

		if (!pProc->Table->Animate(pProc->Proc, ElapsedTime))
		{
		/*char ErrStr[1024];
			sprintf(ErrStr,"ProcEng_Animate: (%s)->Animate failed !",pProc->Table->Name);
			geErrorLog_AddString(-1,ErrStr, NULL);*/
			return GE_FALSE;
		}
	}
	return GE_TRUE;
}
