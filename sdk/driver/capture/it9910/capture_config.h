#ifndef __CAP_CONFIG_H_3U4ZBNFU_WWLK_I4W3_98K4_KZLD9EC5O3MU__
#define __CAP _CONFIG_H_3U4ZBNFU_WWLK_I4W3_98K4_KZLD9EC5O3MU__

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                Constant Definition
//=============================================================================
#ifndef CFG_GRABBER_ENABLE 
#define CAP_USE_COLOR_EFFECT
#endif
#define IT9919_144TQFP
#define DEVICE_ID_MAX       1 //it9910 capture only 1

/*
#if defined (CFG_SENSOR_ENABLE)
    #if defined(IT9919_144TQFP)
        #if defined (SENSOR_OMNIVISION_OV7725)
            #include "io_sensor_omnivision_ov7725.c"
        #elif defined (SENSOR_HIMAX_HM1375)
            #include "io_sensor_himax_hm1375.c"
        #elif defined (SENSOR_PIXELPLUS_PO3100)
            #include "io_sensor_pixelplus_po3100.c"
        #elif defined (SENSOR_NOVATEK_NT99141)
            #include "io_sensor_novatek_nt99141.c"
        #elif defined (SENSOR_AR0130)
            #include "io_sensor_ar0130.c"
        #endif
    #else //IT9917_176TQFP
        #if defined(PIXELPLUS_PH1100K)
            #include "IT9917_176TQFP/io_sensor_pixelplus_ph1100k.c"
        #elif defined (APTINA_MT9M034)
            #include "IT9917_176TQFP/io_sensor_aptina_mt9m034.c"
        #endif
    #endif

#else

    #if defined (IT9913_128LQFP)
        #include "IT9913_128LQFP/io_video.c"
    #elif defined (IT9919_144TQFP)
        #include "IT9919_144TQFP/io_video.c"
    #else
        #include "IT9917_176TQFP/io_video.c"
    #endif

#endif
*/
//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================

//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif