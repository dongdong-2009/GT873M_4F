#include "gwd_poe.h"
#include "cs_cmn.h"
#include "sdl_peri_util.h"
#include "onu_syscfg.h"
#include "sdl_port.h"
#include "cli_common.h"

#if (RPU_MODULE_POE == MODULE_YES)
#define CPLD_OP_READ 0x03
#define CPLD_OP_WRITE 0x02

#ifndef DATA_SHOW
#define DATA_SHOW		1
#endif

PoeOperation_t PoeOperation;

unsigned int gwd_poe_thread_id = 0;
#define GWD_POE_THREAD_NAME "gwd_poe"
#define GWD_POE_THREAD_STACKSIZE               (2 * 4096)

unsigned int gulPoeEnable = 0;
unsigned char gucPoeDisablePerPort[UNI_PORT_MAX] = {0};
unsigned char gucPoedefaultconfig[UNI_PORT_MAX] = {0};

extern int cmd_onu_cpld_reg_cfg_set(struct cli_def *cli, char *command, char *argv[], int argc);
extern int gw_cli_interface_debug_terminal(struct cli_def *cli, char *command, char *argv[], int argc);

epon_return_code_e Gwd_onu_poe_exist_stat_set(unsigned int poe_stat)
{
    gulPoeEnable = poe_stat;
    return EPON_RETURN_OK;
}
epon_return_code_e Gwd_onu_poe_exist_stat_get(unsigned int *poe_stat)
{
	unsigned char ret = 0;
	if(NULL != poe_stat)
	{
		*poe_stat = gulPoeEnable;
		ret = EPON_RETURN_OK;
	}
	else
	{
		ret = EPON_RETURN_ERROR;
	}
    return ret;
}

int gwd_poe_cpld_read(unsigned int type,unsigned char* val)
{

    cs_callback_context_t    context;
    unsigned int ret = EPON_RETURN_OK;	

	ret += cs_plat_ssp_cpld_write(context, 0, 0, CPLD_OP_READ);
    ret += cs_plat_ssp_cpld_write(context,0,0,type);
    ret += cs_plat_ssp_cpld_read(context,0,0,val);
	return ret;
}
int gwd_poe_cpld_write(unsigned int type,unsigned char val)
{
    cs_callback_context_t    context;
    unsigned int ret = EPON_RETURN_OK;	
	
	ret += cs_plat_ssp_cpld_write(context, 0, 0, CPLD_OP_WRITE);
    ret += cs_plat_ssp_cpld_write(context,0,0,type);
    ret += cs_plat_ssp_cpld_write(context,0,0,val);
	return ret;
}


int onu_cpld_read_register(unsigned int type,unsigned char* val)
{

    unsigned int reg = 0;
    unsigned int ret = EPON_RETURN_OK;

    if(!val)
    {
        return EPON_RETURN_ERROR;
    }
    
    switch(type)
    {
        case GWD_CPLD_VERSION_REG:
        case GWD_CPLD_RESERVED_REG:
        case GWD_CPLD_POWER3_REG:
        case GWD_CPLD_RESET_REG:
        case GWD_CPLD_POWER2_REG:
        case GWD_CPLD_POWER_ALARM_REG:
        case GWD_CPLD_POWER1_REG:
        case GWD_CPLD_PROTECT_REG:
            reg = type;
            if(gwd_poe_cpld_read(reg,val) != EPON_RETURN_OK)
            {
                cs_printf("read cpld register(0x%02x) fail\n",reg);
            }
            
            break;
           default:
              ret = EPON_RETURN_ERROR;
                break;
    }
    if(ret)
    {
    	cs_printf("read cpld register type error\n");
        return EPON_RETURN_ERROR;
    }
    return EPON_RETURN_OK;
}

int onu_cpld_write_register(unsigned int type,unsigned int val)
{
    unsigned int reg = 0;
    int ret = EPON_RETURN_OK;

    switch(type)
    {
        case GWD_CPLD_VERSION_REG:
        case GWD_CPLD_RESERVED_REG:
        case GWD_CPLD_POWER3_REG:
        case GWD_CPLD_RESET_REG:
        case GWD_CPLD_POWER2_REG:
        case GWD_CPLD_POWER_ALARM_REG:
        case GWD_CPLD_POWER1_REG:
            reg = type;
            
            if((ret += gwd_poe_cpld_write(GWD_CPLD_PROTECT_REG,UNLOCK_CPLD)) != EPON_RETURN_OK)
            {
                cs_printf("UNLOCK CPLD fail\n");
            }
            if((ret += gwd_poe_cpld_write(reg,val)) != EPON_RETURN_OK)
            {
            	cs_printf("write cpld register(0x%02x) fail\n",reg);
            }
            if((ret += gwd_poe_cpld_write(GWD_CPLD_PROTECT_REG,LOCK_CPLD)) != EPON_RETURN_OK)
            {
            	cs_printf("LOCK CPLD fail\n");
            }
            break;
           default:
              ret = EPON_RETURN_ERROR;
                break;
    }
    if(ret)
    {
    	cs_printf("write cpld register type error\n");
        return EPON_RETURN_ERROR;
    }
    return EPON_RETURN_OK;
}




