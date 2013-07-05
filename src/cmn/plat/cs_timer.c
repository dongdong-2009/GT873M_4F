/***********************************************************************/
/* This file contains unpublished documentation and software                                            */
/* proprietary to Cortina Systems Incorporated. Any use or disclosure,                              */
/* in whole or in part, of the information in this file without a                                              */
/* written consent of an officer of Cortina Systems Incorporated is                                     */
/* strictly prohibited.                                                                                                             */
/* Copyright (c) 2002-2010 by Cortina Systems Incorporated.                                            */
/***********************************************************************/

#include "cs_timer.h"
#include "osal_common.h"
#include "plat_config.h"

cs_uint32 timer_thread_count = 0;
cs_uint32 sys_interval_in_tick = 1;
cs_timer_control_t cs_timers;
cs_uint32 timer_pool_id;
cs_int32 cs_timers_debug = 1;

static
cs_int32 cs_process_timer()
{
    cs_timer_t *pTimer = (cs_timer_t *)0;
    cs_timer_t *pExpireTimer = (cs_timer_t *)0;
    cs_uint64  begin_time = 0;
    cs_uint64  end_time = 0;
    cs_uint64  cb_begin_time=0;
    cs_uint64  cb_end_time=0;

    begin_time = cs_current_time();
    cs_mutex_lock(cs_timers.timer_mutex_id);
    cs_lst_init(&cs_timers.expire_timer_list , NULL);
    for (pTimer = (cs_timer_t *)cs_lst_first(&cs_timers.timer_list) ; pTimer ;) {
        if(pTimer->remain_interval < cs_timers.delta)
            pTimer->remain_interval = 0;
        else
            pTimer->remain_interval -= cs_timers.delta;

        if(pTimer->remain_interval > 0)
            pTimer->remain_interval -= sys_interval_in_tick;

        if (pTimer->remain_interval >= sys_interval_in_tick) {
            pTimer = (cs_timer_t *)cs_lst_next((cs_node *)pTimer);
            continue;
        }

        pExpireTimer = pTimer;
        pTimer = (cs_timer_t *)cs_lst_next((cs_node *)pTimer);
        
        cs_lst_delete(&cs_timers.timer_list , (cs_node *)pExpireTimer);
        cs_lst_add(&cs_timers.expire_timer_list , (cs_node *)pExpireTimer);
        cs_timers.total_timeout_timer++;
    }
    cs_mutex_unlock(cs_timers.timer_mutex_id);

    while((pExpireTimer = (cs_timer_t *)cs_lst_get(&cs_timers.expire_timer_list)) != NULL)
    {
        cb_begin_time = cs_current_time();
        if (pExpireTimer->callback) {
            pExpireTimer->callback(pExpireTimer);
        }
        cb_end_time = cs_current_time();
        if(cb_end_time - cb_begin_time > cs_timers.callback_peek_time)
        {
            cs_timers.callback_peek_time = cb_end_time - cb_begin_time;
            if(CS_ONCE_TIMER == pExpireTimer->timer_type)
            {
                cs_timers.callback_peek_ptr = (cs_uint32)pExpireTimer->callback;
            }
            else
            {
                // circle timer need to record real app's callback
                cs_timers.callback_peek_ptr = (cs_uint32)pExpireTimer->app_callback;
            }
        }
    }

    end_time = cs_current_time();
    cs_timers.delta = end_time - begin_time;
    cs_timers.total_timer_delta += cs_timers.delta;
    return CS_TIMER_OK;
}


