// verify.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "getopt.h"//getopt_long
#include "stdlib.h"//EXIT_SUCCESS
#include "stdio.h"
#include "stdarg.h"

typedef signed char int8_t; 
typedef signed short int16_t; 
typedef signed int int32_t; 
typedef unsigned char uint8_t; 
typedef unsigned short uint16_t; 
typedef unsigned int uint32_t;

#define Address 2
#define Value   3
#define CRC1_CRC2           0x02 // CRC1 + CRC2
#define Length_CRC1_CRC2    0x03 // Length + CRC1 + CRC2

#define Chaining_Bit_Single 0x80  // 0x80 | pID
#define Chaining_Bit_Fragment   0x7F // 0x7F & pID
#define I_Frame_Mask    0x80
#define S_Frame_Mask    0xC0
#define U_Frame_Mask    0xE0

typedef enum Pnx_Cmd_Opt
{
    PNX_OPT = 255,// Placeholder to set start value
    PNX_OPT_HEL,// PNx Command Help
    PNX_OPT_PCV,// PNx Chip Version(HW,FW version)
    PNX_OPT_AST,// Antenna Self Test
    PNX_OPT_SST,// SWP Self Test
    PNX_OPT_CSR,// Chip Soft Reset
    PNX_OPT_RRV,// Read the Register Vaule
    PNX_OPT_WRV,// Write Register Value
    PNX_OPT_ARV,// All Register Values
    PNX_OPT_CUI,// Card Unique Identifier
    PNX_OPT_CMM,// Configurable Memory Map
    PNX_OPT_COUNT
}PNX_CMD_OPT;

typedef enum Pnx_Hel_Flg
{
    CMM,// Only dispaly Configurable memory map
    HELP,// Just dispaly helper information
    BOTH,// (Reserved)Both configurable memory map and help information
    MAX
}PNX_HEL_FLG;
#include <string.h>
static int Verbose = false;

int Pnx_Hci_Print(const char *fmt, ...)
{
    if(Verbose != true)
    return 0;
	int ret;
	va_list ap;

	va_start(ap, fmt);
	ret = vfprintf(stdout, fmt, ap);
	va_end(ap);
	return (ret);
}


static void Nxp_Pnx_Verbose_Check(int argc, char* argv[])
{
        int i=0;
        for(i=1;i<argc;i++){
        if(!strcmp(argv[i], "--v")){
            Verbose = true;
            Pnx_Hci_Print("Verbose####\n");
            Pnx_Hci_Print("Verbose----%d\n",Verbose);
            break;
        }
        }
        return;

}
static void Nxp_Pnx_Usage( char * pname,PNX_HEL_FLG pnflg)
{
    if( pnflg == CMM && Verbose == true)
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
    printf( "Options:\n" );
    printf( "\t--? | --h | --help\tPNx Command Help\n");
    printf( "\t--pcv                 \tPNx Chip Version(HW,FW version)\n");
    printf( "\t--ast                 \tAntenna Self Test\n" );
    printf( "\t--sst                 \tSWP Self Test\n" );
    printf( "\t--csr                 \tChip Soft Reset\n" );
    printf( "\t--rrv <Address>       \tRead the Register Vaule\n" );
    printf( "\t--wrv <Address> <Value>\tWrite Register Value\n" );
    printf( "\t--arv                 \tAll Register Values\n" );
    printf( "\t--cui                 \tCard Unique Identifier\n" );
    printf( "\t--cmm                 \tConfigurable Memory Map\n" );
    printf( "\tIf you meet some problem, please contact xiaohua.wang@nxp.com\n" );
    printf( "\tCopyright (c) 2011-2012, NXP Semiconductors N.V.\n\n" );
    return;
}
typedef enum
{
   NFC_HCI_U_RSET = 0,
   NFC_HCI_CLEARALLPIPES,
   NFC_HCI_OPEN_PIPE_ADMIN,
   NFC_HCI_CREATE_PIPE_LINK_MGNT,
   NFC_HCI_OPEN_PIPE_LINK_MGNT,
   NFC_HCI_CREATE_PIPE_ID_MGNT,
   NFC_HCI_OPEN_PIPE_ID_MGNT,
   NFC_HCI_VERSION_SW,
   NFC_HCI_HCI_VERSION,
   NFC_HCI_VERSION_HARD,
   NFC_HCI_VENDOR_NAME,
   NFC_HCI_MODEL_ID,
   NFC_HCI_GATES_LIST,
   NFC_HCI_SET_HW_CONF,
   NFC_HCI_SET_FRAC_CLKSEL,
   NFC_HCI_SET_PL_CLKREQ,
   NFC_HCI_SET_PWR_STATUS,
   NFC_HCI_SET_REQUEST_POWER,
   NFC_HCI_SET_TIMING,
   NFC_HCI_SET_AST_GSP1,
   NFC_HCI_SET_AST_GSP2,
   NFC_HCI_SET_AST_CURRENT,
   NFC_HCI_SET_TOLERANCE,
   NFC_HCI_SELF_TEST_ANTENNA,
   NFC_HCI_OPEN_PIPE,
   NFC_HCI_CREATE_PIPE_READER_A,
   NFC_HCI_CREATE_PIPE_READER_B,
   NFC_HCI_CREATE_PIPE_READER_FELICA,
   NFC_HCI_CREATE_PIPE_NFCINITIATOR,
   NFC_HCI_SET_NFC_I_MODE_SPEED,
   NFC_HCI_CREATE_PIPE_PL_MNGT,
   NFC_HCI_CREATE_PIPE_READER_JEWEL,
   NFC_HCI_CREATE_PIPE_READER_ISO15693,
   NFC_HCI_SET_RD_PHASE,
   NFC_HCI_SET_RD_PHASE2,
   NFC_HCI_STOP_TYPE_A_ACT,
   NFC_HCI_SEND_EVT_READER_REQ,
   NFC_HCI_WRITE_MEM,
   NFC_HCI_READ_MEM,
   NFC_HCI_CREATE_PIPE_SYSTEM_MGNT,
   NFC_HCI_OPEN_PIPE_SYSTEM_MGNT,
   NFC_HCI_TEST_SWP,
   NFC_HCI_SET_XTAL_WAIT_TIME1,
   NFC_HCI_SET_XTAL_WAIT_TIME2,
   NFC_HCI_NFPL_OPEN_PIPE_ADMIN,
   NFC_HCI_NFPL_ADMIN_CLEAR_ALLPIPE,
   NFC_HCI_NFPL_OPEN_PIPE_LINK_MGT,
   NFC_HCI_NFPL_CREATE_PIPE_PL_MNGT,
   NFC_HCI_NFPL_OPEN_PIPE_PL_MNGT,
   NFC_HCI_NFPL_OPEN_PIPE,
   NFC_HCI_NFPL_SET_RD_PHASE,
   NFC_HCI_NFPL_SET_RD_PHASE2,
   NFC_HCI_NFPL_CREATE_PIPE_READER_A,
   NFC_HCI_NFPL_SET_NFC_I_MODE_SPEED_A,
   NFC_HCI_NFPL_CREATE_PIPE_READER_B,
   NFC_HCI_NFPL_SET_NFC_I_MODE_SPEED_B,
   NFC_HCI_NFPL_CREATE_PIPE_READER_FELICA,
   NFC_HCI_NFPL_SEND_EVT_READER_REQ,
   NFC_HCI_NFPL_POST_EVENT_REQ_1,
   NFC_HCI_NFPL_POST_EVENT_REQ_2,
   NFC_HCI_NFPL_POST_EVENT_REQ_3,
   NFC_HCI_NFPL_POST_EVENT_REQ_4
} nfc_hci_cmd_t;

