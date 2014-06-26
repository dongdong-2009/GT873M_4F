#include "gwd_poe.h"
#include "cs_cmn.h"
#include "sdl_peri_util.h"
#include "onu_syscfg.h"
#include "sdl_port.h"
#include "cli_common.h"

#if (RPU_MODULE_POE == MODULE_YES)
#define CPLD_OP_READ 0x03
#define CPLD_OP_WRITE 0x02

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

        cs_thread_delay(5000);
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
	    cs_thread_create(&gwd_poe_thread_id, GWD_POE_THREAD_NAME, gwd_onu_poe_thread_hander, NULL, GWD_POE_THREAD_STACKSIZE, PORT_STATS_THREAD_PRIORITY, 0);
	}
	else
	{
		Gwd_onu_poe_exist_stat_set(0);
		return;
	}
}


#endif

