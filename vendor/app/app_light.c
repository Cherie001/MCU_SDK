#include "app_light.h"
#include "log.h"
#include "light.h"
#include "drivers.h"
#include "bs_protocol.h"
#include "tl_common.h"
#include "flash_env.h"
#include "fan.h"

extern LightParam _lightParam;
//volatile LightParam _lightParam;
bs_light_param_t _bsLightParam;
//bs_light_param_t _bsLightParam_ai;
u8 gLightSta = 0;

void bs_light_save_flash(void)
{
	u8 buff[FLASH_ENV_UNIT_BYTE];
	memset(buff,0,FLASH_ENV_UNIT_BYTE);

	buff[0] = gLightSta;
	memcpy(buff+1,(u8 *)&_bsLightParam, sizeof(bs_light_param_t));

	flash_set(FLASH_LIGHT_PARAM_ADDR,buff,FLASH_ENV_UNIT_BYTE-1);
	DEBUG_LOG("Write flash light Sta & param.\r\n");
}

void bs_light_read_flash(void)
{
	u8 buff[FLASH_ENV_UNIT_BYTE];
	memset(buff,0,FLASH_ENV_UNIT_BYTE);

	if(flash_get(FLASH_LIGHT_PARAM_ADDR,buff,FLASH_ENV_UNIT_BYTE-1) == false)
	{
		DEBUG_LOG("Read light param from flash fail.use defalut settings.\r\n");
		gLightSta= 1;
		_bsLightParam.Mode = LIGHT_MODE_RAINBOW;
		_bsLightParam.Speed = 3600;
		_bsLightParam.Timeout = 0xFFFF;
		_bsLightParam.ColorCount = 0x0;

		bs_light_save_flash();
	}
	else
	{
		gLightSta = buff[0];
		memcpy((u8 *)&_bsLightParam, buff+1, sizeof(bs_light_param_t));

		if(_bsLightParam.Mode > LIGHT_MODE_OFF && _bsLightParam.Mode < LIGHT_MODE_MAXEND)
		{
			DEBUG_LOG("Light mode is ok.\r\n");
		}
		else
		{
			gLightSta= 1;
			_bsLightParam.Mode = LIGHT_MODE_RAINBOW;
			_bsLightParam.Speed = 3600;
			bs_light_save_flash();
		}

	}

	DEBUG_LOG("Read flash light Sta = %d.\r\n",gLightSta);
	DEBUG_LOG("Read flash light Mode = %d.\r\n",_bsLightParam.Mode);
	DEBUG_LOG("Read flash Timeout = %d.\r\n",_bsLightParam.Timeout);
	DEBUG_LOG("Read flash Speed = %d.\r\n",_bsLightParam.Speed);
	DEBUG_LOG("Read flash ColorCount = %d.\r\n",_bsLightParam.ColorCount);
	DEBUG_LOG("Read flash light rgb[0][0] = %d,[0][1] = %d,[0][2] = %d.\r\n",_bsLightParam.rgb[0][0],_bsLightParam.rgb[0][1],_bsLightParam.rgb[0][2]);
	DEBUG_LOG("Read flash light rgb[1][0] = %d,[1][1] = %d,[1][2] = %d.\r\n",_bsLightParam.rgb[1][0],_bsLightParam.rgb[1][1],_bsLightParam.rgb[1][2]);
	DEBUG_LOG("Read flash light rgb[2][0] = %d,[2][1] = %d,[2][2] = %d.\r\n",_bsLightParam.rgb[2][0],_bsLightParam.rgb[2][1],_bsLightParam.rgb[2][2]);

}

