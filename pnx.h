//============================================================================
//  File Name: pnx.h 
//
//  2012 Copyright (c), NXP Semiconductors. All rights reserved.
//
//  This source code is NXP Semiconductors N.V. proprietary and confidential
//  information
//
//  Description:
//      Macros and interface functions     
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
//=============================================================================
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>  
#include <time.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>  

 
#define PN544_CHAR_BASE		'P'
#define PN544_IOR(num, dtype)	_IOR(PN544_CHAR_BASE, num, dtype)
#define PN544_IOW(num, dtype)	_IOW(PN544_CHAR_BASE, num, dtype)
#define PN544_GET_FW_MODE	PN544_IOW(1, unsigned int)
#define PN544_SET_FW_MODE	PN544_IOW(2, unsigned int)
#define PN544_GET_DEBUG		PN544_IOW(3, unsigned int)
#define PN544_SET_DEBUG		PN544_IOW(4, unsigned int)

  
#define PN544_MAGIC	0xE9
/*
 * PN544 power control via ioctl   
 * PN544_SET_PWR(0): power off
 * PN544_SET_PWR(1): power on
 * PN544_SET_PWR(2): reset and power on with firmware download enabled
 */  
#define PN544_SET_PWR	_IOW(PN544_MAGIC, 0x01, unsigned int)

struct pn544_i2c_platform_data 
{
	unsigned int irq_gpio;
	unsigned int ven_gpio;
	unsigned int firm_gpio;
};

#ifndef NULL
#define NULL  ((void *)0)
#endif
#ifndef TRUE
#define TRUE	(0x01)
#endif
#ifndef FALSE
#define FALSE	(0x00)
#endif

#define MAX_FRAME_LEN    0x21    /* Max 33 bytes per frame */

typedef uint8_t	bool_t;

typedef enum
{
   NFC_RES_OK,  
   NFC_RES_ERR,
   NFC_RES_TMO,       
   NFC_RES_NOT_FOUND,
   NFC_RES_SYNTAX
} NFC_STATUS;

typedef enum
{
   TYPE_A,     
   TYPE_B,
   TYPE_F,       
   NO_MODULATION,
   TECH_RFU
} NFC_PBRS_TECH;

typedef enum
{
   _106_KBPS = 0,  
   _212_KBPS,
   _424_KBPS,       
   BITRATE_RFU
} NFC_PBRS_BITRATE;

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
   NFC_HCI_SELF_TEST_PBRS,
   NFC_HCI_CREATE_PIPE_SWP_MGNT,
   NFC_HCI_OPEN_PIPE_SWP_MGNT,
   NFC_EVT_SWP_SWITCH_MODE,
//
   NFC_SMX_HCI_Open_Pipe_Admin,
   NFC_SMX_HCI_Admin_ClearAllPipe,
   NFC_SMX_HCI_ReOpen_Pipe_Admin,
   NFC_SMX_HCI_LINK_MANAGMENT_GATE,
   NFC_SMX_HCI_ADMINISTRATION_GATE,
   NFC_HCI_CREATE_PIPE_SMX_MGNT,
   NFC_HCI_OPEN_PIPE_SMX_MGNT,
   NFC_EVT_SMX_SWITCH_MODE,
   NFC_PH_HCI_GATE_READER_A_GATE,
   NFC_HCI_OPEN_READER_A_GATE,
   NFC_HCI_GATE_PL_MNGT,
   NFC_HCI_OPEN_GATE_PL_MNGT,
   NFC_HCI_SET_RD_PHASE_1,
   NFC_HCI_SET_RD_PHASE_2,
   NFC_EVT_READER_REQUESTED_1,
   NFC_EVT_READER_REQUESTED_2,
   NFC_HCI_READES_UID,
   NFC_HCI_reads_Appl_Data,
   NFC_HCI_reads_SAK,
   NFC_HCI_reads_Fwi,
   NFC_APDU_Select_Card_Manager_JCOP,
   NFC_Get_CPLC_from_JCOP,
   NFC_CLOSE_POLLING_SEND_EVT_END_OPERATION,   
//
   NFC_HCI_NFC_WI_GATE,
   NFC_HCI_OPEN_NFC_WI_GATE,
   NFC_EVT_SE_Switch_virtual,
