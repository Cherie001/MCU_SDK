#include "FP6606_usb_pd_policy_engine.h"
#include "string.h"
#include <tl_common.h>
#include "stack/ble/ble.h"

#define GET_SRC_PDO_MAX     5
#define PDO_SUPPLY_TYPE(pdo) ((pdo) >> 30)
#define PDO_MAX_CURRENT(pdo) ((pdo) & 0x3FF)
#define PDO_MIN_VOLTAGE(pdo) (((pdo) >> 10) & 0x3FF)
#define PDO_PEAK_CURRENT(pdo) (((pdo) >> 20) & 0x03)

// guding vol
#define FIXED_PDO_MAX_VOLTAGE(pdo) (((pdo) >> 10) & 0x3FF)

// APDO -> PPS
#define APDO_MIN_VOLTAGE(pdo) (((pdo) >> 8) & 0xFF)
#define APDO_MAX_VOLTAGE(pdo) (((pdo) >> 17) & 0xFF)

//


typedef enum
{
    RDO_GIVEBACK_FLAG         = ((unsigned int)1 << 27),
    RDO_CAPABILITY_MISMATCH   = ((unsigned int)1 << 26),
    RDO_USB_COMM_CAPABLE      = ((unsigned int)1 << 25),
    RDO_NO_USB_SUSPEND        = ((unsigned int)1 << 24),
    RDO_UNCHUNKED_MSG_SUPPORT = ((unsigned int)1 << 23)
} rdo_bits_t;


u32 rdo;

static u8  selected_snk_pdo_idx;
static u32 selected_pdo;
static usb_pd_port_config_t pd_port_config[NUM_TCPC_DEVICES];


usb_pd_port_config_t* usb_pd_pm_get_config(void)
{
    return &pd_port_config[0];
}

u32 get_data_object(u8 *obj_data)
{
    return ((((u32)obj_data[3]) << 24) |
            (((u32)obj_data[2]) << 16) |
            (((u32)obj_data[1]) << 8) |
            (((u32)obj_data[0])));
}

