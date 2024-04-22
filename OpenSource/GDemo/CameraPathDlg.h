/****************************************************************************************/
/*  CameraPath.h                                                                        */
/*                                                                                      */
/*  Author: Frank Maddin                                                                */
/*  Description:    Maintains a dialog and motions for a camera			                */
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

#ifndef __CAMERAPATHDLG_H__
#define __CAMERAPATHDLG_H__

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_CAMERA_CUTS 32

typedef struct CameraPath CameraPath;

typedef enum CameraCBtype
{
	CAMERA_GET_DATA,
	CAMERA_PUT_DATA,
	CAMERA_PUT_CUT_INDEX,
	CAMERA_PUT_CUT_TIME,
	CAMERA_ERROR,
};

typedef geBoolean (*CP_CB)(int Type, float Time, geXForm3d *XF1, geXForm3d *XF2);

CameraPath	*CameraPath_Create(CP_CB, geMotion**);
void		CameraPath_Destroy(CameraPath **);

geBoolean CameraPathDlg_Show(HANDLE hInstance, HWND hwndParent, CameraPath *CP, const char *SceneName);

#ifdef __cplusplus
}
#endif


#endif