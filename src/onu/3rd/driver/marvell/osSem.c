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


#include <msApi.h>

#include "cs_cmn.h"
#include "osal_api_core.h"

GT_SEM osSemCreate( GT_SEM_BEGIN_STATE state);
GT_STATUS osSemDelete(GT_SEM smid);
GT_STATUS osSemWait(  GT_SEM smid, GT_U32 timeOut);
GT_STATUS osSemSignal(GT_SEM smid);

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
	if(cs_semaphore_init((unsigned int*)&osSemId, "swSemId", state, 0) == CS_E_OK)
		return osSemId;
	else
		return GT_FAIL;
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
	if(cs_semaphore_wait(smid, timeOut) != CS_E_OK)
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