#define MAX_FRAME_LEN    0x21    /* Max 33 bytes per frame */

typedef struct
{
  char cmd;                  /* Command id */
  char sz;                   /* Size of hci buffer */
  char hci[MAX_FRAME_LEN];   /* Up to NXP LLC(2bytes) + ETSI HCI(29 bytes) + CRC(2bytes) */
} nfc_hci_t;

static const nfc_hci_t nfc_hci_tab[] =
{
  {NFC_HCI_U_RSET,0x3,{0xf9,0x4,0x0}},
  {NFC_HCI_CLEARALLPIPES,0x3,{0x80,0x81,0x14}},
  {NFC_HCI_OPEN_PIPE_ADMIN,0x3,{0x80,0x81,0x03}},
  {NFC_HCI_CREATE_PIPE_LINK_MGNT,0x6,{0x80,0x81,0x10,0x20,0x0,0x90}},
  {NFC_HCI_OPEN_PIPE_LINK_MGNT,0x3,{0x80,0x82,0x03}},
  {NFC_HCI_CREATE_PIPE_ID_MGNT,0x6,{0x80,0x81,0x10,0x05,0x00,0x05}},
  {NFC_HCI_OPEN_PIPE_ID_MGNT,0x3,{0x80,0x82,0x03}},
  {NFC_HCI_VERSION_SW,0x4,{0x80,0x82,0x02,0x01}},
  {NFC_HCI_HCI_VERSION,0x4,{0x80,0x82,0x02,0x02}},
  {NFC_HCI_VERSION_HARD,0x4,{0x80,0x82,0x02,0x03}},
  {NFC_HCI_VENDOR_NAME,0x4,{0x80,0x82,0x02,0x04}},
  {NFC_HCI_MODEL_ID,0x4,{0x80,0x82,0x02,0x05}},
  {NFC_HCI_GATES_LIST,0x4,{0x80,0x82,0x02,0x06}},
  {NFC_HCI_SET_HW_CONF,0x7,{0x80,0x82,0x3F,0x00,0x98,0x10,0xBC}},
  {NFC_HCI_SET_FRAC_CLKSEL,0x7,{0x80,0x82,0x3F,0x00,0x98,0x09,0x00}},
  {NFC_HCI_SET_PL_CLKREQ,0x7,{0x80,0x82,0x3F,0x00,0x9E,0x71,0x00}},
  {NFC_HCI_SET_PWR_STATUS,0x7,{0x80,0x82,0x3F,0x00,0x9E,0xAA,0x00}}, // TODO: Active bat(0x00) -> Standby(0x01)
  {NFC_HCI_SET_REQUEST_POWER,0x7,{0x80,0x82,0x3F,0x00,0x9E,0xB4,0x00}},
  /* Last two bytes will be changed for the different loops */
  {NFC_HCI_SET_TIMING,0x7,{0x80,0x82,0x3F,0x00,0x9C,0xA0,0x10}},
  /* Last byte will be set according to self test input param 1 */
  {NFC_HCI_SET_AST_GSP1,0x7,{0x80,0x82,0x3F,0x00,0x98,0x9F,0x2D}},
  /* Last byte will be set according to self test input param 2 */
  {NFC_HCI_SET_AST_GSP2,0x7,{0x80,0x82,0x3F,0x00,0x98,0x9D,0x27}},
  {NFC_HCI_SET_AST_CURRENT,0x7,{0x80,0x82,0x3F,0x00,0x98,0x9E,0x3F}},
  /* Last two bytes will be changed for the different loops */
  {NFC_HCI_SET_TOLERANCE,0x7,{0x80,0x82,0x3F,0x00,0x9B,0xF5,0x00}},
  {NFC_HCI_SELF_TEST_ANTENNA,0x7,{0x80,0x82,0x20,0x03,0x05,0x0E,0xFF}},
  /* Opens a pipe, byte two sets pipe nbr */
  {NFC_HCI_OPEN_PIPE,0x03,{0x80,0x80,0x03}},
  {NFC_HCI_CREATE_PIPE_READER_A,0x06,{0x80,0x81,0x10,0x20,0x00,0x13}},
  {NFC_HCI_CREATE_PIPE_READER_B,0x06,{0x80,0x81,0x10,0x20,0x00,0x11}},
  {NFC_HCI_CREATE_PIPE_READER_FELICA,0x06,{0x80,0x81,0x10,0x20,0x00,0x14}},
  {NFC_HCI_CREATE_PIPE_NFCINITIATOR,0x06,{0x80,0x81,0x10,0x20,0x00,0x30}},
  {NFC_HCI_SET_NFC_I_MODE_SPEED,0x05,{0x80,0x83,0x01,0x01,0x08}},
  {NFC_HCI_CREATE_PIPE_PL_MNGT,0x06,{0x80,0x81,0x10,0x20,0x00,0x94}},
  {NFC_HCI_CREATE_PIPE_READER_JEWEL,0x06,{0x80,0x81,0x10,0x15,0x00,0x15}},
  {NFC_HCI_CREATE_PIPE_READER_ISO15693,0x06,{0x80,0x81,0x10,0x12,0x00,0x12}},
  {NFC_HCI_SET_RD_PHASE,0x05,{0x80,0x86,0x01,0x06,0x7F}},
  {NFC_HCI_SET_RD_PHASE2,0x04,{0x80,0x86,0x02,0x06}},
  {NFC_HCI_STOP_TYPE_A_ACT,0x05,{0x80,0x82,0x01,0x10,0x00}},
  {NFC_HCI_SEND_EVT_READER_REQ,0x03,{0x80,0x84,0x50}},
  /* Read/Write Mem (inc. register and eeprom) */
  {NFC_HCI_WRITE_MEM,0x7,{0x80,0x82,0x3F,0x00,0x00,0x00,0x00}},
  {NFC_HCI_READ_MEM,0x6,{0x80,0x82,0x3E,0x00,0x00,0x00}},
  {NFC_HCI_CREATE_PIPE_SYSTEM_MGNT,0x6,{0x80,0x81,0x10,0x90,0x00,0x90}},
  {NFC_HCI_OPEN_PIPE_SYSTEM_MGNT,0x3,{0x80,0x82,0x03}},
  {NFC_HCI_TEST_SWP,0x3,{0x80,0x82,0x21}},
  {NFC_HCI_SET_XTAL_WAIT_TIME1,0x7,{0x80,0x82,0x3f,0x00,0x9c,0x5c,0x07}},
  {NFC_HCI_SET_XTAL_WAIT_TIME2,0x7,{0x80,0x82,0x3f,0x00,0x9c,0x5d,0xcc}},
  /* NFPL Polling Loop related commands */
  {NFC_HCI_NFPL_OPEN_PIPE_ADMIN,0x03,{0x80,0x81,0x03}},
  {NFC_HCI_NFPL_ADMIN_CLEAR_ALLPIPE,0x03,{0x80,0x81,0x14}},
  {NFC_HCI_NFPL_OPEN_PIPE_LINK_MGT,0x03,{0x80,0x80,0x03}},
  {NFC_HCI_NFPL_CREATE_PIPE_PL_MNGT,0x06,{0x80,0x81,0x10,0x20,0x00,0x94}},
  {NFC_HCI_NFPL_OPEN_PIPE_PL_MNGT,0x03,{0x80,0x82,0x03}},
  {NFC_HCI_NFPL_OPEN_PIPE,0x03,{0x80,0x80,0x03}},
  {NFC_HCI_NFPL_SET_RD_PHASE,0x05,{0x80,0x82,0x01,0x06,0x7F}},
  {NFC_HCI_NFPL_SET_RD_PHASE2,0x04,{0x80,0x82,0x02,0x06}},
  {NFC_HCI_NFPL_CREATE_PIPE_READER_A,0x06,{0x80,0x81,0x10,0x13,0x00,0x13}},
  {NFC_HCI_NFPL_SET_NFC_I_MODE_SPEED_A,0x05,{0x80,0x83,0x01,0x10,0x00}},
  {NFC_HCI_NFPL_CREATE_PIPE_READER_B,0x06,{0x80,0x81,0x10,0x13,0x00,0x11}},
  {NFC_HCI_NFPL_SET_NFC_I_MODE_SPEED_B,0x05,{0x80,0x83,0x01,0x02,0x00}},
  {NFC_HCI_NFPL_CREATE_PIPE_READER_FELICA,0x06,{0x80,0x81,0x10,0x20,0x00,0x14}},
  {NFC_HCI_NFPL_SEND_EVT_READER_REQ,0x03,{0x80,0x83,0x50}},
  {NFC_HCI_NFPL_POST_EVENT_REQ_1,0x03,{0x80,0x83,0x04}},
  {NFC_HCI_NFPL_POST_EVENT_REQ_2,0x04,{0x80,0x81,0x11,0x03}},
  {NFC_HCI_NFPL_POST_EVENT_REQ_3,0x03,{0x80,0x82,0x04}},
  {NFC_HCI_NFPL_POST_EVENT_REQ_4,0x04,{0x80,0x81,0x11,0x02}}
};