void usb_pd_pm_evaluate_src_caps(void)
{
    usb_pd_port_config_t *config = usb_pd_pm_get_config();
    usb_pd_port_t *dev = usb_pd_pe_get_device();
    snk_cap_t  *dev_snkPdo = &snk_pdo;
    u8 i;
    u8 num_offered_pdos = dev->rx_msg_data_len >> 2;
    u8 *pdo_data = dev->rx_msg_buf;
    unsigned short idx_offset = 0;

    u32  m_src_pdo[GET_SRC_PDO_MAX] = {0};
    supply_type_t m_src_supply_type[GET_SRC_PDO_MAX] = {SUPPLY_TYPE_FIXED};

    // Initialize globals for vSafe5V.
    dev->object_position = 1;

    selected_pdo = get_data_object(&pdo_data[0]);

    selected_snk_pdo_idx = 0;

    for(i = 1 , idx_offset = 4; i < num_offered_pdos; i++, idx_offset += 4)
    {
        m_src_pdo[i] = get_data_object(&pdo_data[idx_offset]);
        m_src_supply_type[i] = (supply_type_t)PDO_SUPPLY_TYPE(m_src_pdo[i]);

        DEBUG_LOG("m_src_supply_type[%d] = 0x%x\r\n",i, m_src_supply_type[i]);
        if(m_src_supply_type[i] == SUPPLY_TYPE_APDO)
        {
            if ((APDO_MIN_VOLTAGE(m_src_pdo[i]) >= config->snk_caps[4].MinV) &&
                    (APDO_MAX_VOLTAGE(m_src_pdo[i]) <= config->snk_caps[4].MaxV))
            {
                selected_pdo = get_data_object(&pdo_data[i <<2]);
                dev->object_position = i + 1;
                selected_snk_pdo_idx = 4;
                break;
            }
        }
        else if(m_src_supply_type[i] == SUPPLY_TYPE_FIXED)
        {
/*            if(FIXED_PDO_MAX_VOLTAGE(m_src_pdo[i]) <= config->snk_caps[1].MaxV)   //old
            {
                selected_pdo = get_data_object(&pdo_data[i <<2]);
                dev->object_position = i + 1;
                selected_snk_pdo_idx = 1;
                break;
            }*/

            m_src_pdo[i+1] = get_data_object(&pdo_data[idx_offset+4]);
            m_src_supply_type[i+1] = (supply_type_t)PDO_SUPPLY_TYPE(m_src_pdo[i+1]);
        	if(FIXED_PDO_MAX_VOLTAGE(m_src_pdo[i]) >= config->snk_caps[3].MaxV) //wellsian
        	{
        		selected_pdo = get_data_object(&pdo_data[i <<2]);
        		dev->object_position = i + 1;
        		selected_snk_pdo_idx = 3;		// .port_config.snk_caps[2]

        		DEBUG_LOG("i = %d.\r\n", i);
        		DEBUG_LOG("num_offered_pdos = %d.\r\n", num_offered_pdos);
//        		DEBUG_LOG("FIXED_PDO_MAX_VOLTAGE(m_src_pdo[i]) = %d.\r\n", FIXED_PDO_MAX_VOLTAGE(m_src_pdo[i]));
        		break;
        	}
        	else if((m_src_supply_type[i+1] == SUPPLY_TYPE_APDO ) || (num_offered_pdos == 2))
        	{
        		selected_pdo = get_data_object(&pdo_data[i <<2]);
        		dev->object_position = i + 1;
        		selected_snk_pdo_idx = i;		// .port_config.snk_caps[2]
        		break;
        	}
        	/* 2021/11/30  20:01
            if(FIXED_PDO_MAX_VOLTAGE(m_src_pdo[i]) == config->snk_caps[3].MaxV) //wellsian
            {
                selected_pdo = get_data_object(&pdo_data[i <<2]);
                dev->object_position = i + 1;
                selected_snk_pdo_idx = 3;		// .port_config.snk_caps[2]
                break;
            }
            else if(FIXED_PDO_MAX_VOLTAGE(m_src_pdo[i]) > config->snk_caps[2].MaxV)
            {
            	selected_pdo = get_data_object(&pdo_data[i-1 <<2]);
            	dev->object_position = i;
            	selected_snk_pdo_idx = 2;		// .port_config.snk_caps[2]
            	break;
            }
            else if(FIXED_PDO_MAX_VOLTAGE(m_src_pdo[i]) == config->snk_caps[1].MaxV)
            {
            	selected_pdo = get_data_object(&pdo_data[i <<2]);
            	dev->object_position = i+1;
            	selected_snk_pdo_idx = 1;		// .port_config.snk_caps[2]
            	break;
            }*/
        }
    }

    dev->min_voltage = PDO_MIN_VOLTAGE(selected_pdo) << 1;
    dev_snkPdo->snk_getSrcStatus.srcSupplyType = (supply_type_t)PDO_SUPPLY_TYPE(selected_pdo);
    dev_snkPdo->snk_getSrcStatus.srcPeakCur = (peak_current_t)PDO_PEAK_CURRENT(selected_pdo);
    return;
}

