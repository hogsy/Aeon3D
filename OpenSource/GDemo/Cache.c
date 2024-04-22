/****************************************************************************************/
/*  Cache.c                                                                             */
/*                                                                                      */
/*  Author: Frank Maddin                                                                */
/*  Description:    Caches bitmaps and sounds							                */
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

#include <windows.h>
#include <assert.h>
#include "genesis.h"
#include "cache.h"

void Cache_Bmp(TexturePool *TPool)
{
	assert(TPool);

	//SPool_Add(SPool, NULL, FileName );

	// for easing trigger system - pre load effects
	TPool_Add( TPool, NULL, "bmp\\effect\\squiddy01.bmp", NULL );
	TPool_Add( TPool, NULL, "bmp\\effect\\squiddy02.bmp", NULL );
	TPool_Add( TPool, NULL, "bmp\\effect\\skinfx.bmp", "bmp\\effect\\a_skinfx.bmp" );
	//TPool_Add( TPool, NULL, "bmp\\effect\\skinfx2.bmp", "bmp\\effect\\a_skinfx2.bmp" );
	TPool_Add( TPool, NULL, "bmp\\effect\\lvsmoke.bmp", "bmp\\effect\\a_lvsmoke.bmp" );
	TPool_Add( TPool, NULL, "bmp\\effect\\flame02.bmp", "bmp\\effect\\a_flame.bmp" );
	TPool_Add( TPool, NULL, "bmp\\effect\\flame03.bmp", "bmp\\effect\\a_flame.bmp" );

	TPool_Add( TPool, NULL, "bmp\\effect\\trans.bmp", "bmp\\effect\\a_trans.bmp" );
	TPool_Add( TPool, NULL, "bmp\\effect\\bubl.bmp", "bmp\\effect\\a_bubl.bmp" );
	TPool_Add( TPool, NULL, "bmp\\effect\\gem.bmp", "bmp\\effect\\a_gem.bmp" );
	TPool_Add( TPool, NULL, "bmp\\effect\\squiddy.bmp", NULL );
	TPool_Add( TPool, NULL, "bmp\\effect\\seafloor.bmp", NULL );
}

void Cache_BmpDir(char *Dir, TexturePool *TPool)
	{
	geVFile * FileBase;
	geVFile_Finder * Finder;
	geVFile_Finder * AFinder;
	int i;
	char BasePath[1024];
	char FName[1024];
	char AName[1024];
	char Alpha1[256];
	char Alpha2[256];
	char *found,*ptr;
	char *Ext[] = {	"\\bmp",
					"\\bmp\\effect"
					,NULL	};

	for (i = 0; Ext[i]; i++)
		{
		strcpy(BasePath, Dir);
		strcat(BasePath,Ext[i]);

		FileBase = geVFile_OpenNewSystem(NULL,GE_VFILE_TYPE_DOS,BasePath,NULL,GE_VFILE_OPEN_READONLY|GE_VFILE_OPEN_DIRECTORY);
		if ( ! FileBase )
			return;

		Finder = geVFile_CreateFinder(FileBase,"*.bmp");
		if ( ! Finder )
			{
			geVFile_Close(FileBase);
			break;
			}

		while( geVFile_FinderGetNextFile(Finder) )
			{
			geVFile_Properties Properties;
			geVFile_FinderGetProperties(Finder,&Properties);

			strlwr(Properties.Name);
			if (strnicmp(Properties.Name, "a_", 2) == 0 || strstr(Properties.Name, "_a.bmp"))
				continue;

			sprintf(Alpha1, "a_%s", Properties.Name);

			strcpy(Alpha2, Properties.Name);
			ptr = strstr(Alpha2,".bmp");
			assert(ptr);
			ptr[0] = '\0'; // take the .bmp off the end
			strcat(Alpha2,"_a.bmp"); // add the _a.bmp to the end

			AFinder = geVFile_CreateFinder(FileBase,"*.bmp");
			if ( ! Finder )
				{
				geVFile_Close(FileBase);
				break;
				}

			// search for alpha in the same directory
			found = NULL;
			while( geVFile_FinderGetNextFile(AFinder) )
				{
				geVFile_Properties AProperties;
				geVFile_FinderGetProperties(AFinder,&AProperties);

				if (stricmp(Alpha1, AProperties.Name) == 0)
					{
					found = Alpha1;
					break;
					}
				else
				if (stricmp(Alpha2, AProperties.Name) == 0)
					{
					found = Alpha2;
					break;
					}
				}

			strcpy(FName,&Ext[i][1]);
			strcat(FName,"\\");
			strcat(FName,Properties.Name);

			if (found)
				{
				strcpy(AName,&Ext[i][1]);
				strcat(AName,"\\");
				strcat(AName,found);

				TPool_Add( TPool, NULL, FName, AName );
				}
			else
				{
				TPool_Add( TPool, NULL, FName, NULL );
				}
	
			geVFile_DestroyFinder(AFinder);
			}

		geVFile_DestroyFinder(Finder);
		geVFile_Close(FileBase);
		}
	}

void Cache_Wav(char *Dir, SoundPool *SPool)
	{
	geVFile * FileBase;
	geVFile_Finder * Finder;
	char BasePath[1024];
	char FName[1024];

	strcpy(BasePath, Dir);
	strcat(BasePath,"\\wav");

	FileBase = geVFile_OpenNewSystem(NULL,GE_VFILE_TYPE_DOS,BasePath,NULL,GE_VFILE_OPEN_READONLY|GE_VFILE_OPEN_DIRECTORY);
	if ( ! FileBase )
		return;

	Finder = geVFile_CreateFinder(FileBase,"*.wav");
	if ( ! Finder )
		{
		geVFile_Close(FileBase);
		return;
		}

	while( geVFile_FinderGetNextFile(Finder) )
		{
		geVFile_Properties Properties;
		geVFile_FinderGetProperties(Finder,&Properties);

		strlwr(Properties.Name);

		strcpy(FName,"wav\\");
		strcat(FName,Properties.Name);

		SPool_Add(SPool, NULL, FName );
		}

	geVFile_DestroyFinder(Finder);
	geVFile_Close(FileBase);
	}


