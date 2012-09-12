//============================================================================
//  File Name: hci.c 
//
//  2012 Copyright (c), NXP Semiconductors. All rights reserved.
//
//  This source code is NXP Semiconductors N.V. proprietary and confidential
//  information
//
//  Description:
//      HCI(Host Controller Interface) data parser    
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
  
#include <pnx.h>

#define CRC1_CRC2               0x02 // CRC1 + CRC2
#define Length_CRC1_CRC2        0x03 // Length + CRC1 + CRC2
#define Chaining_Bit_Single     0x80 // 0x80 | pID
#define Chaining_Bit_Fragment   0x7F // 0x7F & pID
#define I_Frame_Mask            0x80
#define S_Frame_Mask            0xC0
#define U_Frame_Mask            0xE0

const general_hci_table nfc_hci_tab[] =
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
  /* PBRS test command */
  {NFC_HCI_SELF_TEST_PBRS,0x5,{0x80,0x82,0x25,0x03,0x0}},
  /* SWP On/Off test event */
  {NFC_HCI_CREATE_PIPE_SWP_MGNT,0x6,{0x80,0x81,0x10,0xA0,0x00,0xA0}},
  {NFC_HCI_OPEN_PIPE_SWP_MGNT,0x3,{0x80,0x82,0x03}},
  {NFC_EVT_SWP_SWITCH_MODE,0x5,{0x80,0x82,0x01,0x01,0x01}},

  /* SMX Wired/Virtual Test  S2C SE On(Wired Mode)*/
  {NFC_SMX_HCI_Open_Pipe_Admin,0x3,{0x80,0x81,0x03}},
  {NFC_SMX_HCI_Admin_ClearAllPipe,0x3,{0x80,0x81,0x14}},
  {NFC_SMX_HCI_ReOpen_Pipe_Admin,0x3,{0x80,0x81,0x03}},
  {NFC_SMX_HCI_LINK_MANAGMENT_GATE,0x3,{0x80,0x80,0x03}},
  {NFC_SMX_HCI_ADMINISTRATION_GATE,0x3,{0x80,0x81,0x03}},
  {NFC_HCI_CREATE_PIPE_SMX_MGNT,0x6,{0x80,0x81,0x10,0x20,0x00,0xA1}},
  {NFC_HCI_OPEN_PIPE_SMX_MGNT,0x3,{0x80,0x82,0x03}},
  {NFC_EVT_SMX_SWITCH_MODE,0x4,{0x80,0x82,0x43,0x00}},
  {NFC_PH_HCI_GATE_READER_A_GATE,0x6,{0x80,0x81,0x10,0x20,0x00,0x13}},
  {NFC_HCI_OPEN_READER_A_GATE,0x3,{0x80,0x83,0x03}},
  {NFC_HCI_GATE_PL_MNGT,0x6,{0x80,0x81,0x10,0x20,0x00,0x94}},
  {NFC_HCI_OPEN_GATE_PL_MNGT,0x3,{0x80,0x84,0x03}},
  {NFC_HCI_SET_RD_PHASE_1,0x5,{0x80,0x84,0x01,0x06,0x7f}},
  {NFC_HCI_SET_RD_PHASE_2,0x4,{0x80,0x84,0x02,0x06}},
  {NFC_EVT_READER_REQUESTED_1,0x3,{0x80,0x83,0x50}},
  {NFC_EVT_READER_REQUESTED_2,0x3,{0x80,0x83,0x75}},
  {NFC_HCI_READES_UID,0x4,{0x80,0x83,0x02,0x02}},

  {NFC_HCI_reads_Appl_Data,0x4,{0x80,0x83,0x02,0x05}},
  {NFC_HCI_reads_SAK,0x4,{0x80,0x83,0x02,0x03}},
  {NFC_HCI_reads_Fwi,0x4,{0x80,0x83,0x02,0x06}},
  {NFC_APDU_Select_Card_Manager_JCOP,17,{0x80,0x83,0x10,0x05,0x00,0xa4,0x04,0x00,0x07,0xa0,0x00,0x00,0x00,0x03,0x00,0x00,0x00}},
  {NFC_Get_CPLC_from_JCOP,0x9,{0x80,0x83,0x10,0x05,0x80,0xca,0x9f,0x7f,0x00}},
  {NFC_CLOSE_POLLING_SEND_EVT_END_OPERATION,0x3,{0x80,0x83,0x51}},
  /* SMX Wired/Virtual Test S2C SE On(Virtual Mode)*/ 
  {NFC_HCI_NFC_WI_GATE,0x6,{0x80,0x81,0x10,0x20,0x00,0xa1}},
  {NFC_HCI_OPEN_NFC_WI_GATE,0x3,{0x80,0x82,0x03}},
  {NFC_EVT_SE_Switch_virtual,0x4,{0x80,0x82,0x43,0x02}},

  /* Opens a pipe, byte two sets pipe nbr */
  {NFC_HCI_OPEN_PIPE,0x03,{0x80,0x80,0x03}},
  {NFC_HCI_CREATE_PIPE_READER_A,0x06,{0x80,0x81,0x10,0x13,0x00,0x13}},
  {NFC_HCI_CREATE_PIPE_READER_B,0x06,{0x80,0x81,0x10,0x20,0x00,0x11}},
  {NFC_HCI_CREATE_PIPE_READER_FELICA,0x06,{0x80,0x81,0x10,0x20,0x00,0x14}},
  {NFC_HCI_CREATE_PIPE_NFCINITIATOR,0x06,{0x80,0x81,0x10,0x20,0x00,0x30}},
  {NFC_HCI_SET_NFC_I_MODE_SPEED,0x05,{0x80,0x83,0x01,0x01,0x08}},
  {NFC_HCI_SET_NFC_NEXT_MODE_SPEED,0x05,{0x80,0x85,0x01,0x01,0x09}},
  {NFC_HCI_CREATE_PIPE_PL_MNGT,0x06,{0x80,0x81,0x10,0x20,0x00,0x94}},
  {NFC_HCI_CREATE_PIPE_READER_JEWEL,0x06,{0x80,0x81,0x10,0x15,0x00,0x15}},
  {NFC_HCI_CREATE_PIPE_READER_ISO15693,0x06,{0x80,0x81,0x10,0x12,0x00,0x12}},
  {NFC_HCI_SET_RD_PHASE,0x05,{0x80,0x81,0x01,0x06,0x7F}},
  {NFC_HCI_SET_RD_PHASE2,0x04,{0x80,0x81,0x02,0x06}},
  {NFC_HCI_STOP_TYPE_A_ACT,0x05,{0x80,0x82,0x01,0x10,0x00}},//Stop Activation after SAK received (not to jump into T=CL)
  {NFC_HCI_SEND_A_READER_REQ,0x03,{0x80,0x82,0x50}},// Type A
  {NFC_HCI_SEND_B_READER_REQ,0x03,{0x80,0x83,0x50}},// Type B
  {NFC_HCI_SEND_EVT_READER_REQ,0x03,{0x80,0x84,0x50}},// Felica
  {NFC_HCI_SEND_I_READER_REQ,0x03,{0x80,0x85,0x50}},// NFC-I
  {NFC_HCI_SEND_J_READER_REQ,0x03,{0x80,0x87,0x50}},// Jewel
  {NFC_HCI_SEND_V_READER_REQ,0x03,{0x80,0x88,0x50}},// ISO15693
  /*find out more*/
  {NFC_HCI_M_A_U,0x04,{0x80,0x82,0x02,0x02}},//82 02 02; // Type A UID
  {NFC_HCI_M_B_P,0x04,{0x80,0x83,0x02,0x04}},//83 02 04; // Type B PUPI
  {NFC_HCI_M_F_C,0x04,{0x80,0x84,0x02,0x01}},//84 02 01; // Felica System code
  {NFC_HCI_M_I_T,0x04,{0x80,0x85,0x02,0x0B}},//85 02 0B; // NFC-T NFCID3 
  {NFC_HCI_M_J_T,0x04,{0x80,0x87,0x02,0x00}},//87 20 00; // Jewel/Topaz (tag must be in the field to read out data)
  {NFC_HCI_M_R_I,0x04,{0x80,0x88,0x02,0x01}},//88 02 01; // ISO15693 read inventory
  /*restart polling */
  {NFC_HCI_R_A_U,0x03,{0x80,0x82,0x75}},//82 75; // Type A 
  {NFC_HCI_R_B_P,0x03,{0x80,0x83,0x75}},//83 75; // Type B 
  {NFC_HCI_R_F_C,0x03,{0x80,0x84,0x75}},//84 75; // Felica 
  {NFC_HCI_R_I_T,0x03,{0x80,0x85,0x75}},//85 75; // NFC-I 
  {NFC_HCI_R_J_T,0x03,{0x80,0x87,0x75}},//87 75; // Jewel/Topaz 
  {NFC_HCI_R_R_I,0x03,{0x80,0x88,0x75}},//88 75; // ISO15693
  /*TO CLOSE THE POLLING SEND EVT END OPERATION*/
  {NFC_HCI_C_A_U,0x03,{0x80,0x82,0x51}},//82 51;  // Type A
  {NFC_HCI_C_B_P,0x03,{0x80,0x83,0x51}},//83 51;  // Type B
  {NFC_HCI_C_F_C,0x03,{0x80,0x84,0x51}},//84 51;  // Felica
  {NFC_HCI_C_I_T,0x03,{0x80,0x85,0x51}},//85 51;  // NFC-I
  {NFC_HCI_C_J_T,0x03,{0x80,0x87,0x51}},//87 51;  // Jewel
  {NFC_HCI_C_R_I,0x03,{0x80,0x88,0x51}},//88 51;  // ISO15693

  /* Read/Write Mem (inc. register and eeprom) */
  {NFC_HCI_WRITE_MEM,0x7,{0x80,0x82,0x3F,0x00,0x00,0x00,0x00}},
  {NFC_HCI_READ_MEM,0x6,{0x80,0x82,0x3E,0x00,0x00,0x00}},
  {NFC_HCI_CREATE_PIPE_SYSTEM_MGNT,0x6,{0x80,0x81,0x10,0x90,0x00,0x90}},
  {NFC_HCI_OPEN_PIPE_SYSTEM_MGNT,0x3,{0x80,0x82,0x03}},
  {NFC_HCI_TEST_SWP,0x3,{0x80,0x82,0x21}},
  {NFC_HCI_SET_XTAL_WAIT_TIME1,0x7,{0x80,0x82,0x3f,0x00,0x9c,0x5c,0x07}},
  {NFC_HCI_SET_XTAL_WAIT_TIME2,0x7,{0x80,0x82,0x3f,0x00,0x9c,0x5d,0xcc}},
  /* NFPL Polling Loop related commands */
  {NFC_HCI_NFPL_GET_EVT_HAND,0x04,{0x80,0x81,0x02,0x02}},//Get Event handling
  {NFC_HCI_NFPL_SET_EVT_HAND,0x05,{0x80,0x81,0x01,0x02,0xff}},//Set Event Handling: All Events

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
  {NFC_HCI_NFPL_SEND_EVT_READER_REQ,0x03,{0x80,0x83,0x50}},//Host sends an Event "ReaderRequested"
  {NFC_HCI_NFPL_POST_EVENT_REQ_1,0x03,{0x80,0x83,0x04}},
  {NFC_HCI_NFPL_POST_EVENT_REQ_2,0x04,{0x80,0x81,0x11,0x03}},
  {NFC_HCI_NFPL_POST_EVENT_REQ_3,0x03,{0x80,0x82,0x04}},
  {NFC_HCI_NFPL_POST_EVENT_REQ_4,0x04,{0x80,0x81,0x11,0x02}}
};

