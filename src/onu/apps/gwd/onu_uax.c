#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> 

#include "onu_specs.h"
#include "cli_common.h"
#include "onu_syscfg.h"
#include "onu_datetype.h"
#include "onu_uax.h"

unsigned long g_ulMAXPortOnSlotNum              = MAXPortOnSlotNum;

#define STATE_S 1
#define STATE_PS 2
#define STATE_PE 3
struct slot_port * BEGIN_PARSE_PORT_EAND_SLOT(char * argv, struct slot_port* my_onu,char *ifname,struct cli_def* cli_i )
{
	struct slot_port *p;
    unsigned long ulState = STATE_S;
    unsigned long ulSlot;
	unsigned long ulPort;
    char digit_temp[ 12 ];
    char cToken;
    unsigned long list_i = 0;
    unsigned long temp_i = 0;
    unsigned long ulListLen = 0;
    char * list;

	p = my_onu;
    ulListLen = strlen(argv);
    list = (char *)iros_malloc(IROS_MID_APP,ulListLen + 2);
    if ( list == NULL )
    {
        return NULL;
    }
    strncpy( list,argv, ulListLen + 1 );
    list[ ulListLen ] = ',';
    list[ ulListLen + 1 ] = '\0';

    cToken = list[ list_i ];

	
	while ( cToken != 0 )
    {
 
        switch ( ulState )
        {
        	
            case STATE_S:
                if ( isdigit( cToken ) )
                {
        
                    digit_temp[ temp_i ] = cToken;
                    temp_i++;
                    if ( temp_i >= 11 )
                    {
                        goto error;
                    }
                }
                else if ( isspace( cToken ) )
                {}
	/********************************************************************************************************
	判断板级号，检查板级的有效性。
	*********************************************************************************************************/
                else if ( cToken == '/' )
                {
 
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    ulSlot = ( unsigned long ) atol( digit_temp );

					 if ( ulSlot >= MAXSlotNum )                   	
	                    {
	                        goto error;
	                    }
					p->ulSlot = ulSlot;
                    temp_i = 0;
                    ulState = STATE_PS;
                }

/***************************************************************************************************************
判断板级上的PORT 的号，并检查PORT 的有效性，通过板级的号来判断板级的类型，
并且创建索引。
****************************************************************************************************************/
                else if ( cToken == ',' )
                {
      
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    ulPort = ( unsigned long ) atol( digit_temp );
	
	                if( FALSE==SysIsValidPortOnSlot( ulPort))
	                   {
	                       goto error;
	                   }

					p->ulPort = ulPort;
                    temp_i = 0;
                    ulState = STATE_S;
                }
                else if ( cToken == '-' )
                {
   
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    ulPort = ( ULONG ) atol( digit_temp );
					p->ulPort = ulPort;
                    temp_i = 0;
                    ulState = STATE_PE;
                }

                else
                {
                    goto error;
                }
                break;
            case STATE_PS:
                if ( isdigit( cToken ) )
                {

                    digit_temp[ temp_i ] = cToken;
                    temp_i++;
                    if ( temp_i >= 11 )
                    {
                        goto error;
                    }
                }
                else if ( isspace( cToken ) )
                {}
                else if ( cToken == ',' )
                {

                    if ( temp_i == 0 )
                    {	
   
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    ulPort = ( ULONG ) atol( digit_temp );	
	                if ( ulPort >= MAXPortOnSlotNum )
	                    {
	                        goto error;
	                    }
					p->ulPort = ulPort;
                    temp_i = 0;
                    ulState = STATE_S;
	
                }
                else if ( cToken == '-' )
                {
   
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    ulPort = ( ULONG ) atol( digit_temp );
					p->ulPort = ulPort;
                    temp_i = 0;
                    ulState = STATE_PE;
                }
                else
                {
                    goto error;
                }
                break;
            case STATE_PE:
                if ( isdigit( cToken ) )
                {

                    digit_temp[ temp_i ] = cToken;
                    temp_i++;
                    if ( temp_i >= 11 )
                    {
                        goto error;
                    }
                }
                else if ( isspace( cToken ) )
                {}
                else if ( cToken == ',' )
                {

                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    ulPort= ( ULONG ) atol( digit_temp );
					p->ulPort = ulPort;
                    temp_i = 0;
                    ulState = STATE_S;
                }
                else
                {
                    goto error;
                }
                break;
            default:
                goto error;
        }

        list_i++;
        cToken = list[ list_i ];

    }
	iros_free(list);
	return p;
error:
	iros_free(list);
	cli_print( cli_i, "%% Can not find interface %s.\r\n", ifname );
	return NULL;

       
 }
unsigned long BEGIN_PARSE_TO_PORT(char *argv)
{
	unsigned long ulPort;
	unsigned long temp_i = 0;
    char digit_temp[ 12 ];
    char cToken;
    unsigned long list_i = 0;
	unsigned long ulListLen = 0;
    char * list;
    ulListLen = strlen(argv);
    list = iros_malloc(IROS_MID_APP,ulListLen + 2);
    if ( list == NULL )
    {
        return -1;
    }
    strncpy( list,argv, ulListLen + 1 );
    list[ ulListLen ] = ',';
    list[ ulListLen + 1 ] = '\0';

    cToken = list[ list_i ];

	while ( cToken != '\0' )
		{
			while(isspace( cToken ))
				{	
					list_i++;
       				cToken = list[ list_i ];
					
				}
			if ( isdigit( cToken ) )
                {
                    digit_temp[ temp_i ] = cToken;
                    temp_i++;
                }

			list_i++;
       		cToken = list[ list_i ];
				
		}
	ulPort = ( unsigned long ) atol( digit_temp );
	diag_printf("*^*^*^*^*^*^*^*^ CTOKEN = %c\n",cToken);
	iros_free(list);
	return ulPort;
	
}
#if 1
unsigned long * ETH_ParsePortList(char * argv,unsigned long onu_roter_port_num)
{
	unsigned long roter_port[50];
    unsigned long ulState = STATE_S;
	unsigned long ulPort;
	unsigned long ulPort_E;
	unsigned long ul_i;
	unsigned long ul_e;
	int i = 0;
    char digit_temp[ 12 ];
    char cToken;
    unsigned long list_i = 0;
	int port_i = 0;
    unsigned long temp_i = 0;
    unsigned long ulListLen = 0;
   	char *list;
	unsigned long *rt_roter_port;
    ulListLen = strlen(argv);
    list = (char *)iros_malloc(IROS_MID_APP,ulListLen + 2);
    if ( list == NULL )
    {
        return NULL;
    }
    strncpy( list,argv, ulListLen + 1 );
    list[ ulListLen ] = ',';
    list[ ulListLen + 1 ] = '\0';

    cToken = list[ list_i ];
	bzero(roter_port,sizeof(roter_port));
	
	while ( cToken != 0 )
    {
        switch ( ulState )
        {
        	
            case STATE_S:
                if ( isdigit( cToken ) )
                {
                    digit_temp[ temp_i ] = cToken;
                    temp_i++;
                    if ( temp_i >= 11 )
                    {
                        goto error;
                    }
                }
                else if ( isspace( cToken ) )
                {}

                else if ( cToken == ',' )
                {
                	i = 0;     				
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    ulPort = ( unsigned long ) atol( digit_temp );					
					while((roter_port[i] !=ulPort) && (i<20))
						{
							
							if(i == 19)
								{									
									roter_port[port_i] = ulPort;
									port_i++;
									if(ulPort > onu_roter_port_num)
										{	
											diag_printf("Port %ld input error\n",ulPort);
											diag_printf("the max port is %ld,so you are wrong\n",onu_roter_port_num);
											goto error;
										}
								}
							i++;
						}
                    temp_i = 0;
                    ulState = STATE_S;
                }
                else if ( cToken == '-')
                {
                	i = 0;   					
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    ulPort = ( ULONG ) atol( digit_temp );
					while((roter_port[i] !=ulPort) && (i<20))
						{
							
							if(i == 19)
								{									
									roter_port[port_i] = ulPort;
									port_i++;
									if(ulPort > onu_roter_port_num)
										{	
											diag_printf("Port %ld input error\n",ulPort);
											diag_printf("the max port is %ld,so you are wrong\n",onu_roter_port_num);
											goto error;
										}
								}
							i++;
						}
                    temp_i = 0;
                    ulState = STATE_PE;
                }
                else
                {
                    goto error;
                }
                break;
            case STATE_PS:
                if ( isdigit( cToken ) )
                {

                    digit_temp[ temp_i ] = cToken;
                    temp_i++;
                    if ( temp_i >= 11 )
                    {
                        goto error;
                    }
                }
                else if ( isspace( cToken ) )
                {}
                else if ( cToken == ',' )
                {
					i = 0;
                    if ( temp_i == 0 )
                    {	   						
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    ulPort = ( ULONG ) atol( digit_temp );	
	                if ( ulPort >= 20 )
	                    {
	                        goto error;
	                    }
					while((roter_port[i] !=ulPort) && (i<20))
						{
							
							if(i == 19)
								{									
									roter_port[port_i] = ulPort;
									port_i++;
									if(ulPort > onu_roter_port_num)
										{																				
											goto error;
										}
								}
							i++;
						}
                    temp_i = 0;
                    ulState = STATE_S;
	
                }
                else if ( cToken == '-' )
                {
   					i = 0;
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    ulPort = ( ULONG ) atol( digit_temp );

					while((roter_port[i] !=ulPort) && (i<20))
						{
							
							if(i == 19)
								{									
									roter_port[port_i] = ulPort;
									port_i++;
									if(ulPort> onu_roter_port_num)
										{																					
											goto error;
										}
								}
							i++;
						}
                    temp_i = 0;
                    ulState = STATE_PE;
                }
                else
                {
                    goto error;
                }
                break;
            case STATE_PE:
                if ( isdigit( cToken ) )
                {					
                    digit_temp[ temp_i ] = cToken;
                    temp_i++;
                    if ( temp_i >= 11 )
                    {
                        goto error;
                    }
                }
                else if ( isspace( cToken ) )
                {}
                else if ( cToken == ',' )
                {
                	i = 0;					
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    ulPort_E = ( ULONG ) atol( digit_temp );
					ul_i = ulPort;
					ul_e = ulPort_E;
					if(ul_i < ul_e)
						{
							ul_i = ulPort;
							ul_e = ulPort_E;
						}
					else
						{
							ul_i = ulPort_E;
							ul_e = ulPort;
						}
	 			
				    while(ul_i <= ul_e )
					{						
						while((roter_port[i] != ul_i) && (i < 20))
						{							
							if(i == 19)
								{								
									roter_port[port_i] = ul_i;
									port_i++;
									if(ul_i > onu_roter_port_num)
										{
											diag_printf("the port %ld  input error \n",ul_i);
											diag_printf("the max port is %ld,so you are wrong\n",onu_roter_port_num);
											goto error;
										}
								}
							i++;
						}																						
						ul_i++;						
						i = 0;
					}								                 	
                    temp_i = 0;
                    ulState = STATE_S;
                }
                else
                {
                    goto error;
                }
                break;
            default:				
                goto error;
        }

        list_i++;
        cToken = list[ list_i ];		

    }
		
	iros_free(list);
	rt_roter_port = (unsigned long *)iros_malloc(IROS_MID_APP,(port_i+1)* sizeof(unsigned long));
	if(rt_roter_port == NULL)
		{
			diag_printf("rt_roter_port malloc null error\n");
			return NULL;
		}
	memcpy(rt_roter_port,roter_port,(port_i+1)*sizeof(unsigned long));
	return rt_roter_port;
error:
	iros_free(list);
	return NULL;

       
	
}

long GetMacAddr( char * szStr, char * pucMacAddr )
{
    CHAR * p = NULL, *q = NULL, *pcTmp = NULL;
    CHAR cTmp[ 3 ];
    ULONG i = 0, j = 0, k = 0;

    /* 检查字符串长度是否正常 */
    if ( 14 != strlen( szStr ) )
    {
        return ECOS_ERROR;
    }

    p = szStr;
    for ( i = 0; i < 3; i++ )
    {
        if ( i != 2 )
        {
            /* 查看有无'.' */
            q = strchr( p, '.' );
            if ( NULL == p )
            {
                return ECOS_ERROR;
            }
        }
        else
        {
            q = szStr + strlen( szStr );
        }

        /* 一个H不是4个字符 */
        if ( 4 != q - p )
        {
            return ECOS_ERROR;
        }
        /* 检查是否是16进制的数字 */
        for ( j = 0; j < 4; j++ )
        {
            if ( !( ( *( p + j ) >= '0' && *( p + j ) <= '9' ) || ( *( p + j ) >= 'a' && *( p + j ) <= 'f' )
                    || ( *( p + j ) >= 'A' && *( p + j ) <= 'F' ) ) )
            {
                return ECOS_ERROR;
            }
        }

        cTmp[ 0 ] = *p;
        cTmp[ 1 ] = *( p + 1 );
        cTmp[ 2 ] = '\0';
        pucMacAddr[ k ] = ( CHAR ) strtoul( cTmp, &pcTmp, 16 );
        k++;

        cTmp[ 0 ] = *( p + 2 );
        cTmp[ 1 ] = *( p + 3 );
        cTmp[ 2 ] = '\0';
        pucMacAddr[ k ] = ( CHAR ) strtoul( cTmp, &pcTmp, 16 );
        k++;

        p = q + 1;
    }

    /* 判断是否全部为0 */
    if ( 0x0 == pucMacAddr[ 0 ] && 0x0 == pucMacAddr[ 1 ] && 0x0 == pucMacAddr[ 2 ]
            && 0x0 == pucMacAddr[ 3 ] && 0x0 == pucMacAddr[ 4 ] && 0x0 == pucMacAddr[ 5 ] )
    {
        return ECOS_ERROR;
    }

    /* 判断是否全部为ff */
    if ( 0xff == pucMacAddr[ 0 ] && 0xff == pucMacAddr[ 1 ] && 0xff == pucMacAddr[ 2 ]
            && 0xff == pucMacAddr[ 3 ] && 0xff == pucMacAddr[ 4 ] && 0xff == pucMacAddr[ 5 ] )
    {
        return ECOS_ERROR;
    }

    /* 判断是否为多播 */
    if ( 0 != ( pucMacAddr[ 0 ] & 0x01 ) )
    {
        return ECOS_ERROR;
    }

    return ECOS_OK;

}

#endif