/***********************************************************************
* NAME : cs_create_timer
* DESC :
*             Only allocate a memory block and initialize the memory according to these argu-
*             ments you provided . It is still not active unless you activate it. The callback function
*             will be involved when the timer is timeout. When you start the timer , perhaps the
*             timer will not run immediately.If the value of tiggle is zero,the timer will run immediately
*             when you start it.
*             For Example:
*                cs_create_timer(cb_func , 10 , 300)
*                if you start this timer , the timer will be activated after 10 ms, and will be timeout
*                after 310 ms . When the timer is timeout, cb_func is involved.
* ARGUMENT:
*             callback - pointer of callback function
*             triggle -
*             timeout -
* RETURN :
*             If create successfully , return the pointer of the timer , else return NULL
***********************************************************************/
static
cs_timer_t *cs_create_timer(
    cs_int32 (*callback)(cs_timer_t *pTimer) ,
    cs_int32 triggle ,
    cs_int32 timeout)
{
    cs_timer_t *pTimer = (cs_timer_t *)0;

    if (callback == NULL || triggle < 0 || timeout <= 0) {
        iros_timer_log("\r\n cs_create_timer failed , cause some parameter is null");
		diag_printf("\r\n cs_create_timer failed , cause some parameter is null");
        return (cs_timer_t *)pTimer;
    }

    pTimer = (cs_timer_t *)cs_mem_malloc(timer_pool_id);
    if (pTimer == (cs_timer_t *)0) {
        iros_timer_log("\r\n timer memory alloc failed");
		diag_printf("\r\n timer memory alloc failed");
        return (cs_timer_t *)pTimer;
    }

    memset((void *)pTimer , 0 , sizeof(cs_timer_t));
    pTimer->interval = pTimer->remain_interval  = (cs_int32)cs_milli_to_ticks(timeout);
    pTimer->callback = callback;
    pTimer->queue_id = CS_TIMER_NO_QUEUE;

    return (cs_timer_t *)pTimer;
}

static
cs_int32 cs_lookup_timer(cs_timer_t *pTimer)
{
    cs_timer_t *p = NULL;

    if(pTimer == NULL) {
        return CS_TIMER_ERROR;
    }

    cs_mutex_lock(cs_timers.timer_mutex_id);
    cs_lst_scan(&cs_timers.timer_list, p, cs_timer_t *)
    {
        if(p == pTimer) {
            cs_mutex_unlock(cs_timers.timer_mutex_id);
            return CS_TIMER_OK;
        }
    }
    cs_mutex_unlock(cs_timers.timer_mutex_id);

    return CS_TIMER_ERROR;
}


/***********************************************************************
* NAME : cs_count_timer
* DESC :
*            Get the count of timers in system timer-list
* ARGUMENT:
*             N/A
* RETURN :
*             count of timers
***********************************************************************/
cs_uint32 cs_count_timer()
{
    return cs_lst_count(&cs_timers.timer_list);
}

/***********************************************************************
* NAME : cs_destroy_timer
* DESC :
*            Destory a timer and free the memory. If the timer is active, you can not deotry it
*            successfully. If you have to destory it , you must stop if firstly.
* ARGUMENT:
*             pTimer - pointer of the timer you want to destroy
* RETURN :
*             If destroy it successfully , return CS_TIMER_OK , else return CS_TIMER_ERROR
***********************************************************************/
static
cs_int32 cs_destroy_timer(cs_timer_t *pTimer)
{
    if (pTimer == (cs_timer_t *)0) {
        iros_timer_log("\r\n You want to destroy a NULL timer?");
        return CS_TIMER_OK;
    }

    pTimer->stop_flag = CS_TIMER_DESTROYED;
    if(CS_TIMER_OK == cs_lookup_timer(pTimer)) {
        iros_timer_log("\r\n The timer is active , pls stop it firstly.");
        return CS_TIMER_OK;
    }

    cs_mem_free((cs_uint8 *)pTimer);

    return CS_TIMER_OK;
}