typedef enum
{
   FELICA_MAIN,
   NFC_SECOND,
   NFC_MAX   // Always last
} nfc_id_t;
typedef enum
{
   NFC_RES_OK,
   NFC_RES_ERR,
   NFC_RES_TMO,       
   NFC_RES_NOT_FOUND,
   NFC_RES_SYNTAX
} nfc_res_t;

static bool nfc_pnx_started = false;

void sleep(int a)
{
    while(a)
    {
        a--;
    }
}
void pnx_DbgTrace(uint8_t data[], uint32_t size)
{
   uint32_t i;

   if(size == 0)
      return;

   for(i = 0; i < size; i++)
   {
      if((i % 10) == 0)
      printf("\n\t\t\t");
      printf("%02X ", data[i]);
   }
   printf("\n\tBlock size is: %d\n", size);
   printf("\r");
}

/*-----------------------------------------------------------------------------
Function : nfc_crc
Compute and add CRCs to the frame
------------------------------------------------------------------------------*/
uint16_t nfc_crc(uint8_t* pData, uint8_t length, uint8_t* pCrc1, uint8_t* pCrc2)
{
  uint8_t  crc_byte = 0, 
           index = 0;
  uint16_t crc = 0;

  crc = 0xFFFF;
  do 
  {
    crc_byte = pData[index];
    crc_byte = (crc_byte ^ (uint8_t)((crc) & 0x00FF));
    crc_byte = (crc_byte ^ (crc_byte << 4));
    crc = (crc >> 8) ^ ((uint16_t)crc_byte << 8) ^
          ((uint16_t)crc_byte << 3) ^
          ((uint16_t)crc_byte >> 4);
    index++;
  } while (index < length);

  crc = ~crc; /*  ISO/IEC 13239 (formerly ISO/IEC 3309) */
  *pCrc1 = (uint8_t) (crc & 0xFF);
  *pCrc2 = (uint8_t) ((crc >> 8) & 0xFF);
  crc = *pCrc2;
  crc = (crc << 8) + *pCrc1;
  return crc;
}