NFC_STATUS Pnx_Hci_Trace(PNX_HANDLE pNx,uint8_t data[], uint32_t size)
{
   uint32_t i;

   if(size == 0 )
   return NFC_RES_ERR;

   for(i = 0; i < size; i++)

   {
      if((i % 10) == 0)
      pNx->pnx_print(pNx,"\n\t\t\t");
      pNx->pnx_print(pNx,"%02X ", data[i]);
   }
   pNx->pnx_print(pNx,"\n\tBlock size is: %d\n", size);
   pNx->pnx_print(pNx,"\r");
   return NFC_RES_OK;
}

NFC_STATUS Pnx_Hci_Crc(uint8_t* pData, uint8_t length, uint8_t* pCrc1, uint8_t* pCrc2)
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
  return NFC_RES_OK;
}

NFC_STATUS _i2c_write(PNX_HANDLE pNx,uint8_t * pBuffer, int nNbBytesToWrite)
{
    int ret;
    int numWrote = 0;

    //pNx->pnx_print(pNx,"_i2c_write() called to write %d bytes\n", nNbBytesToWrite);

    while (numWrote < nNbBytesToWrite) {
        ret = write(pNx->fp, pBuffer + numWrote, nNbBytesToWrite - numWrote);
        if (ret > 0) {
            //pNx->pnx_print(pNx,"wrote %d bytes", ret);
            numWrote += ret;
        } else if (ret == 0) {
            //pNx->pnx_print(pNx,"_i2c_write() EOF\n");
            return -1;
        } else {
            //pNx->pnx_print(pNx,"_i2c_write() errno=%d\n", errno);
            if (errno == EINTR || errno == EAGAIN) {
                continue;
            }
            return -1;
        }
    }
    return numWrote;
}

