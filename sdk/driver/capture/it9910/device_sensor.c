#include "sensor/mmp_sensor.h"
#include "capture/sensor_device_table.h"
#include "capture/mmp_capture.h"

//=============================================================================
//                Constant Definition
//=============================================================================

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================

//=============================================================================
//                Global Data Definition
//=============================================================================

static MMP_BOOL         gtDeviceReboot = MMP_TRUE;

extern MMP_UINT32       CapMemBuffer[CAPTURE_MEM_BUF_COUNT];

static MMP_CAP_DEVICE_ID inputDeviceID;

extern pthread_mutex_t gCapmem_mutex;

extern MMP_CAP_SHARE    DevInfo;

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================


//=============================================================================
/**
 * Cap GetCaptureDevice ID 
 */
//=============================================================================

MMP_CAP_DEVICE_ID ithCapGetCaptureDevice(void)
{
    return inputDeviceID;
}

//=============================================================================
/**
 * Cap Auto Detect Device.
 */
//=============================================================================
MMP_CAP_DEVICE_ID
ithCapAutoDetectDevice(
    void)
{
    return MMP_CAP_DEV_SENSOR;
}

//=============================================================================
/**
 * Cap Device Tri-State.
 */
//=============================================================================
void
ithCapDeviceAllDeviceTriState(
    void)
{
#if defined (IT9919_144TQFP)
    #if defined (REF_BOARD_CAMERA)
    //ithCap_Set_UnFire(DEVICE_ID);
    return;
    #endif
#endif

}

//=============================================================================
/**
 * Cap Device initialization.
 */
//=============================================================================
MMP_RESULT
ithCapDeviceInitialize(
    MMP_CAP_DEVICE_ID Device_ID)
{
    MMP_RESULT result = MMP_SUCCESS;

    inputDeviceID = Device_ID;

    ithSensorRegsisterCallback();
    
    ithSensorInitialize();

    ithCapDeviceAllDeviceTriState();

    memset(&DevInfo, 0, sizeof(MMP_CAP_SHARE));

    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "%s error %d", __FUNCTION__, __LINE__);

    return (MMP_RESULT)result;
}

//=============================================================================
/**
 * Cap Device terminate.
 */
//=============================================================================
void
ithCapDeviceTerminate(
    void)
{
    MMP_RESULT  result = MMP_SUCCESS;
    MMP_UINT16  status;

    ithCapDeviceAllDeviceTriState();

    memset(&DevInfo, 0, sizeof(MMP_CAP_SHARE));
}

MMP_BOOL
ithCapGetDeviceIsSignalStable(
    void)
{
    return MMP_TRUE;
}

//=============================================================================
/**
 * ithCapGetDeviceInfo
 */
//=============================================================================
void
ithCapGetDeviceInfo(
    CAPTURE_HANDLE *cap_handle, MMP_CAP_SHARE *data)
{
    CAP_CONTEXT *Capctxt = &cap_handle->cap_info;
    //CAP share with ISP Set info
    data->IsInterlaced          = MMP_FALSE;
    data->Width                 = (MMP_UINT16)SENSOR_TABLE.HActive;
    data->Height                = (MMP_UINT16)SENSOR_TABLE.VActive;
    data->HSyncPol              = SENSOR_TABLE.HPolarity;
    data->VSyncPol              = SENSOR_TABLE.VPolarity;
    data->FrameRate             = SENSOR_TABLE.FrameRate;
    data->OutWidth              = SENSOR_TABLE.ROIWidth;
    data->OutHeight             = SENSOR_TABLE.ROIHeight;
    
    pthread_mutex_lock(&gCapmem_mutex);
    data->OutAddrY[0]           = CapMemBuffer[CAP_MEM_Y0];
    data->OutAddrUV[0]          = CapMemBuffer[CAP_MEM_UV0];
    data->OutAddrY[1]           = CapMemBuffer[CAP_MEM_Y1];
    data->OutAddrUV[1]          = CapMemBuffer[CAP_MEM_UV1];
    data->OutAddrY[2]           = CapMemBuffer[CAP_MEM_Y2];
    data->OutAddrUV[2]          = CapMemBuffer[CAP_MEM_UV2];
    pthread_mutex_unlock(&gCapmem_mutex);

    data->bMatchResolution        = MMP_TRUE;

    //Cap info  Set info
    Capctxt->Skippattern        = SENSOR_TABLE.Skippattern;
    Capctxt->funen.EnCSFun      = MMP_FALSE;
    Capctxt->ininfo.HNum1       = 0;
    Capctxt->ininfo.HNum2       = 0;
    Capctxt->ininfo.LineNum1    = 0;
    Capctxt->ininfo.LineNum2    = 0;
    Capctxt->ininfo.LineNum3    = 0;
    Capctxt->ininfo.LineNum4    = 0;
    //Cap info Set ROI
    Capctxt->ininfo.ROIPosX     = SENSOR_TABLE.ROIPosX;
    Capctxt->ininfo.ROIPosY     = SENSOR_TABLE.ROIPosY;
    Capctxt->ininfo.ROIWidth    = SENSOR_TABLE.ROIWidth;
    Capctxt->ininfo.ROIHeight   = SENSOR_TABLE.ROIHeight;

    memcpy(&DevInfo, data, sizeof(MMP_CAP_SHARE));
}

//=============================================================================
/**
 * Device reboot.
 */
//=============================================================================
MMP_RESULT
ithCapSetDeviceReboot(
    MMP_BOOL flag)
{
    gtDeviceReboot = flag;

    if (!gtDeviceReboot)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "------Not Reboot Device-------\n");
}

MMP_BOOL
ithCapGetDeviceReboot(
    void)
{
    return gtDeviceReboot;
}

//=============================================================================
/**
 * Capture Resolution.
 */
//=============================================================================
MMP_UINT16
ithCapGetResolutionIndex(
    MMP_CAP_DEVICE_ID id)
{
    return 0;
}

//=============================================================================
/**
 * Capture VESA Timing Check.
 */
//=============================================================================
MMP_BOOL
ithCapVESATimingCheck(
    MMP_UINT16          width,
    MMP_UINT16          height,
    MMP_CAP_INPUT_INFO  *info)
{
    (*info) = MMP_CAP_INPUT_INFO_CAMERA;
    return MMP_TRUE;
}