void nfc_pnx_init(void)
{
   nfc_pnx_started = false;
   printf("nfc_pnx_init\n");
   return;
}
/*-----------------------------------------------------------------------------
Function : nfc_pnx_start
Open Communication, power up the chip and init the communication by sending U-RSET and
creating the admin. pipe.
-----------------------------------------------------------------------------*/
int nfc_i2c_hnd = 0;
char scfbuffer[MAX_FRAME_LEN*2] = "\0";  /* Print buffer  */
uint16_t n_s=0; /* Number of send frame */
uint16_t n_r=0; /* Number of receive next frame */
static uint16_t only_ack=0;
static uint8_t frame_buf[MAX_FRAME_LEN]={0};
static uint8_t frame_sz=0;

static nfc_hci_t* nfc_pnx_hci_cmd_get(nfc_hci_cmd_t cmd)
{
  return (nfc_hci_t*)&nfc_hci_tab[cmd];//###Porting Pointstatic_CAST(uint8_t, cmd)
}

/*-----------------------------------------------------------------------------
Function : nfc_pnxi_send
This function adds length, N(s), N(R) and both CRCs to the frame p_params and send it to the
pn65.The response is sent back through p_resp.
According to the frame definition below, both header and HCI command are provided as input.

LLC Frame definition:
< 1 Byte > < 1 Byte > < max 29 Bytes > < 2 Bytes >
   Length       Header       HCI command       CRC

 Frame Types
 -----------  
      8     7     6     5             4     3     2     1     
I     1     0     <      N(S)      >     <  N(R)  >
S    1     1     0     <  TYPE   >     <  N(R)  >
U    1     1     1     <               M               >
-----------------------------------------------------------------------------*/
static nfc_res_t nfc_pnx_send(uint8_t *p_params,       /* header + HCI command*/
                                 const uint8_t n_params,  /* Number of bytes */
                                 uint8_t *p_resp,         /* Response buffer*/
                                 uint8_t *p_resplen)      /* Number of bytes returned*/
{
  uint32_t i=0;
  uint8_t len=n_params+Length_CRC1_CRC2;               /* Number of bytes to send including length, crc1 and crc2*/
  uint8_t p_buf[MAX_FRAME_LEN]={0};     /* Max. number of bytes per frame */

  uint8_t frame_t=0;  /* Frame type */
  char cframe;        /* Frame type */
  
  uint8_t* p_crc1 = NULL;  
  uint8_t* p_crc2 = NULL;

  uint8_t local_n_s = 0; /* for check n_s of received frame from PN65 */
  int ret;

  if (nfc_pnx_started)
  {
      if (n_params>MAX_FRAME_LEN-Length_CRC1_CRC2) { //4) {
      printf("Two many parameters");
      return NFC_RES_ERR;
    }

    sleep(3000);

    /* Populate buffer to send. */
    p_buf[0]=n_params+CRC1_CRC2;        /* Adjust frame length i.e from header to crc */
    for (i=0;i<n_params;i++) {
      p_buf[i+1]=*(p_params+i);
    }
    
    /* Host frame type detection */
    frame_t=p_buf[1] & U_Frame_Mask; 
    switch (frame_t)
    {
      case U_Frame_Mask:
       cframe='U';
      break;
      case S_Frame_Mask:
       cframe='S'; 
       /* Adjust frame counters. To be added if needed */
      break;
      default:
        cframe='I';
        /* Adjust frame counters. */
        frame_t = frame_t | n_r | (n_s << 3);//xiaohua,n_r occupy 0,1,2;n_s:occupy 3,4,5
        n_s++;//xiaohua, +1
        n_s&=7;//xiaohua, reduce high bit,only 0~7
        n_r = (n_s + 8 - only_ack) % 8;//(1+8-0)%8=1 ????
      break;
    }
    
    /* Add CRCs */
    p_crc1 = &p_buf[n_params+1];
    p_crc2 = &p_buf[n_params+2];
    nfc_crc((uint8_t*)&p_buf,n_params+1,p_crc1,p_crc2);//+1:include length
    
    /* Debug print */
    scfbuffer[0] = '\0';
    ////scf_bin_to_ascii(scfbuffer,(char*)&p_buf,len,0);//###Porting Point                 
    printf("AP->NFC(%c-frame):");
    pnx_DbgTrace(p_buf, len);

    /* Send request */
#if 0 //2012
if ((ret = (write(nfc_i2c_hnd, &p_buf[0], len)))<0)
{ 
    printf("pn544 write error retcode = %d, errno = %d\n", ret, errno); 
    //goto err_; 
}
#endif 
    //i2c_write(&nfc_i2c_hnd,&p_buf[0],len);//###Porting Point
#if 0 //2012
    /* Read no bytes available on I2C*/
if ((ret = (read(nfc_i2c_hnd, p_resp, 1))) < 0) 
{ printf("pn544 read error retcode = %d, errno = %d\n", ret, errno);
      *p_resplen=0;  /* Clear the length to avoid dcp_print crash */
       return NFC_RES_ERR;
}
#endif

#if 0
    if ((ret = i2c_read(&nfc_i2c_hnd,p_resp,1)) < 0) {
      TRC_S1(nfc_pn65_fuji,
             TRC_DBG_PRINT,
             "Error: i2c_read returned %d.",
             ret);
      *p_resplen=0;  /* Clear the length to avoid dcp_print crash */
       return NFC_RES_ERR;
    }
#endif
    *p_resplen =*p_resp;

    if(*p_resplen >= MAX_FRAME_LEN){
      *p_resplen=MAX_FRAME_LEN;
      *p_resp=MAX_FRAME_LEN;
    }
#if 0 //2012
    /* Read *p_resplen bytes from I2C */
if ((ret = (read(nfc_i2c_hnd, p_resp+1, *p_resplen))) < 0) 
{ printf("pn544 read error retcode = %d, errno = %d\n", ret, errno);
       return NFC_RES_ERR;
}
#endif
#if 0
    if (i2c_read(&nfc_i2c_hnd,p_resp+1,*p_resplen) < 0) {
      TRC_S1(nfc_pn65_fuji,
             TRC_DBG_PRINT,
             "nfc_handle_frame: I2C error, could not fetch %d bytes",
             *p_resplen);
      return NFC_RES_ERR;
    }   
#endif
    /* Slave frame type detection  */
    frame_t=*(p_resp+1) & U_Frame_Mask;
    switch (frame_t)
    {
      case U_Frame_Mask:
        cframe='U';
        break;
      case S_Frame_Mask:
        cframe='S';
        break;
      default:
        cframe='I';
        break;
    }

    /* Debug print */
    //scf_bin_to_ascii(scfbuffer,(char*)p_resp,*p_resplen+1,0);//###Porting Point                   
    printf("NFC->AP(%c-frame):");
    pnx_DbgTrace(p_resp, *p_resplen+1);

    if (cframe=='I') 
    {
      /* check N(r) of I/S frame */
      local_n_s = (*(p_resp+1) & 0x38) >> 3;//local_n_s: 0x38->5,4,3 >>3
      printf("Expected N_R is %d\n", (n_r+7)%8);//0
      printf("Recieved N_R is %d\n", local_n_s);//?
      if ((n_r+7)%8 != local_n_s) //Expected != Recieved       ??????Not Sure
      {   
#if 0 //2012
          /* Read no bytes available on I2C */
          if ((ret = read(nfc_i2c_hnd,p_resp,1)) < 0) {
            printf("Error: i2c_read returned %d.",ret);
            *p_resplen=0;  /* Clear the length to avoid dcp_print crash */
            return NFC_RES_ERR;
          }
#endif
          *p_resplen =*p_resp;

          if(*p_resplen >= MAX_FRAME_LEN){
            *p_resplen=MAX_FRAME_LEN;
            *p_resp=MAX_FRAME_LEN;
          }
#if 0 //2012
          /* Read data from I2C */
          if (read(nfc_i2c_hnd,p_resp+1,*p_resplen) < 0)
          {
            printf("nfc_handle_frame: I2C error, could not fetch %d bytes",*p_resplen);
            return NFC_RES_ERR;
          }
#endif
          /* Slave frame type detection  */
          frame_t=*(p_resp+1) & U_Frame_Mask;
          switch (frame_t)
          {
            case U_Frame_Mask:
              cframe='U';
              break;
            case S_Frame_Mask:
              cframe='S';
              break;
            default:
              cframe='I';
              break;
          }

          // Debug print 
          //scf_bin_to_ascii(scfbuffer,(char*)p_resp,*p_resplen+1,0);//####Porting Point                   
          printf("NFC->AP(%c-frame):");
          pnx_DbgTrace(p_resp, *p_resplen+1);
      }
      else
      {
        /* Return ACK to slave */
        len = 4;
        
        p_buf[0]=3;//Length=3:Only for Length+Header+CRC1+CRC2
        p_buf[1] = S_Frame_Mask | n_r;//0xC1
        
        /* Add CRCs */
        uint8_t* p_crc1 = NULL;  
        uint8_t* p_crc2 = NULL;
        p_crc1 = &p_buf[2];
        p_crc2 = &p_buf[3];
        nfc_crc((uint8_t*)&p_buf,2,p_crc1,p_crc2);
        
        /* Debug print */
        scfbuffer[0] = '\0';
        //scf_bin_to_ascii(scfbuffer,(char*)&p_buf,len,0);  //###Porting Point                 
        printf("AUTO ACK AP->NFC:");
        pnx_DbgTrace(p_buf, len);
#if 0 //2012
        /* Send request */
        write(nfc_i2c_hnd,&p_buf[0],len);
#endif
        return NFC_RES_OK;
      }
    }

    /* Event request are acknowleeged by an S-Frame  */
    /* Wait until the event is fired */
    if (cframe=='S') {
//NFC_HCI_NFPL_SEND_EVT_READER_REQ
      if ((p_params[0]==0x80) && (p_params[1]==0x83) && (p_params[2]==0x50)){
          only_ack = (only_ack + 1) % 8;
          n_r = (n_r + 8 - only_ack) % 8;
          /* no card detect event is taken as ok, so return ok */
          return NFC_RES_OK;
      }
      else {
#if 0 //2012
        /* Read no bytes available on I2C */
        if ((ret = read(nfc_i2c_hnd,p_resp,1)) < 0) {
        printf("Error: i2c_read returned %d.",ret);
          *p_resplen=0;  /* Clear the length to avoid dcp_print crash */
          return NFC_RES_ERR;
        }
#endif
        *p_resplen =*p_resp;

        if(*p_resplen >= MAX_FRAME_LEN){
          *p_resplen=MAX_FRAME_LEN;
          *p_resp=MAX_FRAME_LEN;
        }
#if 0 //2012
        /* Read data from I2C */
        if (read(nfc_i2c_hnd,p_resp+1,*p_resplen) < 0)
        {
        printf("nfc_handle_frame: I2C error, could not fetch %d bytes",*p_resplen);
          return NFC_RES_ERR;
        }
#endif
        /* Slave frame type detection  */
        frame_t=*(p_resp+1) & U_Frame_Mask;
        switch (frame_t)
        {
          case U_Frame_Mask:
            cframe='U';
            break;
          case S_Frame_Mask:
            cframe='S';
            break;
          default:
            cframe='I';
            break;
        }
        /* Debug print */
        //scf_bin_to_ascii(scfbuffer,(char*)p_resp,*p_resplen+1,0); //####Porting Point                  
        printf("NFC->AP(%c-frame):");
        pnx_DbgTrace(p_resp, *p_resplen+1);
      }
    }

    /* Check I-Frame again */
    if (cframe=='I') 
    {
      /* check N(r) of I/S frame */
      local_n_s = (*(p_resp+1) & 0x38) >> 3;
      printf("Expected N_R is %d\n", (n_r+7)%8);
      printf("Recieved N_R is %d\n", local_n_s);
      if ((n_r+7)%8 != local_n_s) {
#if 0 //2012
          /* Read no bytes available on I2C */
          if ((ret = read(nfc_i2c_hnd,p_resp,1)) < 0) {
            printf("Error: i2c_read returned %d.",ret);
            *p_resplen=0;  /* Clear the length to avoid dcp_print crash */
            return NFC_RES_ERR;
          }
#endif
          *p_resplen =*p_resp;

          if(*p_resplen >= MAX_FRAME_LEN){
            *p_resplen=MAX_FRAME_LEN;
            *p_resp=MAX_FRAME_LEN;
          }
#if 0 //2012
          /* Read data from I2C */
          if (read(nfc_i2c_hnd,p_resp+1,*p_resplen) < 0)
          {
            printf("nfc_handle_frame: I2C error, could not fetch %d bytes",*p_resplen);
            return NFC_RES_ERR;
          }
#endif
          /* Debug print */
          //scf_bin_to_ascii(scfbuffer,(char*)p_resp,*p_resplen+1,0);
          printf("NFC->AP(%c-frame):");
          pnx_DbgTrace(p_resp, *p_resplen+1);

          local_n_s = (*(p_resp+1) & 0x38) >> 3;
          printf("Expected N_R is %d\n", (n_r+7)%8);
          printf("Recieved N_R is %d\n", local_n_s);
      }
      /* Return ACK to slave */
      len = 4;

      p_buf[0]=3;
      p_buf[1] = 0xC0 | n_r;

      /* Add CRCs */
      p_crc1 = &p_buf[2];
      p_crc2 = &p_buf[3];
      nfc_crc((uint8_t*)&p_buf,2,p_crc1,p_crc2);

      /* Debug print */
      scfbuffer[0] = '\0';
      //scf_bin_to_ascii(scfbuffer,(char*)&p_buf,len,0);//###Porting Point
      printf("AUTO ACK AP->NFC:");
      pnx_DbgTrace(p_buf, len);
#if 0 //2012
      /* Send request */
      write(nfc_i2c_hnd,&p_buf[0],len);
#endif
    }
    return NFC_RES_OK;
  }
  
  printf("NFC is not yet started.\n");
  return NFC_RES_ERR;
}