NFC_STATUS Pnx_Hci_Send(PNX_HANDLE pNx)
{
    int ret = 0;
    uint32_t i = 0;
    uint8_t len = pNx->i_len + Length_CRC1_CRC2;               
    uint8_t p_buf[MAX_FRAME_LEN] = {0};

    uint8_t frame_t = 0;
    char cframe = 0;

    uint8_t* p_crc1 = NULL;  
    uint8_t* p_crc2 = NULL;

    uint8_t local_n_s = 0; /* for check n_s of received frame from PN65 */
    

    if (pNx->_started)
    {
        if (pNx->i_len > MAX_FRAME_LEN - Length_CRC1_CRC2) 
        { 
            printf("Two many parameters");
            return NFC_RES_ERR;
        }

        /* Populate buffer to send. */
        p_buf[0] = pNx->i_len + CRC1_CRC2;        /* Adjust frame length i.e from header to crc */
        for (i=0;i<pNx->i_len;i++) 
        {
            p_buf[i+1] = *(pNx->i_buf+i);
        }

        /* Host frame type detection */
        frame_t = p_buf[1] & U_Frame_Mask; 
        switch (frame_t)
        {
            case U_Frame_Mask:
                cframe = 'U';
                break;
            case S_Frame_Mask:
                cframe = 'S'; 
                /* Adjust frame counters. To be added if needed */
                break;
            default:
                cframe='I';
                /* Adjust frame counters. */
                p_buf[1] = frame_t | pNx->hci_private->n_r | (pNx->hci_private->n_s << 3);//n_r occupy 0,1,2;n_s:occupy 3,4,5
                //pNx->pnx_print(pNx,"##before##NS[%d],NR[%d],frame_t[0x%02x]\n",n_s,n_r,frame_t);
                pNx->hci_private->n_s++;//+1
                pNx->hci_private->n_s &= 7;//reduce high bit,only 0~7
                pNx->hci_private->n_r = (pNx->hci_private->n_s + 8 - pNx->hci_private->_ack) % 8;//(1+8-0)%8=1
                //pNx->pnx_print(pNx,"##after##NS[%d],NR[%d]\n",n_s,n_r);
                break;
        }

        /* Add CRCs */
        p_crc1 = &p_buf[pNx->i_len+1];
        p_crc2 = &p_buf[pNx->i_len+2];
        pNx->hci_crc((uint8_t*)&p_buf,pNx->i_len+1,p_crc1,p_crc2);//+1:include length

        /* Debug print */
        pNx->pnx_print(pNx,"AP->NFC(%c-frame):",cframe);
        pNx->pnx_hci_trace(pNx,p_buf, len);

        /* Send request */
        //if ((ret = (write(pNx->fp,&p_buf[0],len)))<0)
        if ((ret = (_i2c_write(pNx,&p_buf[0],len)))<0)
        { 
            printf("pnx write error retcode = %d, errno = %d\n", ret, errno); 
            return NFC_RES_ERR;
        }
        usleep(3000);

        /* Read no bytes available on I2C*/
        if ((ret = (read(pNx->fp,pNx->r_buf,1))) < 0)          
    	{             
    		printf("pnx read error retcode = %d, errno = %d\n", ret, errno);             
    		*pNx->r_len=0;  /* Clear the length to avoid dcp_print crash */             
    		return NFC_RES_ERR;         
    	}
        
        *pNx->r_len =*pNx->r_buf;

        if(*pNx->r_len >= MAX_FRAME_LEN)
        {
            *pNx->r_len=MAX_FRAME_LEN;
            *pNx->r_buf=MAX_FRAME_LEN;
        }

        /* Read *p_resplen bytes from I2C */
        if ((ret = (read(pNx->fp, pNx->r_buf+1, *pNx->r_len))) < 0) 
        { 
            printf("pnx read error retcode = %d, errno = %d\n", ret, errno);
            return NFC_RES_ERR;
        }

        /* Slave frame type detection  */
        frame_t=*(pNx->r_buf+1) & U_Frame_Mask;
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
        pNx->pnx_print(pNx,"NFC->AP(%c-frame):",cframe);
        pNx->pnx_hci_trace(pNx,pNx->r_buf, *pNx->r_len+1);

        if (cframe=='I') 
        {
            /* check N(r) of I/S frame */
            local_n_s = (*(pNx->r_buf+1) & 0x38) >> 3;//local_n_s: 0x38->5,4,3 >>3
            //pNx->pnx_print(pNx,"Expected N_R is %d\n", (n_r+7)%8);//
            //pNx->pnx_print(pNx,"Recieved N_R is %d\n", local_n_s);//
            if ((pNx->hci_private->n_r+7)%8 != local_n_s) //Expected != Recieved
            {   
                /* Read no bytes available on I2C */
                if ((ret = read(pNx->fp,pNx->r_buf,1)) < 0) 
                {
                    printf("Error: i2c_read returned %d.",ret);
                    *pNx->r_len=0;  /* Clear the length to avoid dcp_print crash */
                    return NFC_RES_ERR;
                }
                *pNx->r_len = *pNx->r_buf;

                if(*pNx->r_len >= MAX_FRAME_LEN)
                {
                    *pNx->r_len = MAX_FRAME_LEN;
                    *pNx->r_buf = MAX_FRAME_LEN;
                }
                /* Read data from I2C */
                if (read(pNx->fp,pNx->r_buf+1,*pNx->r_len) < 0)
                {
                    printf("nfc_handle_frame: I2C error, could not fetch %d bytes",*pNx->r_len);
                    return NFC_RES_ERR;
                }

                /* Slave frame type detection  */
                frame_t = *(pNx->r_buf + 1) & U_Frame_Mask;
                switch (frame_t)
                {
                    case U_Frame_Mask:
                        cframe = 'U';
                        break;
                    case S_Frame_Mask:
                        cframe = 'S';
                        break;
                    default:
                        cframe = 'I';
                        break;
                }

                // Debug print 
                pNx->pnx_print(pNx,"NFC->AP(%c-frame):",cframe);
                pNx->pnx_hci_trace(pNx,pNx->r_buf,*pNx->r_len + 1);
            }
            else
            {
                /* Return ACK to slave */
                len = 4;

                p_buf[0] = 3;//Length=3:Only for Length+Header+CRC1+CRC2
                p_buf[1] = S_Frame_Mask | pNx->hci_private->n_r;//0xC1

                /* Add CRCs */
                uint8_t* p_crc1 = NULL;  
                uint8_t* p_crc2 = NULL;
                p_crc1 = &p_buf[2];
                p_crc2 = &p_buf[3];
                pNx->hci_crc((uint8_t*)&p_buf,2,p_crc1,p_crc2);

                /* Debug print */
                pNx->pnx_print(pNx,"AUTO ACK AP->NFC:");
                pNx->pnx_hci_trace(pNx,p_buf,len);
                /* Send request */
                //if ((ret = (write(pNx->fp,&p_buf[0],len)))<0)
                if ((ret = (_i2c_write(pNx,&p_buf[0],len)))<0)
                { 
                    printf("pnx write error retcode = %d, errno = %d\n", ret, errno); 
                    return NFC_RES_ERR;
                }
                usleep(3000);
                return NFC_RES_OK;
            }
        }

        /* Event request are acknowleeged by an S-Frame  */
        /* Wait until the event is fired */
        if (cframe=='S') 
        {
            if((pNx->i_buf[1] == 0x82 && pNx->i_buf[2] == 0x43 && pNx->i_buf[3] == 0x00) \
              || (pNx->i_buf[1] == 0x82 && pNx->i_buf[2] == 0x43 && pNx->i_buf[3] == 0x02) \
              /*|| (pNx->i_buf[1] == 0x83 && pNx->i_buf[2] == 0x50 ) \
              || (pNx->i_buf[1] == 0x83 && pNx->i_buf[2] == 0x75)*/)
            {
                pNx->hci_private->_ack = (pNx->hci_private->_ack + 1) % 8;
                pNx->hci_private->n_r = (pNx->hci_private->n_r + 8 - pNx->hci_private->_ack) % 8;
                pNx->pnx_print(pNx,"Occured Events\n");
                /* no card detect event is taken as ok, so return ok */
                return NFC_RES_OK;
            }
            else 
            {
                /* Read no bytes available on I2C */
                if ((ret = read(pNx->fp,pNx->r_buf,1)) < 0) 
                {
                    printf("Error: i2c_read returned %d.",ret);
                    *pNx->r_len = 0;  /* Clear the length to avoid dcp_print crash */
                    return NFC_RES_ERR;
                }
                *pNx->r_len = *pNx->r_buf;

                if(*pNx->r_len >= MAX_FRAME_LEN)
                {
                    *pNx->r_len = MAX_FRAME_LEN;
                    *pNx->r_buf = MAX_FRAME_LEN;
                }
                /* Read data from I2C */
                if (read(pNx->fp,pNx->r_buf+1,*pNx->r_len) < 0)
                {
                    printf("nfc_handle_frame: I2C error, could not fetch %d bytes",*pNx->r_len);
                    return NFC_RES_ERR;
                }

                /* Slave frame type detection  */
                frame_t = *(pNx->r_buf + 1) & U_Frame_Mask;
                switch (frame_t)
                {
                    case U_Frame_Mask:
                        cframe = 'U';
                        break;
                    case S_Frame_Mask:
                        cframe = 'S';
                        break;
                    default:
                        cframe = 'I';
                        break;
                }
                /* Debug print */
                pNx->pnx_print(pNx,"NFC->AP(%c-frame):",cframe);
                pNx->pnx_hci_trace(pNx,pNx->r_buf,*pNx->r_len+1);
            }
        }

        /* Check I-Frame again */
        if (cframe == 'I') 
        {
            /* check N(r) of I/S frame */
            local_n_s = (*(pNx->r_buf + 1) & 0x38) >> 3;
            //pNx->pnx_print(pNx,"Expected N_R is %d\n", (n_r+7)%8);
            //pNx->pnx_print(pNx,"Recieved N_R is %d\n", local_n_s);
            if ((pNx->hci_private->n_r + 7)%8 != local_n_s) 
            {
                /* Read no bytes available on I2C */
                if ((ret = read(pNx->fp,pNx->r_buf,1)) < 0) 
                {
                    printf("Error: i2c_read returned %d.",ret);
                    *pNx->r_len = 0;  /* Clear the length to avoid dcp_print crash */
                    return NFC_RES_ERR;
                }

                *pNx->r_len = *pNx->r_buf;

                if(*pNx->r_len >= MAX_FRAME_LEN)
                {
                    *pNx->r_len = MAX_FRAME_LEN;
                    *pNx->r_buf = MAX_FRAME_LEN;
                }

                /* Read data from I2C */
                if (read(pNx->fp,pNx->r_buf + 1,*pNx->r_len) < 0)
                {
                    printf("nfc_handle_frame: I2C error, could not fetch %d bytes",*pNx->r_len);
                    return NFC_RES_ERR;
                }

                /* Debug print */
                pNx->pnx_print(pNx,"NFC->AP(%c-frame):",cframe);
                pNx->pnx_hci_trace(pNx,pNx->r_buf, *pNx->r_len+1);

                local_n_s = (*(pNx->r_buf + 1) & 0x38) >> 3;
                //pNx->pnx_print(pNx,"Expected N_R is %d\n", (n_r+7)%8);
                //pNx->pnx_print(pNx,"Recieved N_R is %d\n", local_n_s);
            }
            /* Return ACK to slave */
            len = 4;

            p_buf[0] = 3;
            p_buf[1] = 0xC0 | pNx->hci_private->n_r;

            /* Add CRCs */
            p_crc1 = &p_buf[2];
            p_crc2 = &p_buf[3];
            pNx->hci_crc((uint8_t*)&p_buf,2,p_crc1,p_crc2);

            /* Debug print */
            pNx->pnx_print(pNx,"AUTO ACK AP->NFC:");
            pNx->pnx_hci_trace(pNx,p_buf,len);

            /* Send request */
            //if ((ret = (write(pNx->fp, &p_buf[0], len)))<0)
            if ((ret = (_i2c_write(pNx,&p_buf[0],len)))<0)
            { 
                printf("pnx write error retcode = %d, errno = %d\n", ret, errno); 
                return NFC_RES_ERR;
            }            
            usleep(3000);

        }
        return NFC_RES_OK;
    }
    pNx->pnx_print(pNx,"NFC is not yet started.\n");
    return NFC_RES_ERR;
}