/***********************************************************************
* NAME : cs_start_timer
* DESC :
*            Activate the timer , add it to the system timer-list.
* ARGUMENT:
*             pTimer - pointer of the timer
* RETURN :
*             If start it successfully , return CS_TIMER_OK , else return CS_TIMER_ERROR
***********************************************************************/
static
cs_int32 cs_start_timer(cs_timer_t *pTimer)
{
    cs_timer_t *pNode = NULL;
    cs_int32 flag=0;

    if (pTimer == (cs_timer_t *)0) {
        iros_timer_log("\r\n start a NULL timer");
		diag_printf("\r\n start a NULL timer");
        return CS_TIMER_ERROR;
    }

    pTimer->stop_flag = CS_TIMER_NO_STOPPED;
    if(CS_TIMER_OK == cs_lookup_timer(pTimer)) {
        iros_timer_log("\r\n The timer is active already, do not need start");
		diag_printf("\r\n The timer is active already, do not need start");
        return CS_TIMER_OK;
    }

    cs_mutex_lock(cs_timers.timer_mutex_id);
    pTimer->remain_interval = pTimer->interval;
    cs_lst_scan(&cs_timers.timer_list, pNode, cs_timer_t *)
    {
        if(pNode->remain_interval <= pTimer->remain_interval)
            continue;

        cs_lst_insert(&cs_timers.timer_list, cs_lst_prev((cs_node *)pNode), (cs_node *)pTimer);
        flag = 1;
        break;
    }

    if(!flag)
        cs_lst_add(&cs_timers.timer_list , (cs_node *)pTimer);

    if(cs_timers.timer_peek_value <= cs_count_timer())
        cs_timers.timer_peek_value = cs_count_timer();
    cs_mutex_unlock(cs_timers.timer_mutex_id);
	//diag_printf("ok-----------------------------------------\n");
    return CS_TIMER_OK;
}

/***********************************************************************
* NAME : cs_stop_timer
* DESC :
*            Stop the timer , delete it from the system timer-list
* ARGUMENT:
*             pTimer - pointer of the timer
* RETURN :
*             If stop it successfully , return CS_TIMER_OK , else return CS_TIMER_ERROR
***********************************************************************/
static
cs_int32 cs_stop_timer(cs_timer_t *pTimer)
{
    if (pTimer == (cs_timer_t *)0) {
        iros_timer_log("\r\n stop a NULL timer");
        return CS_TIMER_ERROR;
    }

    pTimer->stop_flag = CS_TIMER_STOPPED;
    if(CS_TIMER_ERROR == cs_lookup_timer(pTimer)) {
        iros_timer_log("\r\n the timer is non-active");
        return CS_TIMER_OK;
    }

    cs_mutex_lock(cs_timers.timer_mutex_id);
    cs_lst_delete(&cs_timers.timer_list , (cs_node *)pTimer);
    cs_mutex_unlock(cs_timers.timer_mutex_id);

    return CS_TIMER_OK;
}

/***********************************************************************
* NAME : cs_restart_timer
* DESC :
*            Restart the timer. Firstly we stop it , then start it again.
* ARGUMENT:
*             pTimer - pointer of the timer
* RETURN :
*             If restart it successfully , return CS_TIMER_OK , else return CS_TIMER_ERROR
***********************************************************************/
static
cs_int32 cs_restart_timer(cs_timer_t *pTimer)
{
    if (pTimer == (cs_timer_t *)0) {
        iros_timer_log("\r\n restart a NULL timer");
        return CS_TIMER_ERROR;
    }

    cs_stop_timer(pTimer);
    cs_start_timer(pTimer);

    return CS_TIMER_OK;
}

static
cs_int32 cs_retiming_timer(cs_timer_t *pTimer , cs_uint32 timeout)
{
    if(pTimer == NULL) {
        iros_timer_log("\r\n retiming failed cause some NULL parameter");
        return CS_TIMER_ERROR;
    }

    if(CS_TIMER_ERROR == cs_lookup_timer(pTimer)) {
        iros_timer_log("\r\n the timer is non-active , don't need retiming");
        return CS_TIMER_OK;
    }

    cs_mutex_lock(cs_timers.timer_mutex_id);
    pTimer->remain_interval = pTimer->interval = (cs_int32)cs_milli_to_ticks(timeout);
    cs_mutex_unlock(cs_timers.timer_mutex_id);

    return CS_TIMER_OK;
}