static nfc_res_t nfc_pnx_sf(uint8_t cmd,
                              uint8_t pipe_id,
                              uint8_t *p_resp,         /* Response buffer*/
                              uint8_t *p_resplen)      /* Number of bytes returned*/
{ 
  nfc_res_t res=NFC_RES_ERR;
  nfc_hci_t* p_hci;
  uint8_t hci_buf[MAX_FRAME_LEN]={0};
  
  p_hci=nfc_pnx_hci_cmd_get((nfc_hci_cmd_t)cmd);
  memcpy(&hci_buf[0],&p_hci->hci,p_hci->sz);
  if (pipe_id >0) {
    hci_buf[1]= Chaining_Bit_Single | pipe_id;
  }

  res=nfc_pnx_send(hci_buf,p_hci->sz,p_resp,p_resplen);
  sleep(100);   

  return res;
}
/*-----------------------------------------------------------------------------
Function : nfc_pnx_get_register
-----------------------------------------------------------------------------*/
static nfc_res_t nfc_pnx_get_register(uint16_t reg, uint16_t *value)
{
  bool res=false;
  nfc_hci_t* p_hci;
  uint8_t hci_buf[MAX_FRAME_LEN];
  uint8_t hci_resp[MAX_FRAME_LEN];
  uint8_t hci_resplen;

  if(!nfc_pnx_started)
    return NFC_RES_ERR;

  p_hci = nfc_pnx_hci_cmd_get(NFC_HCI_READ_MEM);
  memcpy(hci_buf, &p_hci->hci, p_hci->sz);
  hci_buf[4]=(reg>>8);//0x9810:98
  hci_buf[5]=(reg&0xFF);//0x9810:10
  res = nfc_pnx_send(hci_buf, p_hci->sz, hci_resp, &hci_resplen);
  if(res != NFC_RES_OK)
  {
    printf("set reg failed.\n");
    return NFC_RES_ERR;
  }

  if(hci_resplen < 5)
  {
    printf("wrong resp long from get reg.\n");
    return NFC_RES_ERR;
  }
/*06 81 82 80 00 A2 F0 // Unpacked data = [82 80 00 ]*/
  *value=hci_resp[4];
  return NFC_RES_OK;
}