int gwd_poe_cpld_check(unsigned char *val)
{
    int ret = 0;
    ret = EPON_RETURN_EXIST_OK;
	
    if(onu_cpld_read_register(GWD_CPLD_VERSION_REG,val) != EPON_RETURN_OK)
    {
        return EPON_RETURN_ERROR;
    }

    if((*val < GWD_CPLD_VERSION_1)||(0xff == *val))
    {
        ret = EPON_RETURN_EXIST_ERROR;
    }
	    
    return ret;
}

epon_return_code_e Gwd_onu_port_poe_controle_stat_get(unsigned int port, unsigned int* poe_ctl_state)
{
    if(poe_ctl_state == NULL)
        return EPON_RETURN_ERROR;
    
    if((port > UNI_PORT_MAX) || port < 1)
        return EPON_RETURN_ERROR;

    *poe_ctl_state = gucPoeDisablePerPort[port - 1];

    return EPON_RETURN_OK;
}

epon_return_code_e Gwd_onu_port_poe_controle_stat_set(unsigned int port, unsigned int poe_ctl_state)
{    
    if((port > UNI_PORT_MAX) || port < 1)
        return EPON_RETURN_ERROR;

   gucPoeDisablePerPort[port - 1] = (poe_ctl_state == POE_PORT_ENABLE)? POE_PORT_ENABLE:POE_PORT_DISABLE;

    return EPON_RETURN_OK;
}

extern int gwd_onu_tlv_infor_get(int *len, char **value, int *free_need)
{
	int ret = 0;
	//入口规则检查
	if(NULL == len)
	{
		cs_printf("arg check err!\n");
		cs_printf("in %s, line :%d\n", __func__, __LINE__);
		ret=-1;
	}
	else
	{
		*len = sizeof(gucPoeDisablePerPort);
	}
	if(NULL == value)
	{
		cs_printf("arg check err!\n");
		cs_printf("in %s, line :%d\n", __func__, __LINE__);
		ret=-1;
	}
	else
	{
		*value = gucPoeDisablePerPort;
	}
	if(NULL == free_need)
	{
		cs_printf("arg check err!\n");
		cs_printf("in %s, line :%d\n", __func__, __LINE__);
		ret=-1;
	}
	else
	{
		*free_need = 0;
	}
	return ret;
}
extern int gwd_onu_tlv_infor_handle(int length,char *value,int opcode)
{
	if(NULL == value)
	{
		return -1;
	}
	if(length!=sizeof(gucPoeDisablePerPort))
	{
		return -1;
	}
	if(DATA_SHOW != opcode)
	{
		memcpy(gucPoeDisablePerPort,value,sizeof(gucPoeDisablePerPort));
	}
	if(DATA_SHOW == opcode)
	{
		unsigned int lport = 0;
		unsigned int uni_port_num = UNI_PORT_MAX;
		for(lport = 1; lport <= uni_port_num; lport++)
		{
				cs_printf("UNI Port %d : %s\r\n",lport,value[lport-1]?"POE CONTROL ENABLE":"POE CONTROL DISABLE");
		}
	}
	return 0;
}


epon_return_code_e Gwd_onu_port_power_detect_get(unsigned int port,unsigned int* port_power_stat)
{
    unsigned int ret = 0;
    unsigned char power_stat = 0;
    unsigned int uni_port_num = 0;

    uni_port_num = UNI_PORT_MAX;
    
    if(port_power_stat == NULL)
        return EPON_RETURN_ERROR;

    if((port < 1) || port > uni_port_num)
        return EPON_RETURN_ERROR;
    
    if(onu_cpld_read_register(GWD_CPLD_POWER1_REG,&power_stat) != EPON_RETURN_OK)
    {
        ret = EPON_RETURN_ERROR;
    }

    PORT_POWER_STAT(power_stat,port);
    if(power_stat)
        *port_power_stat = POE_POWER_ENABLE;
    else
        *port_power_stat = POE_POWER_DISABLE;

    return EPON_RETURN_OK;
}