//
   NFC_HCI_OPEN_PIPE,
   NFC_HCI_CREATE_PIPE_READER_A,
   NFC_HCI_CREATE_PIPE_READER_B,
   NFC_HCI_CREATE_PIPE_READER_FELICA,
   NFC_HCI_CREATE_PIPE_NFCINITIATOR,
   NFC_HCI_SET_NFC_I_MODE_SPEED,
   NFC_HCI_SET_NFC_NEXT_MODE_SPEED,
   NFC_HCI_CREATE_PIPE_PL_MNGT,
   NFC_HCI_CREATE_PIPE_READER_JEWEL,
   NFC_HCI_CREATE_PIPE_READER_ISO15693,
   NFC_HCI_SET_RD_PHASE,
   NFC_HCI_SET_RD_PHASE2,
   NFC_HCI_STOP_TYPE_A_ACT,
   NFC_HCI_SEND_A_READER_REQ,
   NFC_HCI_SEND_B_READER_REQ,
   NFC_HCI_SEND_EVT_READER_REQ,
   NFC_HCI_SEND_I_READER_REQ,
   NFC_HCI_SEND_J_READER_REQ,
   NFC_HCI_SEND_V_READER_REQ,
   NFC_HCI_M_A_U,
   NFC_HCI_M_B_P,
   NFC_HCI_M_F_C,
   NFC_HCI_M_I_T,
   NFC_HCI_M_J_T,
   NFC_HCI_M_R_I,
   NFC_HCI_R_A_U, 
   NFC_HCI_R_B_P, 
   NFC_HCI_R_F_C,
   NFC_HCI_R_I_T,
   NFC_HCI_R_J_T,
   NFC_HCI_R_R_I,
   NFC_HCI_C_A_U,
   NFC_HCI_C_B_P,
   NFC_HCI_C_F_C,
   NFC_HCI_C_I_T,
   NFC_HCI_C_J_T,
   NFC_HCI_C_R_I,
   NFC_HCI_WRITE_MEM,
   NFC_HCI_READ_MEM,
   NFC_HCI_CREATE_PIPE_SYSTEM_MGNT,
   NFC_HCI_OPEN_PIPE_SYSTEM_MGNT,
   NFC_HCI_TEST_SWP,
   NFC_HCI_SET_XTAL_WAIT_TIME1,
   NFC_HCI_SET_XTAL_WAIT_TIME2,
   NFC_HCI_NFPL_GET_EVT_HAND,
   NFC_HCI_NFPL_SET_EVT_HAND,
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
} GENERAL_HCI_CMD;

typedef struct 
{
  char cmd;                  /* Command id */
  char sz;                   /* Size of hci buffer */
  char hci[MAX_FRAME_LEN];   /* Up to NXP LLC(2bytes) + ETSI HCI(29 bytes) + CRC(2bytes) */
} general_hci_table;

const general_hci_table nfc_hci_tab[];

typedef struct hci_info *PNX_HCI_HANDLE;
typedef struct pnx_info *PNX_HANDLE;

typedef NFC_STATUS (*Nxp_Pnx_Verbose_Check)(PNX_HANDLE pNx);
typedef NFC_STATUS (*Nxp_Pnx_Hci_Trace)(PNX_HANDLE pNx,uint8_t data[], uint32_t size);
typedef int (*Nxp_Pnx_Print)(PNX_HANDLE pNx,const char *fmt, ...);
typedef NFC_STATUS (*Nxp_Pnx_Hci_Crc)(uint8_t* pData, uint8_t length, uint8_t* pCrc1, uint8_t* pCrc2);


typedef struct hci_info{
    uint8_t n_s;// Number of send frame
    uint8_t n_r;// Number of receive next frame
    uint16_t _ack;// ACK
    uint8_t frame_buf[MAX_FRAME_LEN];
    uint8_t frame_sz;
}hci_info_struct;

typedef struct pnx_info{
    int argc;// Argument index
    char** argv;// Argument body   
    bool_t hide;// Unreadable register
    uint32_t fp;// File point
    uint8_t verbose;// Whether to display the HCI trace
    uint16_t fw_version;// Store PNx chip frameware version
    uint8_t _started;// Whether to initialize successfully
    Nxp_Pnx_Hci_Crc hci_crc;
    Nxp_Pnx_Print pnx_print;
    Nxp_Pnx_Hci_Trace pnx_hci_trace;
    Nxp_Pnx_Verbose_Check pnx_verbose_check;
    PNX_HCI_HANDLE hci_private;
    // HCI input
    uint8_t *i_buf;
    uint8_t i_len;
    // HCI ouput
    uint8_t *r_buf;
    uint8_t *r_len;
}pnx_info_struct;

NFC_STATUS Pnx_Print(PNX_HANDLE pNx,const char *fmt, ...);  

NFC_STATUS Pnx_Open(PNX_HANDLE *pNx);
NFC_STATUS Pnx_Close(PNX_HANDLE pNx);
NFC_STATUS Pnx_Uart_Reset(PNX_HANDLE pNx,long level);

NFC_STATUS Pnx_Hci_Trace(PNX_HANDLE pNx,uint8_t data[], uint32_t size);
NFC_STATUS Pnx_Hci_Crc(uint8_t* pData, uint8_t length, uint8_t* pCrc1, uint8_t* pCrc2);
NFC_STATUS Pnx_Hci_Start(PNX_HANDLE pNx);
NFC_STATUS Pnx_Hci_Send(PNX_HANDLE pNx);

NFC_STATUS Pnx_Get_Register(PNX_HANDLE pNx,uint16_t reg, uint16_t *value);
NFC_STATUS Pnx_Set_Register(PNX_HANDLE pNx,uint16_t reg, uint16_t value);
NFC_STATUS Pnx_Get_Pvc(PNX_HANDLE pNx);
NFC_STATUS Pnx_Set_Chr(PNX_HANDLE pNx);
NFC_STATUS Pnx_Set_Ast(PNX_HANDLE pNx,uint8_t *i_buf,const uint8_t i_len);
NFC_STATUS Pnx_Set_Swp(PNX_HANDLE pNx,uint8_t sw);
NFC_STATUS Pnx_Set_Smx(PNX_HANDLE pNx,uint8_t sw);
NFC_STATUS Pnx_Set_Sst(PNX_HANDLE pNx,uint8_t *r_buf);
NFC_STATUS Pnx_Get_Tag(PNX_HANDLE pNx,uint8_t *r_buf);
NFC_STATUS Pnx_Set_Prb(PNX_HANDLE pNx, uint8_t tech, uint8_t rate);

NFC_STATUS Pnx_Verbose_Check(PNX_HANDLE pNx);
NFC_STATUS Pnx_Cmd_Handle(PNX_HANDLE pNx);


