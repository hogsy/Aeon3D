////////////////////////////////////////////////////////////////////////////////////////
//	ModelCtl.h
//                                                                                      
//  The contents of this file are subject to the Genesis3D Public License               
//  Version 1.01 (the "License"); you may not use this file except in                   
//  compliance with the License. You may obtain a copy of the License at                
//  http://www.genesis3d.com                                                            
//                                                                                      
//  Software distributed under the License is distributed on an "AS IS"                 
//  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See                
//  the License for the specific language governing rights and limitations              
//  under the License.                                                                  
//                                                                                      
//  Copyright (C) 1996-1999 Eclipse Entertainment, L.L.C. All Rights Reserved           
//                                                                                      
////////////////////////////////////////////////////////////////////////////////////////
#ifndef	MODELCTL_H
#define	MODELCTL_H

#include	"genesis.h"

#pragma warning( disable : 4068 )

#pragma GE_Type("modelctl.bmp")
typedef struct	ModelController
{
#pragma	GE_Private
	geFloat			LastTime;
	
#pragma GE_Published
    geVec3d			origin;
	geWorld_Model *	Model;
	char *			TriggerName;

#pragma GE_Origin(origin)
#pragma GE_DefaultValue(TriggerName, "NULL")
}	ModelController;

//void	ModelCtl_GetCurrentPosition(ModelController *Controller, geVec3d *Pos);

#pragma warning( default : 4068 )

geBoolean	ModelCtl_Init(void);
geBoolean	ModelCtl_Reset(geWorld *World);
geBoolean	ModelCtl_Frame(geWorld *World, const geXForm3d *ViewPoint, geFloat Time);
geBoolean	ModelCtl_Shutdown(void);

#endif

