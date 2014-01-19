#include <Copyright.h>

#include "msSample.h"

#include "cs_types.h"
#include "cs_cmn.h"
#include "sdl_mdio.h"

/*****************************************************************************
*
* bool etherReadMIIReg (unsigned int portNumber , unsigned int MIIReg,
* unsigned int* value)
*
* Description
* This function will access the MII registers and will read the value of
* the MII register , and will retrieve the value in the pointer.
* Inputs
* portNumber - one of the 2 possiable Ethernet ports (0-1).
* MIIReg - the MII register offset.
* Outputs
* value - pointer to unsigned int which will receive the value.
* Returns Value
* true if success.
* false if fail to make the assignment.
* Error types (and exceptions if exist)
*/

GT_BOOL gtBspReadMii (GT_QD_DEV* dev, unsigned int portNumber , unsigned int MIIReg,
                        unsigned int* value)
{

	cs_callback_context_t cc;
	cs_uint16 v;

	if(cs_plat_mdio_read(cc, 0, 0, portNumber, MIIReg, &v) == CS_E_OK)
	{
		*value = v;
		return GT_TRUE;
	}
	else
		return GT_FALSE;
}

/*****************************************************************************
* 
* bool etherWriteMIIReg (unsigned int portNumber , unsigned int MIIReg,
* unsigned int value)
* 
* Description
* This function will access the MII registers and will write the value
* to the MII register.
* Inputs
* portNumber - one of the 2 possiable Ethernet ports (0-1).
* MIIReg - the MII register offset.
* value -the value that will be written.
* Outputs
* Returns Value
* true if success.
* false if fail to make the assignment.
* Error types (and exceptions if exist)
*/

GT_BOOL gtBspWriteMii (GT_QD_DEV* dev, unsigned int portNumber , unsigned int MIIReg,
                       unsigned int value)
{
	cs_callback_context_t cc;

	if(cs_plat_mdio_write(cc, 0, 0, portNumber, MIIReg, value) == CS_E_OK)
	{
		return GT_TRUE;
	}
	else
		return GT_FALSE;
}


void gtBspMiiInit(GT_QD_DEV* dev)
{
	return;	
}