epon_return_code_e Gwd_onu_port_poe_operation_stat_set(int lport,int state)
{
	cs_callback_context_t context;
    
    PoeOperation.PoeOperationflag = TRUE;
//    cs_printf("Gwd_onu_port_poe_operation_stat_set lport is %d,state is %d\r\n",lport,state);
    if(epon_request_onu_port_admin_set(context,0,0,lport,state) != EPON_RETURN_OK)
    {
        cs_printf("set port poe operation fail\n");
        return EPON_RETURN_ERROR;
    }

    return EPON_RETURN_OK;
}

void gwd_onu_poe_thread_hander()
{
	cs_callback_context_t context;
    unsigned int uni_port_num = UNI_PORT_MAX;
    unsigned int ulport =0;
    unsigned int port_stat = 0;
    unsigned int Pstate = 0;
    unsigned int ctl_state = 0;
        
    while(1)
    {
        for(ulport = 1;ulport <= uni_port_num; ulport++)
        {
            if(epon_request_onu_port_admin_get(context, 0, 0, ulport,&port_stat) != EPON_RETURN_OK)
            {
                continue;
            }
            if(Gwd_onu_port_poe_controle_stat_get(ulport,&ctl_state) != EPON_RETURN_OK)
            {
                cs_printf("get port:%d  admin fail:\n",ulport);
                continue;
            }
//            cs_printf("port_stat is %d,ctl_state is %d\r\n",port_stat,ctl_state);
            if(port_stat == PORT_ADMIN_UP)
            {
                if(ctl_state == POE_PORT_ENABLE)
                {
                    if(Gwd_onu_port_power_detect_get(ulport,&Pstate) != EPON_RETURN_OK)
                    {
                    	cs_printf("get port:%d  power fail:\n",ulport);
                        continue;
                    }
//                    cs_printf("pstate is %d\r\n",Pstate);
                    if(Pstate == POE_POWER_DISABLE)
                    {
                        Gwd_onu_port_poe_operation_stat_set(ulport,PORT_ADMIN_DOWN);
                    }
                }
                                    
            }
            else
            {

                if(ctl_state == POE_PORT_ENABLE)
                {
                    if(Gwd_onu_port_power_detect_get(ulport,&Pstate) != EPON_RETURN_OK)
                    {
                    	cs_printf("get port:%d power fail\n",ulport);
                        continue;
                    }
//                    cs_printf("pstate is %d\r\n",Pstate);
                    if(Pstate == POE_POWER_ENABLE)
                    {
                         Gwd_onu_port_poe_operation_stat_set(ulport,PORT_ADMIN_UP);
                    }
                }
                else
                {
//                	cs_printf("else else\r\n");
                    Gwd_onu_port_poe_operation_stat_set(ulport,PORT_ADMIN_UP);
                }
                    
            }
        }

        cs_thread_delay(500);
    }
}

epon_return_code_e gwd_poe_cpld_init()
{
    unsigned char val = 0;


    if(onu_cpld_write_register(GWD_CPLD_RESET_REG,val) != EPON_RETURN_OK)
    {
        return EPON_RETURN_ERROR;
    }
	if(onu_cpld_read_register(GWD_CPLD_POWER_ALARM_REG,&val) != EPON_RETURN_OK)
	{
		
	}
    val |= 1<<7;
    if(onu_cpld_write_register(GWD_CPLD_POWER_ALARM_REG,val) != EPON_RETURN_OK)
    {
       return EPON_RETURN_ERROR;
    }
    
    return EPON_RETURN_OK;
}
epon_return_code_e gwdonu_port_poe_operation_table_init()
{
	int port = 0;
	PoeOperation.PoeOperationflag = 0;
	
	for(port = 0; port < UNI_PORT_MAX;port++)
	{
	   PoeOperation.CommandOperation[port] = 1;
	   PoeOperation.PoeOpertable[port] = 1;
	}
	return CS_OK;
}
epon_return_code_e gwd_poe_module_init()
{
    memset(gucPoeDisablePerPort,0,sizeof(gucPoeDisablePerPort)); //set poe disable at start
	return EPON_RETURN_OK;
}

