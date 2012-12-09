//============================================================================
//  File Name: cmd.c 
//
//  2012 Copyright (c), NXP Semiconductors. All rights reserved.
//
//  This source code is NXP Semiconductors N.V. proprietary and confidential
//  information
//
//  Description:
//      User command scan    
//
//  Modification History:
//      Revision 1.0  2012/02/06 15:38:44  xiaohua.wang@nxp.com
//      Initial draft
//
//      Revision 1.1  2012/02/06 20:51:22  xiaohua.wang@nxp.com
//      Verified on Micosoft VS 2008
//      
//      Revision 1.2  2012/02/09 15:19:02  xiaohua.wang@nxp.com
//      Verified on Google Nexus S
//
//      Revision 1.3  2012/02/17 13:07:38  xiaohua.wang@nxp.com
//      Add PBRS test command
//
//      Revision 1.4  2012/03/15 11:31:09  xiaohua.wang@nxp.com
//      Add Polling Loop command
//
//      Revision 1.5  2012/03/21 16:08:37  xiaohua.wang@nxp.com
//      Add supported for Google Galaxy Nexus
//
//      Revision 1.6  2012/12/09 19:12:21  xiaohua.wang@nxp.com
//      Display Application processor CPU core information
//
//=============================================================================
  

#include <pnx.h>
#include <getopt.h> 

int Verbose = FALSE;

#define Address 2
#define Value   3

#define ROW             0x10
#define EEPROM_BOTTOM   0x0097FFUL
#define EEPROM_TOP      0x009FFFUL
#define RAM_BOTTOM      0x00F7FFUL
#define RAM_TOP         0x00F8FFUL

typedef enum Pnx_Cmd_Opt
{
    PNX_OPT = 255,// Placeholder to set start value
    PNX_OPT_HEL,// PNx Command Help
    PNX_OPT_PCV,// PNx Chip Version(HW,FW version)
    PNX_OPT_AST,// Antenna Self Test
    PNX_OPT_SST,// SWP Self Test
    PNX_OPT_SWP,// SWP On/Off Test
    PNX_OPT_SMX,// SMX On/Off Test
    PNX_OPT_SSD,// SMX SWP dynamically Switch
    PNX_OPT_CHR,// Chip Soft Reset
    PNX_OPT_RRV,// Read the Register Vaule
    PNX_OPT_WRV,// Write Register Value
    PNX_OPT_ARV,// All Register Values
    PNX_OPT_CUI,// Card Unique Identifier
    PNX_OPT_CMM,// Configurable Memory Map
    PNX_OPT_PPL,// PNx Poll Loop
    PNX_OPT_PRB,// PRBS test Command
    PNX_OPT_COUNT
}PNX_CMD_OPT;

typedef enum Pnx_Hel_Flg
{
    CMM = PNX_OPT_CMM,// Only dispaly Configurable memory map
    HELP = PNX_OPT_HEL,// Just dispaly helper information
    BOTH,// (Reserved)Both configurable memory map and help information
    MAX
}PNX_HEL_FLG;

NFC_STATUS Pnx_Print(PNX_HANDLE pNx,const char *fmt, ...)
{
    if(pNx->verbose != TRUE)
    {
        usleep(5);
        return NFC_RES_OK;
    }
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stdout, fmt, ap);
	va_end(ap);
	return NFC_RES_OK;
}

