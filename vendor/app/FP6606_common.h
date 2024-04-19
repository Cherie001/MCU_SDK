#ifndef __FP6606_COMMON_H__
#define __FP6606_COMMON_H__
#include "global_fun.h"
#include "global_value.h"
#include <tl_common.h>

typedef enum
{
    //ROLE_SRC = 0,
    ROLE_SNK = 1,      /* Accessories are not supported */
    //ROLE_DRP,      /* Dual Role Power */
} tc_role_t;

typedef enum
{
    RP_DEFAULT_CURRENT = 0,
    RP_MEDIUM_CURRENT,     /* 1.5A */
    RP_HIGH_CURRENT        /* 3.0A */
} tcpc_role_rp_val_t;

typedef struct
{
    //unsigned short     flags;
    tc_role_t          role;
    tcpc_role_rp_val_t rp_val;

} tcpc_config_t;

typedef enum
{
    SUPPLY_TYPE_FIXED    = 0,
    SUPPLY_TYPE_BATTERY  = 1,
    SUPPLY_TYPE_VARIABLE = 2,
    SUPPLY_TYPE_APDO = 3,
    SUPPLY_TYPE_NUKNOWN = 4
} supply_type_t;

typedef enum
{
    PEAK_CURRENT_0 = 0,   // Peak current = Ioc default
    PEAK_CURRENT_1 = 1,   // Peak current = 110% Ioc for 10ms
    PEAK_CURRENT_2 = 2,   // Peak current = 125% Ioc for 10ms
    PEAK_CURRENT_3 = 3    // Peak current = 150% Ioc for 10ms
} peak_current_t;

typedef enum
{
    PROGRAM_PSY = 0,
    RESV
}power_supply_t;


typedef struct
{
    supply_type_t          SupplyType;          // Supply type      (fixed, variable, battery)
    peak_current_t         PeakI;               // Peak current
    power_supply_t         Psy;
    unsigned short         MinV;                // Minimum voltage
    unsigned short         MaxV;                // Maximum voltage  (variable, battery)
    unsigned short         MaxOperatingCurrent; // Maximum Current  (fixed, variable)
    unsigned short         MinOperatingCurrent; // Mininum Current  (fixed, variable)
    unsigned short         OperationalCurrent;  // Current          (fixed, variable)
    unsigned short         MaxOperatingPower;   // Maximum Power    (battery only)
    unsigned short         MinOperatingPower;   // Minimum Power    (battery only)
    unsigned short         OperationalPower;    // Power            (battery only)
    unsigned short         OperationalVoltage;   // Voltage
}snk_pdo_t;


#define PD_MAX_PDO_NUM     5

#define PDO_VOLT(mv)       ((unsigned short)((mv)/50))    // 50mV LSB
#define PDO_CURR(ma)       ((unsigned short)((ma)/10))    // 10mA LSB
#define PDO_OP_PWR(mw)     ((unsigned short)((mw)/250))   // 250mW LSB

typedef enum
{
    PRIORITY_VOLTAGE = 0,
    PRIORITY_CURRENT,
    PRIORITY_POWER
} pdo_priority_t;


typedef struct
{
    /* Sink Caps */
    unsigned char      num_snk_pdos;
    snk_pdo_t          snk_caps[PD_MAX_PDO_NUM];

    pdo_priority_t     priority;

    unsigned char      num_snk_apdos;

} usb_pd_port_config_t;
























/** Fp6606 Int Event functions **/
bool tcpm_alert_event(void);
void timer_cancel(DEV_MODE dev_mode);
void tcpc_write_block(u8 reg, u8 *data, u8 len);
void tcpc_read_block(u8 reg, u8 *data, unsigned short len);
void tcpc_read8(unsigned char reg, unsigned char *data);
void tcpc_write16(unsigned char reg, unsigned short data);
void tcpc_write8(unsigned char reg, unsigned char data);
void tcpc_modify8(unsigned char reg, unsigned char clr_mask, unsigned char set_mask);
void timer_start1(DEV_MODE dev_mode,unsigned short timeout_ms,void (*function)(void));
//void timer_start1(DEV_MODE dev_mode,unsigned short timeout_ms,void (*function)(void));
//void timer_cancel(DEV_MODE dev_mode);

#endif //__FP6606_COMMON_H__
