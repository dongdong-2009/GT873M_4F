#include <Copyright.h>
/********************************************************************************
* osSem.c
*
* DESCRIPTION:
*       Semaphore related routines
*
* DEPENDENCIES:
*       OS Dependent.
*
* FILE REVISION NUMBER:
*
*******************************************************************************/

#include <stdio.h>
#include <string.h>

#include <msApi.h>

#include "cs_cmn.h"
#include "osal_api_core.h"
#include "gtSem.h"

GT_SEM osSemCreate( GT_SEM_BEGIN_STATE state);
GT_STATUS osSemDelete(GT_SEM smid);
GT_STATUS osSemWait(  GT_SEM smid, GT_U32 timeOut);
GT_STATUS osSemSignal(GT_SEM smid);

typedef struct{
	GT_BOOL set;
	GT_SEM  smid;
}gt_sem_e_t;

static gt_sem_e_t gt_sem_es[CS_OSAL_MAX_COUNT_SEM];
static GT_BOOL gt_sem_init = GT_FALSE;

static GT_U32  getFreeNameBit()
{
	GT_U32 i;

	for(i=0; i<CS_OSAL_MAX_COUNT_SEM; i++)
	{
		if(gt_sem_es[i].set == GT_FALSE)
			return i;
	}

	return i;
}

static void  setNameBit(GT_U32 pos, GT_BOOL en, GT_SEM id)
{
	if(pos < CS_OSAL_MAX_COUNT_SEM)
	{
		gt_sem_es[pos].set = en;
		gt_sem_es[pos].smid = id;
	}
}

static void clrNameBit(GT_SEM id)
{
	GT_U32 i;

	for(i=0; i<CS_OSAL_MAX_COUNT_SEM; i++)
	{
		if(gt_sem_es[i].smid == id)
		{
			gt_sem_es[i].set = GT_FALSE;
			break;
		}
	}
}

/*******************************************************************************
* osSemCreate
*
* DESCRIPTION:
*       Create semaphore.
*
* INPUTS:
*       name   - semaphore Name
*       init   - init value of semaphore counter
*       count  - max counter (must be >= 1)
*
* OUTPUTS:
*       smid - semaphore Id
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_SEM osSemCreate(GT_SEM_BEGIN_STATE state)
{
	GT_SEM osSemId;
	GT_U32 namebit = 0;

	if(gt_sem_init)
	{
		memset(gt_sem_es, 0, sizeof(gt_sem_e_t)*CS_OSAL_MAX_COUNT_SEM);
		gt_sem_init = GT_TRUE;
	}

	if((namebit = getFreeNameBit()) != CS_OSAL_MAX_COUNT_SEM)
	{
		char name[CS_OSAL_MAX_API_NAME] = "";

		sprintf(name, "swSemid%lu", namebit);

		if(cs_semaphore_init((unsigned int*)&osSemId, name, state, 0) == CS_E_OK)
		{
//			cs_printf("%s (%lu)\n", __func__, osSemId);
			setNameBit(namebit, GT_TRUE, osSemId);
			return osSemId;
		}
		else
		{
			cs_printf("%s failed!\n", __func__);
			return GT_ERROR;
		}
	}

	return GT_ERROR;
}

/*******************************************************************************
* osSemDelete
*
* DESCRIPTION:
*       Delete semaphore.
*
* INPUTS:
*       smid - semaphore Id
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS osSemDelete(GT_SEM smid)
{
	clrNameBit(smid);
	if(cs_semaphore_destroy(smid) != CS_E_OK)
		return GT_ERROR;
	return GT_OK;
}

/*******************************************************************************
* osSemWait
*
* DESCRIPTION:
*       Wait on semaphore.
*
* INPUTS:
*       smid    - semaphore Id
*       timeOut - time out in miliseconds or 0 to wait forever
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*       OS_TIMEOUT - on time out
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS osSemWait(GT_SEM smid, GT_U32 timeOut)
{

	cs_int32 cs_timeout = (timeOut == OS_WAIT_FOREVER)?CS_OSAL_WAIT_FOREVER:timeOut;

	if(cs_semaphore_wait(smid, cs_timeout) != CS_E_OK)
		return GT_ERROR;
	return GT_OK;
}

/*******************************************************************************
* osSemSignal
*
* DESCRIPTION:
*       Signal a semaphore.
*
* INPUTS:
*       smid    - semaphore Id
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS osSemSignal(GT_SEM smid)
{
	if(cs_semaphore_post(smid) != CS_E_OK)
		return GT_ERROR;
	return GT_OK;
}