/*-----------------------------------------------------------------------------
Function : nfc_pnx_set_register
-----------------------------------------------------------------------------*/
static nfc_res_t nfc_pnx_set_register(uint16_t reg, uint16_t value)
{
  bool res=false;
  nfc_hci_t* p_hci;
  uint8_t hci_buf[MAX_FRAME_LEN];
  uint8_t hci_resp[MAX_FRAME_LEN];
  uint8_t hci_resplen;

  if(!nfc_pnx_started)
    return NFC_RES_ERR;

  p_hci = nfc_pnx_hci_cmd_get(NFC_HCI_WRITE_MEM);
  memcpy(hci_buf, &p_hci->hci, p_hci->sz);
  hci_buf[4]=(reg>>8);
  hci_buf[5]=(reg&0xFF);
  hci_buf[6]=value;

  res = nfc_pnx_send(hci_buf, p_hci->sz, hci_resp, &hci_resplen);
  if(res != NFC_RES_OK)
  {
    printf("set reg failed.\n");
    return NFC_RES_ERR;
  }

  return NFC_RES_OK;
}

static nfc_res_t nfc_pnx_start(uint8_t mode)
{
  bool res=false;
  uint8_t r;
  uint16_t value;
  uint16_t addr;
  
  if(!nfc_pnx_started)
  {
    /* Power up - Toggle VEN */
#if 0 //2012
    ioctl(nfc_i2c_hnd, PN544_SET_PWR, 1);
    sleep(5);
    ioctl(nfc_i2c_hnd, PN544_SET_PWR, 0); 
    sleep(10); /* min 7 ms  */
    ioctl(nfc_i2c_hnd, PN544_SET_PWR, 1);  
    sleep(5); /* min 3ms */ 
    
    /* Open I2C port */
    printf("Open I2C port.\n");
	if ((ret = (nfc_i2c_hnd = open("/dev/pn544", O_RDWR))) < 0) {//###Porting Point
		printf("pn544 open error retcode = %d, errno = %d\n", ret, errno);
		return NFC_RES_ERR;
	}
#endif
    n_s=0;  /* reset frame counter */
    n_r=0;  /* reset frame counter */
    only_ack=0;
    
    nfc_pnx_started = true;
    if(nfc_pnx_sf(NFC_HCI_U_RSET,0,(uint8_t*)&frame_buf,&frame_sz) == NFC_RES_ERR) return NFC_RES_ERR; /* Send U-RSET */
    if(nfc_pnx_sf(NFC_HCI_CLEARALLPIPES,0,(uint8_t*)&frame_buf,&frame_sz) == NFC_RES_ERR)  return NFC_RES_ERR;     /* Clear all pipes */
    if(nfc_pnx_sf(NFC_HCI_OPEN_PIPE_ADMIN,0,(uint8_t*)&frame_buf,&frame_sz) == NFC_RES_ERR) return NFC_RES_ERR;   /* Open Administration pipe */
    if(nfc_pnx_sf(NFC_HCI_CREATE_PIPE_LINK_MGNT,0,(uint8_t*)&frame_buf,&frame_sz) == NFC_RES_ERR) return NFC_RES_ERR;  /* Create link management pipe */
    if(nfc_pnx_sf(NFC_HCI_OPEN_PIPE_LINK_MGNT,0,(uint8_t*)&frame_buf,&frame_sz) == NFC_RES_ERR) return NFC_RES_ERR;    /* Open link management pipe */
    
    
    printf("run 2.5!\r\n");
    addr=0x9810;
    if(nfc_pnx_get_register(addr, &value)  ==  NFC_RES_ERR) 
    //return NFC_RES_ERR;
    printf("addr=0x%02x, value=0x%02x!\r\n", addr, value);
    uint16_t vaule_temp = 0x99;
    if(nfc_pnx_set_register(addr, vaule_temp) == NFC_RES_ERR)
    //return NFC_RES_ERR;
    if(nfc_pnx_get_register(addr, &value)  ==  NFC_RES_ERR) 
    //return NFC_RES_ERR;
    printf("addr=0x%02x, value=0x%02x!\r\n", addr, value);
#if 0
    addr=0x9809;
    if(nfc_pnx_get_register(addr, &value)  ==  NFC_RES_ERR) return NFC_RES_ERR;
    printf("addr=0x%02x, value=0x%02x!\r\n", addr, value);
    addr=0x9E71;
    if(nfc_pnx_get_register(addr, &value)  ==  NFC_RES_ERR) return NFC_RES_ERR;
    printf("addr=0x%02x, value=0x%02x!\r\n", addr, value);
    addr=0x9EAA;
    if(nfc_pnx_get_register(addr, &value)  ==  NFC_RES_ERR) return NFC_RES_ERR;
    printf("addr=0x%02x, value=0x%02x!\r\n", addr, value);
    addr=0x9EB4;
    if(nfc_pnx_get_register(addr, &value)  ==  NFC_RES_ERR) return NFC_RES_ERR;
    printf("addr=0x%02x, value=0x%02x!\r\n", addr, value);
    addr=0x9C5C;
    if(nfc_pnx_get_register(addr, &value)  ==  NFC_RES_ERR) return NFC_RES_ERR;
    printf("addr=0x%02x, value=0x%02x!\r\n", addr, value);
    addr=0x9C5D;
    if(nfc_pnx_get_register(addr, &value)  ==  NFC_RES_ERR) return NFC_RES_ERR;
    printf("addr=0x%02x, value=0x%02x!\r\n", addr, value);
    
    
    if(nfc_pnx_sf(NFC_HCI_SET_HW_CONF,0,(uint8_t*)&frame_buf,&frame_sz) == NFC_RES_ERR) return NFC_RES_ERR;       /* Use external crystal for clock generation */
    if(nfc_pnx_sf(NFC_HCI_SET_FRAC_CLKSEL,0,(uint8_t*)&frame_buf,&frame_sz) == NFC_RES_ERR) return NFC_RES_ERR;   /* Set FRAC_ClkSel no input clock */
    if(nfc_pnx_sf(NFC_HCI_SET_PL_CLKREQ,0,(uint8_t*)&frame_buf,&frame_sz) == NFC_RES_ERR) return NFC_RES_ERR;     /* Setting for PlClockRequest => 0x00 */
    if(nfc_pnx_sf(NFC_HCI_SET_PWR_STATUS,0,(uint8_t*)&frame_buf,&frame_sz) == NFC_RES_ERR) return NFC_RES_ERR;    /* Setting for pwr status - Set PWR_STATUS => Standby */
    if(nfc_pnx_sf(NFC_HCI_SET_REQUEST_POWER,0,(uint8_t*)&frame_buf,&frame_sz) == NFC_RES_ERR) return NFC_RES_ERR; /* Setting for Request Power - Set SwpMgt_Request_Power => 0x00 */
    if(nfc_pnx_sf(NFC_HCI_SET_XTAL_WAIT_TIME1,0,(uint8_t*)&frame_buf,&frame_sz) == NFC_RES_ERR) return NFC_RES_ERR; /* Setting for Xtal waiting time1 */
    if(nfc_pnx_sf(NFC_HCI_SET_XTAL_WAIT_TIME2,0,(uint8_t*)&frame_buf,&frame_sz) == NFC_RES_ERR) return NFC_RES_ERR; /* Setting for Xtal waiting time2 */
    
    
    TRC_S0(nfc_pnx, TRC_DBG_PRINT, "run 3!\r\n");
    addr=0x9810;
    if(nfc_pnx_get_register(addr, &value)  ==  NFC_RES_ERR) return NFC_RES_ERR;
    printf("addr=0x%02x, value=0x%02x!\r\n", addr, value);
    addr=0x9809;
    if(nfc_pnx_get_register(addr, &value)  ==  NFC_RES_ERR) return NFC_RES_ERR;
    printf("addr=0x%02x, value=0x%02x!\r\n", addr, value);
    addr=0x9E71;
    if(nfc_pnx_get_register(addr, &value)  ==  NFC_RES_ERR) return NFC_RES_ERR;
    printf("addr=0x%02x, value=0x%02x!\r\n", addr, value);
    addr=0x9EAA;
    if(nfc_pnx_get_register(addr, &value)  ==  NFC_RES_ERR) return NFC_RES_ERR;
    printf("addr=0x%02x, value=0x%02x!\r\n", addr, value);
    addr=0x9EB4;
    if(nfc_pnx_get_register(addr, &value)  ==  NFC_RES_ERR) return NFC_RES_ERR;
    printf("addr=0x%02x, value=0x%02x!\r\n", addr, value);
    addr=0x9C5C;
    if(nfc_pnx_get_register(addr, &value)  ==  NFC_RES_ERR) return NFC_RES_ERR;
    printf("addr=0x%02x, value=0x%02x!\r\n", addr, value);
    addr=0x9C5D;
    if(nfc_pnx_get_register(addr, &value)  ==  NFC_RES_ERR) return NFC_RES_ERR;
    printf("addr=0x%02x, value=0x%02x!\r\n", addr, value);
    
    
    printf("run 4!\r\n");
    nfc_pnx_swptest(&r);
#endif     
    return NFC_RES_OK;
  }

  printf("NFC is already started.\n");
  return NFC_RES_ERR;
}