void bs_light_update(void)
{
	if(gLightSta == 0)
	{
		_lightParam.typeBak = LIGHT_MODE_OFF;
		_lightParam.type = _lightParam.typeBak;
		return ;
	}

	switch(_bsLightParam.Mode)
	{
	case LIGHT_MODE_RAINBOW:		// Rainbow
	case LIGHT_MODE_LEDBREATH:		// Breath
	case LIGHT_MODE_STARLIGHT:
	case LIGHT_MODE_FLYINGRING:
	case LIGHT_MODE_ALLON:
	case LIGHT_MODE_CHASE:
	case LIGHT_MODE_FLOW:
	case LIGHT_MODE_THRCOLOR:
	case LIGHT_MODE_BUFFDELAY:
	case LIGHT_MODE_BOUNCEV1:
	case LIGHT_MODE_BOUNCEV2:
	{
		_lightParam.cycleMs = _bsLightParam.Speed;
		_lightParam.typeBak = _bsLightParam.Mode;
		_lightParam.type = _lightParam.typeBak;
		_lightParam.colorNum = _bsLightParam.ColorCount;

		if(_lightParam.colorNum >= 12) {
			_lightParam.colorNum = 0;
		}
		for(u8 i = 0; i < _lightParam.colorNum; i++) {
			_lightParam.rgb[i][0] =  _bsLightParam.rgb[i][0];
			_lightParam.rgb[i][1] =  _bsLightParam.rgb[i][1];
			_lightParam.rgb[i][2] =  _bsLightParam.rgb[i][2];
		}
		_lightParam.typeBak = LIGHT_MODE_DISABLE;
	}
	break;
	case LIGHT_MODE_OFF:
	default:
		break;
	}
}

/*************************************************************/
static int bs_light_fac_test(bs_cmd_t *cmd)
{
	u8 rgb =0;
	bs_cmd_t *data = cmd;

	rgb = data->param[0];

	DEBUG_LOG("BS APP light fac test,rgb = %d.\r\n",rgb);

	switch(rgb)
	{
	case 1:
		_lightParam.typeBak = LIGHT_MODE_RED;
		_lightParam.type = _lightParam.typeBak;
		break;
	case 2:
		_lightParam.typeBak = LIGHT_MODE_GREEN;
		_lightParam.type = _lightParam.typeBak;
		break;
	case 3:
		_lightParam.typeBak = LIGHT_MODE_BLUE;
		_lightParam.type = _lightParam.typeBak;
		break;
	case 4:
		_lightParam.typeBak = LIGHT_MODE_WHITE;
		_lightParam.type = _lightParam.typeBak;
		break;
	default:
		_lightParam.typeBak = LIGHT_MODE_OFF;
		_lightParam.type = _lightParam.typeBak;
		break;
	}

	data->param[0] = 0x01;
	data->head.bit.length = sizeof(bs_cmd_t) + 1;
	u8 ret = bs_prot_cmd_send(cmd);
	return ret;
}

static int bs_light_set(bs_cmd_t *cmd)
{
	bs_cmd_t *data = cmd;

	DEBUG_LOG("BS APP set light status = %d.\r\n",data->param[0]);

	gLightSta = data->param[0];

	if(gLightSta == 0)
	{
		_lightParam.typeBak = LIGHT_MODE_OFF;
		_lightParam.type = _lightParam.typeBak;
	}
	else
	{
		_lightParam.typeBak = _bsLightParam.Mode;
		_lightParam.type = _lightParam.typeBak;
	}

	bs_light_save_flash();
	bs_light_update();

	data->param[0] = 0x01;
	data->head.bit.length = sizeof(bs_cmd_t) + 1;
	u8 ret = bs_prot_cmd_send(cmd);
	return ret;
}

static int bs_light_get(bs_cmd_t *cmd)
{
	bs_cmd_t *data = cmd;

	DEBUG_LOG("BS APP get light status = %d.\r\n",gLightSta);
	data->param[0] = gLightSta;
	data->head.bit.length = sizeof(bs_cmd_t) + 1;
	u8 ret = bs_prot_cmd_send(cmd);
	return ret;
}

