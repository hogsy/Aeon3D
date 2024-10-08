////////////////////////////////////////////////////////////////////////////////////////
//	USERTYPES.h
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

#pragma warning( disable : 4068 )
#ifdef __cplusplus
extern "C" {
#endif

#pragma GE_BrushContents
typedef enum
{
	Water = 0x00010000,
	Lava = 0x00020000,
	Shape = 0x00040000,
	Poison = 0x00080000,
	HardLava = 0x00100000,
} UserContentsEnum;

// Camera Loc
#pragma GE_Type("CameraStart.ico")
typedef struct	CameraStart
{
#pragma GE_Published
    geVec3d		origin;
	char *		Type;
	char *		Name;
	char *		Motion;
	char *		MotionBone;
	float		MotionScale;
	float		FieldOfView;
	char *		Trigger;
	geVec3d		angles;
#pragma GE_Angles(angles)
#pragma GE_Origin(origin)
#pragma GE_DefaultValue(Type, "") 
#pragma GE_DefaultValue(Name, "") 
#pragma GE_DefaultValue(Motion, "") 
#pragma GE_DefaultValue(MotionBone, "") 
#pragma GE_DefaultValue(Trigger, "") 
#pragma GE_DefaultValue(MotionScale, "1.0") 
#pragma GE_DefaultValue(FieldOfView, "2.0") 
}	CameraStart;

#pragma GE_Type("Shape.ico")
typedef struct	Shape
{
#pragma GE_Published
	char *		Name;
	int	      Type;
	char *	  Trigger;
	int		  CurMotion;
	geWorld_Model *	Model;
	int			Looping;
	geWorld_Model *	TargetModel;
    geVec3d		origin;
	int			Visible;
	int			OpenMotion;
	int			CloseMotion;
#pragma GE_Origin(origin)
#pragma GE_DefaultValue(CurMotion, "-1") 
#pragma GE_DefaultValue(Visible, "1") 
#pragma GE_DefaultValue(OpenMotion, "-2") 
#pragma GE_DefaultValue(CloseMotion, "-2") 
}	ShapeEntity;

#pragma GE_Type("Dlight.ico")
typedef struct	DlightEntity
{
	geLight		*MyLight;
#pragma GE_Published
    geVec3d		origin;
	GE_RGBA rgba;
	float		radius;
#pragma GE_Origin(origin)
}	DlightEntity;

#pragma GE_Type("DoorEntity.ico")
typedef struct	DoorEntity
{
#pragma GE_Published
	char *		Name;
	int	      Type;
	char *	  Trigger;
	int		  CurMotion;
	geWorld_Model *	Model;
	int			Looping;
	geWorld_Model *	TargetModel;
    geVec3d		origin;
	int			Visible;
	int			OpenMotion;
	int			CloseMotion;
#pragma GE_DefaultValue(Type, "7") 
#pragma GE_DefaultValue(Looping, "0") 
#pragma GE_DefaultValue(CurMotion, "-1") 
#pragma GE_DefaultValue(Visible, "1") 
#pragma GE_Origin(origin)
#pragma GE_DefaultValue(OpenMotion, "0") 
#pragma GE_DefaultValue(CloseMotion, "1") 
}	DoorEntity;

#pragma GE_Type("ActivateDoorEntity.ico")
typedef struct	ActivateDoorEntity
{
#pragma GE_Published
	char *		Name;
	int	      Type;
	char *	  Trigger;
	int		  CurMotion;
	geWorld_Model *	Model;
	int			Looping;
	geWorld_Model *	TargetModel;
    geVec3d		origin;
	int			Visible;
	int			OpenMotion;
	int			CloseMotion;
#pragma GE_DefaultValue(Type, "8") 
#pragma GE_DefaultValue(Looping, "0") 
#pragma GE_DefaultValue(CurMotion, "-1") 
#pragma GE_DefaultValue(Visible, "1") 
#pragma GE_Origin(origin)
#pragma GE_DefaultValue(OpenMotion, "0") 
#pragma GE_DefaultValue(CloseMotion, "1") 
}	ActivateDoorEntity;

#pragma GE_Type("WayPoint.ico")
typedef struct	WayPoint WayPoint;
#pragma GE_Type("WayPoint.ico")
typedef struct	WayPoint
{
	float		Dist1;
	float		Dist2;
	float		Dist3;
	float		Dist4;
	int			Checked;
#pragma GE_Published
	char *		Name;
	WayPoint *	Link1;
 	WayPoint *	Link2;
	WayPoint *	Link3;
	WayPoint *	Link4;
   geVec3d		origin;
	geBoolean	Precise;
#pragma GE_Origin(origin)
#pragma GE_DefaultValue(Precise, "False") 
}	WayPoint;

// FogLight
#pragma GE_Type("FogLight.ico")
typedef struct FogLight
{

#pragma GE_Published
	geVec3d		Origin;
	GE_RGBA		Color;
	float		Brightness;
	float		Radius;
#pragma GE_Origin(Origin)
} FogLight;

#ifdef __cplusplus
}
#endif

#pragma warning( default : 4068 )