void gwd_poe_init()
{
	int ret = -1;
	unsigned char val = 0;
	
	ret = gwd_poe_cpld_check(&val);
	cs_printf("GWD CPLD VERSION 0x%x , %s !\r\n", val, ret==EPON_RETURN_EXIST_OK?"VALID":"INVALID");
	if(EPON_RETURN_EXIST_OK == ret)
	{
		Gwd_onu_poe_exist_stat_set(1);
		gwd_poe_cpld_init();
		gwdonu_port_poe_operation_table_init();
		gwd_poe_module_init();
	    cs_thread_create(&gwd_poe_thread_id, GWD_POE_THREAD_NAME, gwd_onu_poe_thread_hander, NULL, GWD_POE_THREAD_STACKSIZE, PORT_STATS_THREAD_PRIORITY, 0);
	}
	else
	{
		Gwd_onu_poe_exist_stat_set(0);
		return;
	}
}


#endif
#if (RPU_MODULE_PSE == MODULE_YES)
#define GWD_PSE1_ADDR 	0x21
#define GWD_PSE2_ADDR 	0x22

#define MAX5980_REG_MODE		0x12
#define MAX5980_REG_DET_CLASS	0x14
#define MAX5980_REG_POWER_PUSH	0x19
#define MAX5980_REG_GLOBAL		0x1A
#define MAX5980_REG_ID			0x1B


#define MAX5980_ID_LEGAL		0xd0

#define PSE_SHUTDOWN		0x0
#define PSE_MANUAL			0x1
#define PSE_SEMIAUTO		0x2
#define PSE_AUTO			0x3

cs_uint8 GwdPseMode[8] = {0};
cs_uint8 GwdPseEnable[8] = {0};
cs_uint32 GwdPsePower[8] = {0};
cs_uint32 uni_port_num = UNI_PORT_MAX;

extern cs_status cs_plat_i2c_read (
    	CS_IN   cs_callback_context_t    	context,
    	CS_IN   cs_dev_id_t              	device,
    	CS_IN   cs_llid_t                	llidport,
    	CS_IN   cs_uint8                 	slave_addr,
    	CS_IN   cs_uint8                 	slave_offset,
    	CS_IN   cs_uint32                	len,
    	CS_OUT  cs_uint8                 	*data);
extern cs_status cs_plat_i2c_write (
    	CS_IN   cs_callback_context_t    	context,
    	CS_IN   cs_dev_id_t              	device,
    	CS_IN   cs_llid_t                	llidport,
    	CS_IN   cs_uint8                 	slave_addr,
    	CS_IN   cs_uint8                 	slave_offset,
    	CS_IN   cs_uint32                	len,
    	CS_IN   cs_uint8                 	*data);

cs_uint32 gwd_pse_enable_set(cs_uint32 port, cs_uint8 mode)
{
    cs_callback_context_t    	context;
	cs_uint8 reg_val = 0, ret = 0;
	cs_uint8 slave_addr = 0;
	cs_uint8 phyPort = port - 1;
	if((port<1) || (port >uni_port_num))
	{
		return CS_ERROR;
	}
	if(mode >1)
	{
		cs_printf("mode error (%d)\n",mode);
		return CS_ERROR;
	}
	if(GwdPseEnable[phyPort] == mode)
	{
		return CS_OK;
	}
	slave_addr = phyPort < 4 ?GWD_PSE1_ADDR:GWD_PSE2_ADDR;
	if(0 == mode)
	{
		cs_plat_i2c_read(context, 0, 0, slave_addr, MAX5980_REG_POWER_PUSH, 1, &reg_val);
		reg_val = 1<<(phyPort%4+4);
//		cs_printf("reg_val is 0x%x\n",reg_val);
		ret = cs_plat_i2c_write(context,0,0,slave_addr,MAX5980_REG_POWER_PUSH, 1,&reg_val);
	}
	else
	{
		cs_plat_i2c_read(context, 0, 0, slave_addr, MAX5980_REG_DET_CLASS, 1, &reg_val);
		reg_val = (1<<(phyPort%4+4))|(1<<(phyPort%4));
//		cs_printf("reg_val is 0x%x\n",reg_val);
		ret = cs_plat_i2c_write(context,0,0,slave_addr,MAX5980_REG_DET_CLASS, 1,&reg_val);
	}
	if(CS_OK == ret)
	{
		GwdPseEnable[phyPort] = mode;
	}
	return CS_OK;
}

cs_uint32 gwd_pse_enable_get(cs_uint32 port, cs_uint8* mode)
{
	cs_uint32 phyPort = port - 1;
	if((port<1) || (port >uni_port_num))
		return CS_ERROR;
	if(NULL == mode)
	{
		return CS_ERROR;
	}
	*mode = GwdPseEnable[phyPort];
	return CS_OK;
}