#define RELEASE 1
static void Pnx_Cmd_Usage( char * pname,PNX_HEL_FLG pnflg)
{
    if( pnflg == CMM)
    {
        printf( "\nRegister:\n");
        printf( "\t\t++++++++++++++RAM+++++++++++++0x010000\n" );
        printf( "\t\t+////////////////////////////+\n" );
        printf( "\t\t+ DigiPwr Register Bank(1k)  +\n" );
        printf( "\t\t+////////////////////////////+\n" );
        printf( "\t\t++++++++++++++++++++++++++++++0x00FC00\n" );
        printf( "\t\t+////////////////////////////+\n" );
        printf( "\t\t+ CL-CoPro Register Bank(1k) +\n" );
        printf( "\t\t+////////////////////////////+\n" );
        printf( "\t\t++++++++++++++++++++++++++++++0x00F800\n" );
        printf( "Configuration:\n");
        printf( "\t\t+++++++++++++EEPROM+++++++++++0x00A000\n" );
        printf( "\t\t+////////////////////////////+\n" );
        printf( "\t\t+////PNx Configuration(2k)///+\n" );
        printf( "\t\t+////////////////////////////+\n" );
        printf( "\t\t++++++++++++++++++++++++++++++0x009800\n" );
        return;
    }
    printf( "\nUsage: %s [options]\n", pname );
    printf( "Description: A utility to support NXP NFC PNx chipset.\n" );
    printf( "Version:1.0\n" );
    printf( "Options:\n" );
    printf( "\t--? | --h | --help   \tPNx Command Help\n");
    printf( "\t--pcv            \tPNx Chip Version(HW,FW version)\n");
    printf( "\t--ast            \tAntenna Self Test,(FW>108)Does not support\n" );
    printf( "\t--sst            \tSWP Self Test\n" );
    printf( "\t--chr            \tChip hard Reset\n" );
    printf( "\t--r | --rrv <Addr>\tRead the Register Vaule\n" );
    printf( "\t--w | --wrv <Addr> <Va> Write Register Value\n" );
    printf( "\t--arv            \tAll Register Values\n" );
    printf( "\t--cui            \tCard Unique Identifier\n" );
    printf( "\t--cmm            \tConfigurable Memory Map\n" );
    printf( "\t--ppl            \tPNx Polling Loop\n" );
    printf( "\t--ssd            \tSMX and SWP dynamic Switch(Take effect PNxC3)\n" );
    printf( "\t--swp <Mode>     \tSWP On/Off Test\n" );
    printf( "                 \t\t******<Mode>******\n" );
    printf( "                 \t\t%d = SWP SE Off(Disable)\n",0 );
    printf( "                 \t\t%d = SWP SE On(Enable)\n",1 );
    printf( "\t--smx <Mode>     \tSMX Wired/Virtual Test\n" );
    printf( "                 \t\t******<Mode>******\n" );
    printf( "                 \t\t%d = S2C&NFC-WI SE On(Wired Mode)\n",0 );
    printf( "                 \t\t%d = S2C&NFC-WI SE On(Virtual Mode)\n",1 );
    printf( "\t--prb <Type> <Rate>\tPRBS test Command\n" );
    printf( "                 \t\t******<Type>******\n" );
    printf( "                 \t\t%d = TypeA\n",TYPE_A );
    printf( "                 \t\t%d = TypeB\n",TYPE_B );
    printf( "                 \t\t%d = TypeF\n",TYPE_F );
    printf( "                 \t\t%d = No modulation\n",NO_MODULATION );
    printf( "                 \t\t******<Rate>******\n" );
    printf( "                 \t\t%d = 106 Kbps\n",_106_KBPS );
    printf( "                 \t\t%d = 212 Kbps\n",_212_KBPS );
    printf( "                 \t\t%d = 424 Kbps\n",_424_KBPS );
#if defined RELEASE
        "\tIf you meet some problem, please contact nfcsphinx@gamil.com\n\n");
#else
    printf( "\tIf you meet some problem, please contact xiaohua.wang@nxp.com\n" );
    printf( "\tCopyright (c) 2011-2012, NXP Semiconductors N.V.\n\n" );
#endif
    return;
}

NFC_STATUS Pnx_Verbose_Check(PNX_HANDLE pNx)
{
    int i=0;
    
    for(i=1;i<pNx->argc;i++)
    {
        if(!strcmp(pNx->argv[i], "--v"))
        {
            pNx->verbose = TRUE;
            break;
        }
    }
    
    return NFC_RES_OK;
}