static int bs_light_mode_set(bs_cmd_t *cmd)
{
	u16 mode_A, mode_B;
	u8 mode_C;
	bs_cmd_t *data = cmd;

	bs_light_param_t *ptr = (bs_light_param_t *)data->param;

	DEBUG_LOG("BS APP Set light param data lenght = %d.\r\n",data->head.bit.length-sizeof(bs_cmd_t));

	u16 mode = ptr->Mode;

	memcpy(&_bsLightParam,data->param,data->head.bit.length-sizeof(bs_cmd_t));

	mode_A = _bsLightParam.Mode;

	DEBUG_LOG("BS APP Set light Mode1 = %x.\r\n",_bsLightParam.Mode);

	_bsLightParam.Mode &= 0x00FF;

	if(_bsLightParam.Mode > LIGHT_MODE_OFF && _bsLightParam.Mode < LIGHT_MODE_MAXEND)
	{
		bs_light_save_flash();
	}
	bs_light_update();

	mode_B = _bsLightParam.Mode;

	DEBUG_LOG("BS APP Set light Mode = %d.\r\n",_bsLightParam.Mode);
	DEBUG_LOG("BS APP Set light Timeout = %d.\r\n",_bsLightParam.Timeout);
	DEBUG_LOG("BS APP Set light Speed = %d.\r\n",_bsLightParam.Speed);
	DEBUG_LOG("BS APP Set light ColorCount = %d.\r\n",_bsLightParam.ColorCount);
	DEBUG_LOG("BS APP Set light rgb[0][0] = %d,[0][1] = %d,[0][2] = %d.\r\n",_bsLightParam.rgb[0][0],_bsLightParam.rgb[0][1],_bsLightParam.rgb[0][2]);
	DEBUG_LOG("BS APP Set light rgb[1][0] = %d,[1][1] = %d,[1][2] = %d.\r\n",_bsLightParam.rgb[1][0],_bsLightParam.rgb[1][1],_bsLightParam.rgb[1][2]);
	DEBUG_LOG("BS APP Set light rgb[2][0] = %d,[2][1] = %d,[2][2] = %d.\r\n",_bsLightParam.rgb[2][0],_bsLightParam.rgb[2][1],_bsLightParam.rgb[2][2]);

	data->param[0] = 0x01;
	data->head.bit.length = sizeof(bs_cmd_t) + 1 + 3;
//	data->head.bit.length = sizeof(bs_cmd_t) + 1;

	memcpy(data->param+1, &mode,sizeof(u16));
//	memcpy(data->param+3, &mode_A,sizeof(u16));
//	memcpy(data->param+5, &mode_B,sizeof(u16));
	memcpy(data->param+3, &mode_C,sizeof(u8));
	u8 ret = bs_prot_cmd_send(cmd);

	return ret;
}

static int bs_light_mode_get(bs_cmd_t *cmd)
{
	bs_cmd_t *data = cmd;

	memcpy(data->param,(u8 *)&_bsLightParam,sizeof(bs_light_param_t));

	DEBUG_LOG("BS APP Get light param data lenght = %d.\r\n",data->head.bit.length);
	DEBUG_LOG("BS APP Get light Sta = %d.\r\n",gLightSta);
	DEBUG_LOG("BS APP Get light Mode = %d.\r\n",_bsLightParam.Mode);
	DEBUG_LOG("BS APP Get light Timeout = %d.\r\n",_bsLightParam.Timeout);
	DEBUG_LOG("BS APP Get light Speed = %d.\r\n",_bsLightParam.Speed);
	DEBUG_LOG("BS APP Get light ColorCount = %d.\r\n",_bsLightParam.ColorCount);
	DEBUG_LOG("BS APP Get light rgb[0][0] = %d,[0][1] = %d,[0][2] = %d.\r\n",_bsLightParam.rgb[0][0],_bsLightParam.rgb[0][1],_bsLightParam.rgb[0][2]);
	DEBUG_LOG("BS APP Get light rgb[1][0] = %d,[1][1] = %d,[1][2] = %d.\r\n",_bsLightParam.rgb[1][0],_bsLightParam.rgb[1][1],_bsLightParam.rgb[1][2]);
	DEBUG_LOG("BS APP Get light rgb[2][0] = %d,[2][1] = %d,[2][2] = %d.\r\n",_bsLightParam.rgb[2][0],_bsLightParam.rgb[2][1],_bsLightParam.rgb[2][2]);


	data->head.bit.length = sizeof(bs_cmd_t)+sizeof(bs_light_param_t);
	u8 ret = bs_prot_cmd_send(data);
	return ret;
}

static int default_pack_process (bs_cmd_t *cmd)
{
	return 0;
}

static bs_cmd_node_t bs_led_module_tbl[] = {
		{LIGHT_MODE_FAC_CMD, default_pack_process, bs_light_fac_test},		// 0x00
		{LIGHT_MODE_SET_CMD, default_pack_process, bs_light_set},			// 0x08
		{LIGHT_MODE_GET_CMD, default_pack_process, bs_light_get},			// 0x10
		{LIGHT_MODE_PARAM_GET_CMD, default_pack_process, bs_light_mode_get},// 0x18
		{LIGHT_MODE_PARAM_SET_CMD, default_pack_process, bs_light_mode_set},// 0x20
};

int light_profile_init(void)
{
	return (bs_prot_cmd_tbl_register(BS_CMD_SETID_LED, ARRAY_SIZE(bs_led_module_tbl), NULL, &bs_led_module_tbl[0]));
}