cs_uint32 gwd_pse_mode_set(cs_uint32 port, cs_uint8 mode)
{
    cs_callback_context_t    	context;
	cs_uint8 port_mode = 0, ret = 0;
	cs_uint8 slave_addr = 0;
	cs_uint8 phyPort = port - 1;
	if((port<1) || (port >uni_port_num))
	{
		return CS_ERROR;
	}
	if(mode >PSE_AUTO)
	{
		cs_printf("mode error (%d)\n",mode);
		return CS_ERROR;
	}
	slave_addr = phyPort < 4 ?GWD_PSE1_ADDR:GWD_PSE2_ADDR;
	cs_plat_i2c_read(context, 0, 0, slave_addr, MAX5980_REG_MODE, 1, &port_mode);

	port_mode &= ~(0x3 << ((phyPort % 4)*2));

	port_mode |= mode << ((phyPort % 4)*2);
	ret = cs_plat_i2c_write(context,0,0,slave_addr,MAX5980_REG_MODE, 1,&port_mode);
	if(CS_OK != ret)
	{
		cs_printf("pse mode set error! port(%d) mode(%d)\n",port,mode);
		return CS_ERROR;
	}
	GwdPseMode[phyPort] = mode;
	return CS_OK;
}

cs_uint32 gwd_pse_mode_get(cs_uint32 port, cs_uint8* mode)
{
	cs_uint32 phyPort = port - 1;
	if((port<1) || (port >uni_port_num))
		return CS_ERROR;
	if(NULL == mode)
	{
		return CS_ERROR;
	}
	*mode = GwdPseMode[phyPort];
	return CS_OK;
}

cs_uint32 gwd_pse_info_show(cs_uint32 port, cs_uint8 pse_mode)
{
	cs_uint8 ret = 0;
	cs_uint8 mode[15] = {0};
	switch(pse_mode)
	{
        case PSE_SHUTDOWN:
            strncpy(mode, "PSE_SHUTDOWN", sizeof(mode));
            break;

        case PSE_MANUAL:
            strncpy(mode, "PSE_MANUAL", sizeof(mode));
            break;

		case PSE_SEMIAUTO:
            strncpy(mode, "PSE_SEMIAUTO", sizeof(mode));
            break;

		case PSE_AUTO:
            strncpy(mode, "PSE_AUTO", sizeof(mode));
            break;

        default:
            strncpy(mode, "UNKNOW", sizeof(mode));
            break;
	}
	cs_printf("Port %d Mode %s \n",port, mode);

	return ret;
}
void gwd_pse_reset()
{
    cs_callback_context_t    	context;
	cs_uint8 max_reset = 1<<4;

	cs_plat_i2c_write(context,0,0,GWD_PSE1_ADDR,MAX5980_REG_GLOBAL, 1,&max_reset);
	cs_plat_i2c_write(context,0,0,GWD_PSE2_ADDR,MAX5980_REG_GLOBAL, 1,&max_reset);
	return;
}
extern cs_status cs_i2c_speed_set(cs_uint8 slave_addr, cs_uint32 freq_khz);
void gwd_pse_init()
{
    cs_callback_context_t    	context;
    cs_uint8 ret = CS_OK, i = 0;
    cs_uint8 reg_val1 = 0;
    cs_uint8 reg_val2 = 0;

    /* Init PSE speed */
    cs_i2c_speed_set(GWD_PSE1_ADDR,70);
    cs_i2c_speed_set(GWD_PSE2_ADDR,70);
    /* */
    gwd_pse_reset();
    cyg_thread_delay(100);

    cs_plat_i2c_read(context, 0, 0, GWD_PSE1_ADDR, MAX5980_REG_ID, 1, &reg_val1);
    cs_plat_i2c_read(context, 0, 0, GWD_PSE2_ADDR, MAX5980_REG_ID, 1, &reg_val2);
    if((MAX5980_ID_LEGAL != reg_val1)||(MAX5980_ID_LEGAL != reg_val2))
    {
    	cs_printf("GWD PSE init failed! reg_val is 0x%x 0x%x\n", reg_val1, reg_val2);
    	return;
    }

    for(i=1; i<=uni_port_num; i++)
    {
    	ret += gwd_pse_mode_set(i, PSE_AUTO);
    }
    memset(GwdPseEnable,1,sizeof(GwdPseEnable));
    if(CS_OK == ret)
    	cs_printf("GWD PSE init successful!\n");
    else
    	cs_printf("GWD PSE init faild! ret is %d\n",ret);

    return;
}
#endif