//int _tmain(int argc, _TCHAR* argv[])
int main(int argc, char* argv[])
{
   int c;
static char *ShortOptions = ":";
static struct option LongOptions[] =
{
    { "help", no_argument, NULL, 'h' },
    { "?", no_argument, NULL, PNX_OPT_HEL },
    { "pcv", no_argument, NULL, PNX_OPT_PCV },
    { "ast", no_argument, NULL, PNX_OPT_AST },
    { "sst", no_argument, NULL, PNX_OPT_SST },
    { "csr", no_argument, NULL, PNX_OPT_CSR },
    { "rrv", required_argument, NULL, PNX_OPT_RRV },
    { "wrv", required_argument, NULL, PNX_OPT_WRV },
    { "arv", no_argument, NULL, PNX_OPT_ARV },
    { "cui", no_argument, NULL, PNX_OPT_CUI },
    { "cmm", no_argument, NULL, PNX_OPT_CMM },
    { NULL, 0, NULL, 0 }
};
nfc_pnx_init();
Nxp_Pnx_Verbose_Check(argc, argv);
while(( c = getopt_long( argc, argv, ShortOptions, LongOptions,
                         NULL )) != -1 )
{
        switch( c )
        {
          case 'h':
          case PNX_OPT_HEL:
              //printf("###[0x%02x]\n",0x80 | 1 | (1 << 3));//Yes it is 0x89
            Nxp_Pnx_Usage( argv[0],HELP);
            return( EXIT_SUCCESS );
            break;

          case PNX_OPT_CMM:
            Nxp_Pnx_Usage( argv[0],CMM);
            return( EXIT_SUCCESS );
            break;

          case PNX_OPT_RRV:
          case PNX_OPT_ARV:
              if( c != PNX_OPT_ARV )
              printf( "Address:0x%x \n",strtoul( optarg, NULL, 0 ));
              nfc_pnx_start(0);
              break;

          case PNX_OPT_WRV:
              if( NULL == argv[Value] )
              {fprintf( stderr, "Please enter the correct parameters.\n" );
              return( EXIT_FAILURE );}

              printf( "Value:0x%x \n",strtoul( argv[3], NULL, 0 ));
            break;

          default:
            fprintf( stderr, "Invalid option specified.\n" );
            return( EXIT_FAILURE );
            /* Display options are handled later. */
            break;
        }
    }
	return 0;
}

