/*****************************************************************************
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information contained
 *  herein is confidential. The software may not be copied and the information
 *  contained herein may not be used or disclosed except with the written
 *  permission of MediaTek Inc. (C) 2005
 *
 *  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
 *  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 *  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 *  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 *  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
 *  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
 *  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
 *  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 *  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 *  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
 *  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
 *  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
 *  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
 *  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
 *  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
 *
 *****************************************************************************/

/*****************************************************************************
 *
 * Filename:
 * ---------
 *   sp2529yuv_Sensor.h
 *
 * Project:
 * --------
 *   MAUI
 *
 * Description:
 * ------------
 *   Image sensor driver declare and macro define in the header file.
 *
 * Author:
 * -------
 *   Mormo
 *
 *=============================================================
 *             HISTORY
 * Below this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Log$
 * 2011/11/03 Firsty Released By Mormo;
 *
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *=============================================================
 ******************************************************************************/
 
#ifndef __SP2529YUV_SENSOR_H
#define __SP2529YUV_SENSOR_H


#define UXGA_PERIOD_PIXEL_NUMS						(1600)
#define UXGA_PERIOD_LINE_NUMS						(1200)

#define SVGA_PERIOD_PIXEL_NUMS						(800)
#define SVGA_PERIOD_LINE_NUMS						(600)

#define IMAGE_SENSOR_SVGA_GRAB_PIXELS			0
#define IMAGE_SENSOR_SVGA_GRAB_LINES			1

#define IMAGE_SENSOR_UXGA_GRAB_PIXELS			0
#define IMAGE_SENSOR_UXGA_GRAB_LINES			1

#define IMAGE_SENSOR_SVGA_WIDTH					(800)
#define IMAGE_SENSOR_SVGA_HEIGHT					(600)

#define IMAGE_SENSOR_UXGA_WIDTH					(1600-16)
#define IMAGE_SENSOR_UXGA_HEIGHT					(1200-12)

#define IMAGE_SENSOR_PV_WIDTH					(IMAGE_SENSOR_SVGA_WIDTH - 8)
#define IMAGE_SENSOR_PV_HEIGHT					(IMAGE_SENSOR_SVGA_HEIGHT - 6)

#define IMAGE_SENSOR_FULL_WIDTH					(IMAGE_SENSOR_UXGA_WIDTH - 8)
#define IMAGE_SENSOR_FULL_HEIGHT					(IMAGE_SENSOR_UXGA_HEIGHT - 6)

#define SP2529_WRITE_ID							0x60
#define SP2529_READ_ID							0x61

typedef enum
{
	Gamma_1_8 = 1,
	Gamma_2_0,
	Gamma_2_2,
	Gamma_dm_3,
	Gamma_dm_5,
	Gamma_dm_6,
	Gamma_dm_7,
	Gamma_dm_8,
	Gamma_dm_9
}SP2529_GAMMA_TAG;

UINT32 SP2529Open(void);
UINT32 SP2529Control(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow, MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
UINT32 SP2529FeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId, UINT8 *pFeaturePara,UINT32 *pFeatureParaLen);
UINT32 SP2529GetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_INFO_STRUCT *pSensorInfo, MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
UINT32 SP2529GetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution);
UINT32 SP2529Close(void);

#endif /* __SENSOR_H */