/* public interfaces */
void cs_timer_thread()
{
    while (1) {
        timer_thread_count++;
        cs_process_timer();
        cs_thread_delay(sys_interval_in_tick * 10); // delay in mili sec
    }
    return;
}

cs_int32 cs_timer_init()
{
    cs_uint32 ret;

    memset((void *)&cs_timers , 0 , sizeof(cs_timer_control_t));
    cs_lst_init(&cs_timers.timer_list , NULL);

    if(CS_E_OSAL_OK != cs_mempool_create(&timer_pool_id, "Timer-pool" , sizeof(cs_timer_t) , CS_MAX_TIMER_NUMBER))
    {
        iros_timer_log("\r\n create timer pool failed");
        return CS_TIMER_ERROR;
    }

    ret = cs_mutex_init(&cs_timers.timer_mutex_id, "timer_thread_mutex", 0);
    if (ret != CS_E_OSAL_OK) {
        iros_timer_log("\r\n cs_timer mutex create failed(%d)",ret);
        cs_mempool_destroy(timer_pool_id);
        return CS_TIMER_ERROR;
    }

    ret = cs_thread_create(&cs_timers.timer_thread_id,
                             TIMER_THREAD_NAME,
                             (const void *)cs_timer_thread, (void *)0,
                             TIMER_THREAD_STACK_SIZE,
                             TIMER_THREAD_PRIORITY ,
                             0);

    if (ret != CS_E_OSAL_OK) {
        iros_timer_log("\r\n Iros-timer thread creat failed.");
        cs_mutex_destroy(cs_timers.timer_mutex_id);
        cs_mempool_destroy(timer_pool_id);
        return CS_TIMER_ERROR;
    }

    return CS_TIMER_OK;
}



void cs_timer_show()
{
    cs_timer_t *pTimer = NULL;
    cs_int8 *circle_str = "circle";
    cs_int8 *once_str = "once";
    cs_int8 *yes_str = "yes";
    cs_int8 *no_str = "no";

    // cs_printf("\r\n========================================================================");
    cs_printf("\r\n%-10s   %-8s %-8s %-8s %-9s %-11s %-8s","Timer-ID","Remain","Triggle","O-Remain",
                                        "O-Triggle","Circle-Type","Msg-Type");
    //cs_printf("\r\n------------------------------------------------------------------------");
    cs_mutex_lock(cs_timers.timer_mutex_id);
    cs_lst_scan(&cs_timers.timer_list, pTimer, cs_timer_t *)
    {
        cs_printf("\r\n0x%08x   %-8d %-8d %-8d %-9d %-11s %-8s",(cs_uint32)pTimer,pTimer->remain_interval,
            0,pTimer->interval,0,
            (pTimer->timer_type == CS_CIRCLE_TIMER)?circle_str:once_str,
            (pTimer->queue_id != CS_TIMER_NO_QUEUE)?yes_str:no_str);
    }
    cs_mutex_unlock(cs_timers.timer_mutex_id);
    //cs_printf("\r\n========================================================================");
    cs_printf("\r\n Total Timer : %d", cs_count_timer());
    cs_printf("\r\n Total Delta : %d Ticks",cs_timers.total_timer_delta);
    cs_printf("\r\n Timer Peek  : %d Timers",cs_timers.timer_peek_value);
    cs_printf("\r\n Total Timeout Timer: %d ",cs_timers.total_timeout_timer);
    cs_printf("\r\n Callback Info      : %d ticks  cb=0x%08x",cs_timers.callback_peek_time,cs_timers.callback_peek_ptr);
    cs_printf("\r\n Msg Timer Error    : %d",cs_timers.msg_timer_sendQ_error);
    cs_printf("\r\n");
    return;
}