NFC_STATUS Pnx_Cmd_Handle(PNX_HANDLE pNx)
{
    int c = 0;
    uint32_t i=0;
    uint16_t value = 0;
    uint16_t addr = 0;
    uint16_t buffer[MAX_FRAME_LEN] = {0};
    
    pNx->pnx_verbose_check(pNx);
    
    static char *ShortOptions = ":";
    static struct option LongOptions[] =
    {
        { "help", no_argument, NULL, PNX_OPT_HEL},
        { "h", no_argument, NULL, PNX_OPT_HEL },
        { "?", no_argument, NULL, PNX_OPT_HEL },
        { "pcv", no_argument, NULL, PNX_OPT_PCV },
        { "ast", no_argument, NULL, PNX_OPT_AST },
        { "sst", no_argument, NULL, PNX_OPT_SST },
        { "swp", required_argument, NULL, PNX_OPT_SWP },
        { "smx", required_argument, NULL, PNX_OPT_SMX },
        { "ssd", no_argument, NULL, PNX_OPT_SSD },
        { "chr", no_argument, NULL, PNX_OPT_CHR },
        { "rrv", required_argument, NULL, PNX_OPT_RRV },
        { "wrv", required_argument, NULL, PNX_OPT_WRV },
        { "arv", no_argument, NULL, PNX_OPT_ARV },
        { "cui", no_argument, NULL, PNX_OPT_CUI },
        { "cmm", no_argument, NULL, PNX_OPT_CMM },
        { "ppl", no_argument, NULL, PNX_OPT_PPL },
        { "prb", required_argument, NULL, PNX_OPT_PRB },
        { NULL, 0, NULL, 0 }
    };

    while((c = getopt_long(pNx->argc,pNx->argv,ShortOptions,LongOptions,
                            NULL )) != -1 )
    {
        switch( c )
        {
            case PNX_OPT_HEL:
            case PNX_OPT_CMM:
                Pnx_Cmd_Usage(pNx->argv[0],(PNX_HEL_FLG)c);
                break;
                
            case PNX_OPT_PCV:
                system("cat /proc/cpuinfo");
                if( Pnx_Get_Pvc(pNx) !=  NFC_RES_OK)    
                {
                    printf("Pnx_Get_Pvc [FAIL]\n");
                    return NFC_RES_ERR;
                }
                break;
                
            case PNX_OPT_AST:
                Pnx_Set_Ast(pNx,buffer,0);
                break;
            
            case PNX_OPT_SST:
                if( Pnx_Set_Sst(pNx,buffer)!=  NFC_RES_OK)
                {
                    printf("Pnx_Set_Sst [FAIL]\n");
                    return NFC_RES_ERR;
                }
                break;
            case PNX_OPT_SWP:
                if(strtoul( optarg, NULL, 0 )>1)
                {
                    printf("Please enter the correct parameters.\n" );
                    return NFC_RES_ERR;
                }
                if( Pnx_Set_Swp(pNx,strtoul( optarg, NULL, 0 ))!=  NFC_RES_OK)
                {
                    printf("Pnx_Set_Sot [FAIL]\n");
                    return NFC_RES_ERR;
                }
                break;

            case PNX_OPT_SMX:
                if(strtoul( optarg, NULL, 0 )>1)
                {
                    printf("Please enter the correct parametPNx Polling Loopers.\n" );
                    return NFC_RES_ERR;
                }
                if( Pnx_Set_Smx(pNx,strtoul( optarg, NULL, 0 ))!=  NFC_RES_OK)
                {
                    printf("Pnx_Set_Sot [FAIL]\n");
                    return NFC_RES_ERR;
                }
                break;  
  
            case PNX_OPT_CHR:
                Pnx_Set_Chr(pNx);
                break;
                
            case 'w':
            case PNX_OPT_WRV:
                if( NULL == pNx->argv[Value] )
                {
                    printf("Please enter the correct parameters.\n" );
                    return NFC_RES_ERR;
                }
                if(Pnx_Set_Register(pNx,strtoul( optarg, NULL, 0 ), strtoul( pNx->argv[3], NULL, 0 )) !=  NFC_RES_OK) 
                {
                    printf("Pnx_Set_Register [FAIL]\n");
                    return NFC_RES_ERR;
                }
                if(Pnx_Get_Register(pNx,strtoul( optarg, NULL, 0 ), &value) !=  NFC_RES_OK) 
                {
                    printf("Pnx_Get_Register [FAIL]\n");
                    return NFC_RES_ERR;
                }
                if(value == strtoul( pNx->argv[3], NULL, 0 ))
                {
                    printf( "\n0x00%02x 0x%02x [Write succeed]\r\n",strtoul( optarg, NULL, 0 ),strtoul( pNx->argv[3], NULL, 0 ));
                }
                else
                {
                    printf( "\n0x00%02x 0x%02x [Write fail]\r\n",strtoul( optarg, NULL, 0 ),strtoul( pNx->argv[3], NULL, 0 ));
                }
                break; 

            case 'r':
            case PNX_OPT_RRV:
                if(Pnx_Get_Register(pNx,strtoul(optarg,NULL,0), &value) !=  NFC_RES_OK) 
                {
                    printf("Pnx_Get_Register [FAIL]\n");
                    return NFC_RES_ERR;
                }
                if(pNx->hide == TRUE)
                {
                    printf("\n0x00%02x 0x--(Hide)\r\n\n",strtoul(optarg,NULL,0));
		            pNx->hide = FALSE;
                }
                else
		        {
                    printf("\n0x00%02x 0x%02x\r\n\n",strtoul(optarg,NULL,0),value);
		        }
                break;

            case PNX_OPT_ARV:
                for(i=1;i<=EEPROM_TOP-EEPROM_BOTTOM;i++)
                {
                    if(Pnx_Get_Register(pNx,EEPROM_BOTTOM+i, &value)  !=  NFC_RES_OK) 
                    {
                        printf("Pnx_Get_Register [FAIL]\n");
                        return NFC_RES_ERR;
                    }
                    if((i%ROW) != 1 && i != 1)
                    {
        		        if(pNx->hide == TRUE)
        		        {
        		            printf(" --");
            			    pNx->hide = FALSE;
        		        }
        		        else
        			    {
        		            printf(" %02x",value );
        			    }
                    }
                    else
                    {
        		        if(pNx->hide == TRUE)
        		        {
        		            printf("\n0x00%02x --",EEPROM_BOTTOM+i);
            			    pNx->hide = FALSE;
        		        }
        		        else
        			    {
        		            printf("\n0x00%02x %02x",EEPROM_BOTTOM+i,value );
        			    }
                    }
                }
                printf("\n");

                for(i=1;i<=RAM_TOP-RAM_BOTTOM;i++)
                {
                    if(Pnx_Get_Register(pNx,RAM_BOTTOM+i, &value) !=  NFC_RES_OK) 
                    {
                        printf("Pnx_Get_Register [FAIL]\n");
                        return NFC_RES_ERR;
                    }
                    if((i%ROW) != 1 && i != 1 )
                    {
        		        if(pNx->hide == TRUE)
        		        {
        		            printf(" --");
            			    pNx->hide = FALSE;
        		        }
        		        else
            			{
        		            printf(" %02x",value );
            			}
                    }
                    else
                    {
        		        if(pNx->hide == TRUE)
        		        {
        		            printf("\n0x00%02x --",RAM_BOTTOM+i);
            			    pNx->hide = FALSE;
        		        }
        		        else
            			{
        		            printf("\n0x00%02x %02x",RAM_BOTTOM+i,value );
            			}
                    }
                }
                printf("\n");
                break;
            
            case PNX_OPT_CUI:
                if( Pnx_Get_Tag(pNx,buffer) !=  NFC_RES_OK)
                {
                    printf("Pnx_Get_Tag [FAIL]\n" );
                    return NFC_RES_ERR;
                }
                break;
            case PNX_OPT_PPL:
                while(1)
                {
                    if( Pnx_Get_Tag(pNx,buffer) !=  NFC_RES_OK)
                    {
                        printf("Pnx_Get_Tag [FAIL]\n" );
                        return NFC_RES_ERR;
                    }
                    else
                    {
                        printf("\t\t***********************************\n");
                    }
                }
                break;
            case PNX_OPT_PRB:
                if( NULL == pNx->argv[Value] || strtoul( optarg, NULL, 0 ) > NO_MODULATION || strtoul( pNx->argv[3], NULL, 0 ) > _424_KBPS)
                {
                    printf("Please enter the correct parameters.\n" );
                    return NFC_RES_ERR;
                }
                if( Pnx_Set_Prb(pNx,strtoul( optarg, NULL, 0 ), strtoul( pNx->argv[3], NULL, 0 )) !=  NFC_RES_OK)
                {
                    printf("Pnx_Set_Prb [FAIL]\n" );
                    return NFC_RES_ERR;
                }
                break;
               
            default:
                /* Display options are handled later. */
                break;
        }
    }
    return NFC_RES_OK;
}