static void build_snk_caps(usb_pd_port_config_t *config)
{
    unsigned int n;
    usb_pd_port_t *dev = usb_pd_pe_get_device();

    // Clear PDOs
    for (n = 0; n < config->num_snk_pdos; n++)
    {
        dev->snk_pdo[n] = 0x00;
    }

    //dev->snk_pdo[0] = (PDO_EXTERNALLY_POWERED_BIT);
    if(config->num_snk_pdos > 0)
    {
        for (n = 0; n < config->num_snk_pdos; n++)
        {
            // SinkPDO(i)(31:30) = pSupplyType(i)
            dev->snk_pdo[n] |= ((u32)(config->snk_caps[n].SupplyType) & 0x03) << 30;

            if (config->snk_caps[n].SupplyType == SUPPLY_TYPE_FIXED)
            {
                // SinkPDO(i)(19:10) = OperationalCurrent(i)
                dev->snk_pdo[n] |= ((u32)(config->snk_caps[n].OperationalCurrent) & 0x3FF) << 10;
                // SinkPDO(i)(9:0) = MaxOperatingCurrent(i)
                dev->snk_pdo[n] |= ((u32)(config->snk_caps[n].MaxOperatingCurrent) & 0x3FF);

            }
            else if (config->snk_caps[n].SupplyType == SUPPLY_TYPE_APDO)
            {
                // APDO(n)[19:9] = SourceCapabilities.APDO(n).OperationalVoltage
                dev->snk_pdo[n] |= ((u32)(config->snk_caps[n].OperationalVoltage) & 0x3FF) << 9;
                // APDO(n)[6:0] = SourceCapabilities.APDO(n).MaximumCurrent
                dev->snk_pdo[n] |= ((u32)(config->snk_caps[n].MaxOperatingCurrent) & 0x7F);
            }
        }
    }
    return;
}


void build_rdo(void)
{
    usb_pd_port_t *dev = usb_pd_pe_get_device();
    usb_pd_port_config_t *config = usb_pd_pm_get_config();
    u8 src_type = PDO_SUPPLY_TYPE(selected_pdo);		// 支持类型

    rdo = 0;
    rdo |= ((u32)dev->object_position) << 28;
    rdo |= config->snk_caps[selected_snk_pdo_idx].MaxOperatingPower & 0x3FF;

    if (PDO_MAX_CURRENT(selected_pdo) < config->snk_caps[selected_snk_pdo_idx].MaxOperatingCurrent)
    {
        // Sink requires higher current for full operation.
        rdo |= RDO_CAPABILITY_MISMATCH;
    }

    if (src_type == SUPPLY_TYPE_BATTERY)
    {
        dev->snk_pps_req_cnt = 0;
        rdo |= ((u32)config->snk_caps[selected_snk_pdo_idx].OperationalPower & 0x3FF) << 10;
    }
    /* Fixed or Variable supply */
    else if((src_type == SUPPLY_TYPE_FIXED) || (src_type == SUPPLY_TYPE_VARIABLE))
    {
        dev->snk_pps_req_cnt = 0;

        if (PDO_MAX_CURRENT(selected_pdo) >= config->snk_caps[selected_snk_pdo_idx].OperationalCurrent)
        {
            rdo |= ((unsigned int)config->snk_caps[selected_snk_pdo_idx].OperationalCurrent & 0x3FF) << 10;
        }
        else
        {
            rdo |= PDO_MAX_CURRENT(selected_pdo);
        }

        if (PDO_MAX_CURRENT(selected_pdo) < config->snk_caps[selected_snk_pdo_idx].MaxOperatingCurrent)
        {
            rdo |= PDO_MAX_CURRENT(selected_pdo);
        }
		else
		{
            rdo |= ((u32)config->snk_caps[selected_snk_pdo_idx].MaxOperatingCurrent & 0x3FF);
		}
    }
    else //apdo
    {
        if(selected_snk_pdo_idx == 4)
        {
            rdo |= ((unsigned int)config->snk_caps[selected_snk_pdo_idx].OperationalVoltage & 0x7FF ) << 9;   // operating voltage
            rdo |= ((unsigned int)config->snk_caps[selected_snk_pdo_idx].MaxOperatingCurrent & 0x7F );
            dev->snk_pps_req_cnt = 1;
        }
    }

    return;
}


void usb_pd_init(const usb_pd_port_config_t *port_config)
{
    WaitMs(50);
    memcpy(pd_port_config, port_config, sizeof(pd_port_config));
    build_snk_caps(&pd_port_config[0]);
    usb_pd_pe_init(&pd_port_config[0]);
    return;
}