static void cs_timer_app_callback(cs_timer_t *pTimer)
{
    if(pTimer->timer_type == CS_CIRCLE_TIMER)
    {
        if(pTimer->stop_flag == CS_TIMER_DESTROYED)
        {
            cs_mem_free((cs_uint8 *)pTimer);
            pTimer = NULL;
            return;
        }

        if(pTimer->stop_flag == CS_TIMER_STOPPED)
        {
            return;
        }
    }

    pTimer->stop_flag = CS_TIMER_NO_STOPPED;
    if(pTimer->app_callback) {
        pTimer->app_callback((void *)pTimer->data);
    }

    if(pTimer->timer_type == CS_ONCE_TIMER) {
        cs_destroy_timer(pTimer);
        pTimer = NULL;
    }
    else
    {
        cs_start_timer(pTimer);
    }

    return;
}


cs_int32 cs_timer_add(
                    cs_uint32 timeout ,
                    void (*callback)(void *) ,
                    void *data
                    )
{
    cs_timer_t *pTimer = (cs_timer_t *)NULL;

    if(callback == NULL) {
        iros_timer_log("\r\n create timer with null callback function");
		diag_printf("\r\n create timer with null callback function");
        return CS_INVALID_TIMER;
    }

    pTimer = (cs_timer_t *)cs_create_timer((void *)cs_timer_app_callback, 0 , timeout);
    if(pTimer == NULL) {
        iros_timer_log("\r\n Create timer failed");
		diag_printf("\r\n Create timer failed");
        return CS_INVALID_TIMER;
    }
	//diag_printf("oooooooooooooooooooooooooooooooooooooo\n");
    pTimer->queue_id = CS_TIMER_NO_QUEUE;
    pTimer->app_callback = callback;
    pTimer->data = data;
    pTimer->timer_type = CS_ONCE_TIMER;

    cs_start_timer(pTimer);

    return (cs_uint32)pTimer;
}

cs_int32 cs_circle_timer_add(cs_uint32 timeout, void (*callback)(void *), void *data)
{
    cs_timer_t *pTimer = (cs_timer_t *)NULL;

    if(callback == NULL) {
        iros_timer_log("\r\n create timer with null callback function");
        return CS_INVALID_TIMER;
    }

    pTimer = (cs_timer_t *)cs_create_timer((void *)cs_timer_app_callback, 0 , timeout);
    if(pTimer == NULL) {
        iros_timer_log("\r\n Create circle timer faileds");
        return CS_INVALID_TIMER;
    }

    pTimer->queue_id = CS_TIMER_NO_QUEUE;
    pTimer->app_callback = callback;
    pTimer->data = data;
    pTimer->timer_type = CS_CIRCLE_TIMER;

    cs_start_timer(pTimer);

    return (cs_uint32)pTimer;
}


cs_int32 cs_timer_del(cs_uint32 timer_handle)
{
    cs_timer_t *pTimer = (cs_timer_t *)timer_handle;

    if(pTimer == NULL) {
        iros_timer_log("\r\n Delete a NULL timer ?");
        return CS_TIMER_ERROR;
    }

    cs_stop_timer(pTimer);
    cs_destroy_timer(pTimer);

    return CS_TIMER_OK;
}

cs_int32 cs_timer_stop(cs_uint32 timer_handle)
{
    cs_timer_t *pTimer = (cs_timer_t *)timer_handle;

    if(pTimer == NULL) {
        iros_timer_log("\r\n stop a NULL timer");
        return CS_TIMER_ERROR;
    }

    cs_stop_timer(pTimer);

    return CS_TIMER_OK;
}

cs_int32 cs_timer_start(cs_uint32 timer_handle)
{
    cs_timer_t *pTimer = (cs_timer_t *)timer_handle;

    if(pTimer == NULL) {
        iros_timer_log("\r\n start a NULL timer.");
        return CS_TIMER_ERROR;
    }

    cs_start_timer(pTimer);

    return CS_TIMER_OK;
}

cs_int32 cs_timer_restart(cs_uint32 timer_handle)
{
    cs_timer_t *pTimer = (cs_timer_t *)timer_handle;

    if(pTimer == NULL) {
        iros_timer_log("\r\n restart a NULL timer");
        return CS_TIMER_ERROR;
    }

    cs_restart_timer(pTimer);

    return CS_TIMER_OK;
}

