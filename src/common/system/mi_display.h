#ifndef MI_DISP_H__
#define MI_DISP_H__

#include "mi_disp.h"
#include "mi_sys.h"

void display_spawnMIDISP(MI_U16 width, MI_U16 height, MI_U32 luma, MI_U32 contrast, MI_U32 hue, MI_U32 saturation)
{
    /* 
        if we don't have a valid mi_disp0 endpoint the screen will look washed out. 
        we also can't push changes into it (luma/hue/sat/contrast, sharpness, etc)
        
        the below allows us to spawn an ephemeral mi_disp0 that will set the screen up correctly, but unfortunately when whatever process that called it ends, so will mi_disp0
        
        it's the circle of life.
        
        some maths ops happen, these were pulled from the l binary in ghidra
        
        this function will ONLY run it a mi_disp0 doesn't already exist. If it does you neeed to push CSC changes in via shell.
    */

    FILE *file = fopen("/proc/mi_modules/mi_disp/mi_disp0", "r");
    if (file == NULL) {
        MI_DISP_DEV DispDev = 0;
        MI_DISP_LAYER DispLayer = 0;
        MI_DISP_INPUTPORT DispInport = 0;
        MI_DISP_PubAttr_t stPubAttr;
        MI_DISP_VideoLayerAttr_t stLayerAttr;
        MI_DISP_InputPortAttr_t stInputPortAttr;
        MI_DISP_LcdParam_t lcdParam;

        MI_U32 lumaProcessed = luma + 17 * 2;
        MI_U32 satProcessed = saturation * 5;
        MI_U32 contProcessed = contrast + 40;
        MI_U32 hueProcessed = hue * 5;

        MI_SYS_Init();
        MI_DISP_Enable(DispDev);

        memset(&stPubAttr, 0, sizeof(MI_DISP_PubAttr_t));
        memset(&stLayerAttr, 0, sizeof(MI_DISP_VideoLayerAttr_t));
        memset(&stInputPortAttr, 0, sizeof(MI_DISP_InputPortAttr_t));

        stPubAttr.eIntfSync = E_MI_DISP_OUTPUT_USER;
        stPubAttr.eIntfType = E_MI_DISP_INTF_LCD;
        stPubAttr.u32BgColor = 0x800080;
        MI_DISP_SetPubAttr(DispDev, &stPubAttr);

        lcdParam.stCsc.eCscMatrix = E_MI_DISP_CSC_MATRIX_BT601_TO_RGB_PC;
        lcdParam.stCsc.u32Luma = lumaProcessed;
        lcdParam.stCsc.u32Contrast = contProcessed;
        lcdParam.stCsc.u32Hue = hueProcessed;
        lcdParam.stCsc.u32Saturation = satProcessed;
        MI_DISP_SetLcdParam(DispDev, &lcdParam);

        stLayerAttr.stVidLayerSize.u16Width = width;
        stLayerAttr.stVidLayerSize.u16Height = height;
        stLayerAttr.stVidLayerDispWin.u16X = 0;
        stLayerAttr.stVidLayerDispWin.u16Y = 0;
        stLayerAttr.stVidLayerDispWin.u16Width = width;
        stLayerAttr.stVidLayerDispWin.u16Height = height;
        MI_DISP_SetVideoLayerAttr(DispLayer, &stLayerAttr);
        MI_DISP_EnableVideoLayer(DispLayer);

        stInputPortAttr.u16SrcWidth = width;
        stInputPortAttr.u16SrcHeight = height;
        stInputPortAttr.stDispWin.u16X = 0;
        stInputPortAttr.stDispWin.u16Y = 0;
        stInputPortAttr.stDispWin.u16Width = width;
        stInputPortAttr.stDispWin.u16Height = height;
        MI_DISP_SetInputPortAttr(DispLayer, DispInport, &stInputPortAttr);
        MI_DISP_EnableInputPort(DispLayer, DispInport);
    }
    else {
        fclose(file);
    }
}

#endif // MI_DISP_H__