NFC_STATUS Pnx_Hci_Dispatch(PNX_HANDLE pNx,uint8_t cmd,uint8_t pipe_id,uint8_t *r_buf,uint8_t *r_len)
{ 
    NFC_STATUS res=NFC_RES_ERR;
    general_hci_table* p_hci = NULL;
    uint8_t hci_buf[MAX_FRAME_LEN]={0};

    p_hci=(general_hci_table*)&nfc_hci_tab[cmd];
    memcpy(&hci_buf[0],&p_hci->hci,p_hci->sz);
    if (pipe_id >0) 
    {
        hci_buf[1]= Chaining_Bit_Single | pipe_id;
    }
    pNx->i_buf = hci_buf;
    pNx->i_len = p_hci->sz;
    pNx->r_buf = r_buf;
    pNx->r_len = r_len;
    res=Pnx_Hci_Send(pNx);
    return res;
}

NFC_STATUS Pnx_Hci_Start(PNX_HANDLE pNx)
{  
  if(!pNx->_started)
  { 
    ioctl(pNx->fp, PN544_SET_PWR, 1);
    ioctl(pNx->fp, PN544_SET_PWR, 0);
    ioctl(pNx->fp, PN544_SET_PWR, 1);
    Pnx_Uart_Reset(pNx,1);
    Pnx_Uart_Reset(pNx,0);
    Pnx_Uart_Reset(pNx,1);
    ioctl(pNx->fp, PN544_SET_FW_MODE, 0);
    ioctl(pNx->fp, PN544_SET_FW_MODE, 1);
    ioctl(pNx->fp, PN544_SET_FW_MODE, 0);  

    pNx->_started = TRUE;
    pNx->hci_private->n_r = 0x0;
    pNx->hci_private->n_s = 0x0;
    pNx->hci_private->_ack = 0x0;
    memset(pNx->hci_private->frame_buf,0,sizeof(pNx->hci_private->frame_buf));
    pNx->hci_private->frame_sz = 0x0;
    pNx->i_buf = NULL;
    pNx->i_len = 0x0;
    pNx->r_buf = NULL;
    pNx->r_len = 0x0;
    if(Pnx_Hci_Dispatch(pNx,NFC_HCI_U_RSET,0,(uint8_t*)&pNx->hci_private->frame_buf,&pNx->hci_private->frame_sz) == NFC_RES_ERR) return NFC_RES_ERR; /* Send U-RSET */
    if(Pnx_Hci_Dispatch(pNx,NFC_HCI_OPEN_PIPE_ADMIN,0,(uint8_t*)&pNx->hci_private->frame_buf,&pNx->hci_private->frame_sz) == NFC_RES_ERR) return NFC_RES_ERR;   /* Open Administration pipe */
    if(Pnx_Hci_Dispatch(pNx,NFC_HCI_CLEARALLPIPES,0,(uint8_t*)&pNx->hci_private->frame_buf,&pNx->hci_private->frame_sz) == NFC_RES_ERR)  return NFC_RES_ERR;     /* Clear all pipes */
    if(Pnx_Hci_Dispatch(pNx,NFC_HCI_OPEN_PIPE_ADMIN,0,(uint8_t*)&pNx->hci_private->frame_buf,&pNx->hci_private->frame_sz) == NFC_RES_ERR) return NFC_RES_ERR;   /* Open Administration pipe */
    if(Pnx_Hci_Dispatch(pNx,NFC_HCI_CREATE_PIPE_LINK_MGNT,0,(uint8_t*)&pNx->hci_private->frame_buf,&pNx->hci_private->frame_sz) == NFC_RES_ERR) return NFC_RES_ERR;  /* Create link management pipe */
    if(Pnx_Hci_Dispatch(pNx,NFC_HCI_OPEN_PIPE_LINK_MGNT,0,(uint8_t*)&pNx->hci_private->frame_buf,&pNx->hci_private->frame_sz) == NFC_RES_ERR) return NFC_RES_ERR;    /* Open link management pipe */
    return NFC_RES_OK;
  }
  pNx->pnx_print(pNx,"NFC is already started\n");
  return NFC_RES_ERR;
}