cs_int32 cs_timer_retiming(cs_uint32 timer_handle , cs_uint32 timeout)
{
    return (cs_int32)cs_retiming_timer((cs_timer_t *)timer_handle, timeout);
}


#ifdef CS_TIMERS_EX_MSG

static void cs_msg_timer_app_callback(cs_timer_t *pTimer)
{
    cs_int32 queue_type = CS_NORMAL_QUEUE_TYPE;
    cs_int32 ret = CS_E_OSAL_ERR;
    cs_int32 max_pri=0;

    if(pTimer == NULL)
    {
        return ;
    }

    queue_type = cs_queue_type_get(pTimer->queue_id);
    if(queue_type == CS_NORMAL_QUEUE_TYPE)
        ret = cs_queue_put(pTimer->queue_id, (void *)&pTimer->data, sizeof(cs_uint32), CS_OSAL_NO_WAIT , 0);
    else {
        max_pri = cs_pri_queue_max_priority(pTimer->queue_id);
        ret = cs_pri_queue_put(pTimer->queue_id, (void *)&pTimer->data, sizeof(cs_uint32), CS_OSAL_NO_WAIT , max_pri);
    }

    if(CS_E_OSAL_OK != ret)
    {
        if(pTimer->timer_type == CS_ONCE_TIMER) {
            cs_destroy_timer(pTimer);
            pTimer = NULL;
        }
        else {
            if(pTimer->data_free_func) {
            pTimer->data_free_func((void *)pTimer->data);
            }
            cs_start_timer(pTimer);
        }
           
        cs_timers.msg_timer_sendQ_error++;
        return ;
    }

    if(pTimer->timer_type == CS_ONCE_TIMER) {
        cs_destroy_timer(pTimer);
        pTimer = NULL;
    }
    else
    {
        cs_start_timer(pTimer);
    }

    return;
}


cs_int32 cs_msg_timer_add(
                        cs_uint32 queue_id ,
                        cs_int32 timeout ,
                        void *data,
                        void (*data_free_func)(void *)
                        )
{
    cs_timer_t *pTimer = (cs_timer_t *)NULL;

    pTimer = (cs_timer_t *)cs_create_timer((void *)cs_msg_timer_app_callback, 0 , timeout);
    if(pTimer == NULL) {
        iros_timer_log("\r\n create msg timer failed");
        return CS_INVALID_TIMER;
    }

    pTimer->queue_id = queue_id;
    pTimer->data = data;
    pTimer->data_free_func = data_free_func;
    pTimer->timer_type = CS_ONCE_TIMER;

    cs_start_timer(pTimer);

    return (cs_uint32)pTimer;
}

cs_int32 cs_msg_circle_timer_add(
                        cs_uint32 queue_id ,
                        cs_int32 timeout ,
                        void *data
                        )
{
    cs_timer_t *pTimer = (cs_timer_t *)NULL;

    pTimer = (cs_timer_t *)cs_create_timer((void *)cs_msg_timer_app_callback, 0 , timeout);
    if(pTimer == NULL) {
        iros_timer_log("\r\n Create msg circle timer failed");
        return CS_INVALID_TIMER;
    }

    pTimer->queue_id = queue_id;
    pTimer->data = data;
    pTimer->data_free_func = NULL;
    pTimer->timer_type = CS_CIRCLE_TIMER;

    cs_start_timer(pTimer);

    return (cs_uint32)pTimer;
}

#endif


#define CS_TIMER_DEBUG
#ifdef CS_TIMER_DEBUG

void tm_cb(void* data)
{
    cs_printf("tm %d %lld\n", *((cs_uint8*)data), cs_current_time());
}

cs_uint8 tm_data_1 = 1;
cs_uint8 tm_data_2 = 2;


void timer_test()
{
    cs_circle_timer_add(2000, tm_cb, &tm_data_1); // 2 sec
    cs_timer_add(120*1000, tm_cb, &tm_data_2); // 2 min
    cs_printf("timer created\n");
}

#endif
