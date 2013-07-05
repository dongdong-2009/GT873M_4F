#include "cs_types.h"
#include "plat_common.h"
#include "sdl_gpio.h"
#include "app_gpio.h"
#include "oam_api.h"

cs_uint64 dgasp_prev_time = 0;
cs_uint8 dgasp_cnt = 0;
cs_uint8 dying_gasp_cnt = 0;
static void cs_gpio_dying_gasp(cs_int32 gpio_pin, cs_uint8 status)
{ 
    cs_uint64 cur_time = 0;

    cur_time = cs_current_time();
    if(cur_time - 100 >= dgasp_prev_time)
    {
        dgasp_cnt = 0;
    }
    else
    {
        dgasp_cnt++;
    }
    dgasp_prev_time = cur_time;
    if(dgasp_cnt > dying_gasp_cnt)
    {
        return;
    }
    oam_send_dying_gasp(0);
    return;
}


void cs_gpio_org_init(void)
{   
    cs_app_gpio_write(RESET_TYPE, 0);
    hal_delay_us(30000);
    cs_app_gpio_write(RESET_TYPE,1);
    
    cs_gpio_trigger_reg(DYING_GASP, cs_gpio_dying_gasp);
}
