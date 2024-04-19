#ifndef GLOBAL_VALUE_H_
#define GLOBAL_VALUE_H_

#include <tl_common.h>

#ifndef MAIN_MAIN
#define EXTERN_MAIN_PARA extern
#else
#define EXTERN_MAIN_PARA
#endif



//----------------------------//
//          Function        //
//----------------------------//
#define SNK_PPS

#define USE_QC_SNK_TASK  1

u8 g_I2C_addr;
u8 g_PD_Ver;
u8 g_sop_flag;
u8 g_sop_p_flag;
u8 g_sop_p_p_flag;
u8 g_hrd_rst_flag;
u8 g_cable_rst_flag;
u8 g_dbg_sop_p_flag;
u8 g_dbg_sop_p_p_flag;




#define OBJ_MAX 32
#define FP6606_DEVIC_ADDR   g_I2C_addr




typedef enum
{
    SET_FALSE = -1,
    SET_OK
} SET_RESULT;


#endif

