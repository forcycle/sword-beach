//============================================================================
//  File Name: opt.c 
//
//  2012 Copyright (c), NXP Semiconductors. All rights reserved.
//
//  This source code is NXP Semiconductors N.V. proprietary and confidential
//  information
//
//  Description:
//           
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

NFC_STATUS Pnx_Get_Register(PNX_HANDLE pNx,uint16_t reg, uint16_t *value)
{
    NFC_STATUS res = NFC_RES_ERR;
    general_hci_table* p_hci = NULL;
    uint8_t hci_buf[MAX_FRAME_LEN] = {0};
    uint8_t hci_resp[MAX_FRAME_LEN] = {0};
    uint8_t hci_resplen = 0;

    Pnx_Hci_Start(pNx);
    p_hci = (general_hci_table*)&nfc_hci_tab[NFC_HCI_READ_MEM];
    memcpy(hci_buf, &p_hci->hci, p_hci->sz);
    hci_buf[4]=(reg>>8);//0x9810:98
    hci_buf[5]=(reg&0xFF);//0x9810:10
    pNx->i_buf = hci_buf;
    pNx->i_len = p_hci->sz;
    pNx->r_buf = hci_resp;
    pNx->r_len = &hci_resplen;
    res = Pnx_Hci_Send(pNx);
    if(res != NFC_RES_OK)
    {
        printf("set reg failed.\n");
        return NFC_RES_ERR;
    }

    if(hci_resplen < 5)  //Some register will return NULL 
    {
        printf("wrong resp long from get reg.\n");
        return NFC_RES_ERR;
    }
    /*06 81 82 80 00 A2 F0 // Unpacked data = [82 80 00 ]*/
    *value = hci_resp[4];

    if(hci_resplen <= 5)
    {
        pNx->hide = TRUE;
    }
    return NFC_RES_OK;
}

NFC_STATUS Pnx_Set_Register(PNX_HANDLE pNx,uint16_t reg, uint16_t value)
{
    NFC_STATUS res = NFC_RES_ERR;
    general_hci_table* p_hci = NULL;
    uint8_t hci_buf[MAX_FRAME_LEN] = {0};
    uint8_t hci_resp[MAX_FRAME_LEN] = {0};
    uint8_t hci_resplen = 0;

    Pnx_Hci_Start(pNx);
    p_hci = (general_hci_table*)&nfc_hci_tab[NFC_HCI_WRITE_MEM];
    memcpy(hci_buf, &p_hci->hci, p_hci->sz);
    hci_buf[4]=(reg>>8);
    hci_buf[5]=(reg&0xFF);
    hci_buf[6]=value;
    pNx->i_buf = hci_buf;
    pNx->i_len = p_hci->sz;
    pNx->r_buf = hci_resp;
    pNx->r_len = &hci_resplen;

    res = Pnx_Hci_Send(pNx);
    if(res != NFC_RES_OK)
    {
        printf("set reg failed.\n");
        return NFC_RES_ERR;
    }

    return NFC_RES_OK;
}

NFC_STATUS Pnx_Get_Pvc(PNX_HANDLE pNx)
{
    NFC_STATUS res = NFC_RES_ERR;
    int i =0;
    uint32_t temp = 0;
    uint8_t r_buf[MAX_FRAME_LEN] = {0};
    uint8_t r_len = 0;

    Pnx_Hci_Start(pNx);
    /* Create ID mgnt pipe */
    res=Pnx_Hci_Dispatch(pNx,NFC_HCI_CREATE_PIPE_ID_MGNT,0,(uint8_t*)&pNx->hci_private->frame_buf,&pNx->hci_private->frame_sz);
    res=Pnx_Hci_Dispatch(pNx,NFC_HCI_OPEN_PIPE_ID_MGNT,pNx->hci_private->frame_buf[8],(uint8_t*)&pNx->hci_private->frame_buf,&pNx->hci_private->frame_sz);
    /* VERSION_SW */
    res=Pnx_Hci_Dispatch(pNx,NFC_HCI_VERSION_SW,pNx->hci_private->frame_buf[8],r_buf,&r_len);
    printf("\nPNx_VERSION_SW:");
    for(i = 0; i < r_len - 5; i++)
    {
        if((i % 10) == 0)
        {
            printf("\n\t\t");
        }
        printf("%02X ", r_buf[4+i]);
        ((uint8_t*)&temp)[i] = r_buf[4+i];
    }
    printf("\n\t\t--0x%02x  FlashLib Minor",((uint8_t*)&temp)[2]);
    printf("\n\t\t--0x%02x  FlashLib Major",((uint8_t*)&temp)[1]);
    printf("\n\t\t--0x%02x  Patch",((uint8_t*)&temp)[0] & 0x0f);
    printf("\n\t\t--0x%02x  RomLib",(((uint8_t*)&temp)[0] >> 4) & 0x0f);
    printf("\n\t\t--%d.%d  Build Firmware version",((uint8_t*)&temp)[1],((uint8_t*)&temp)[2]);
    pNx->fw_version= ((uint8_t*)&temp)[1];
    /* HCI_VERSION */
    res=Pnx_Hci_Dispatch(pNx,NFC_HCI_HCI_VERSION,pNx->hci_private->frame_buf[8],r_buf,&r_len);
    printf("\nPNx_HCI_VERSION:");
    for(i = 0; i < r_len - 5; i++)

    {
        if((i % 10) == 0)
        {
            printf("\n\t\t");
        }
        printf("%02X ",r_buf[4+i]);
    }
    printf("\n\t\t--0x%02x  Supported HCI version",r_buf[4]);
    /* VERSION_HARD */
    res=Pnx_Hci_Dispatch(pNx,NFC_HCI_VERSION_HARD,pNx->hci_private->frame_buf[8],r_buf,&r_len);
    printf("\nPNx_VERSION_HARD:");
    for(i = 0; i < r_len - 5; i++)
    {
        if((i % 10) == 0)
        {
            printf("\n\t\t");
        }
        printf("%02X ", r_buf[4+i]);
        ((uint8_t*)&temp)[i] = r_buf[4+i];
    }
    printf("\n\t\t--0x%02x  BSID version",((uint8_t*)&temp)[2]);
    printf("\n\t\t--0x%02x  Software",((uint8_t*)&temp)[1] & 0x3f);
    printf("\n\t\t--0x%02x  #MPW",(((uint8_t*)&temp)[1] >> 6) & 0x03);
    printf("\n\t\t--0x%02x  HW version",((uint8_t*)&temp)[0] & 0x1f);
    printf("\n\t\t--0x%02x  Derivate",(((uint8_t*)&temp)[0] >> 5) & 0x07);
    /* VENDOR_NAME */
    res=Pnx_Hci_Dispatch(pNx,NFC_HCI_VENDOR_NAME,pNx->hci_private->frame_buf[8],r_buf,&r_len);
    printf("\nPNx_VENDOR_NAME:");
    for(i = 0; i < r_len - 5; i++)
    {
        if((i % 10) == 0)
        {
            printf("\n\t\t");
        }
        printf("%02X ", r_buf[4+i]);
    }

    for(i = 0; i < r_len - 5; i++)
    {
        printf("\n\t\t--0x%02X  %c", r_buf[4+i],r_buf[4+i]);
    }

    /* MODEL_ID */
    res=Pnx_Hci_Dispatch(pNx,NFC_HCI_MODEL_ID,pNx->hci_private->frame_buf[8],r_buf,&r_len);
    printf("\nPNx_MODEL_ID:");
    for(i = 0; i < r_len - 5; i++)
    {
        if((i % 10) == 0)
        {
            printf("\n\t\t");
        }
        printf("%02X ", r_buf[4+i]);
    }
    printf("\n\t\t--0x%02x  PNx(standalone) Model ID",r_buf[4]);
    /* GATES_LIST */
    res=Pnx_Hci_Dispatch(pNx,NFC_HCI_GATES_LIST,pNx->hci_private->frame_buf[8],r_buf,&r_len);
    printf("\nPNx_GATES_LIST:");
    for(i = 0; i < r_len - 5; i++)  
    {
        if((i % 10) == 0)
        {
            printf("\n\t\t");
        }
        printf("%02X ", r_buf[4+i]);
    }
    res=Pnx_Hci_Dispatch(pNx,NFC_HCI_CLEARALLPIPES,0,(uint8_t*)&pNx->hci_private->frame_buf,&pNx->hci_private->frame_sz);     /* Clear all pipes */

    printf("\n");

    return NFC_RES_OK;
}


NFC_STATUS Pnx_Set_Chr(PNX_HANDLE pNx)
{
    int i =0;
    int ret = 0;
    static const unsigned char UA[] = {0x03, 0xE6, 0x17, 0xA7,};

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
        Pnx_Hci_Dispatch(pNx,NFC_HCI_U_RSET,0,(uint8_t*)&pNx->hci_private->frame_buf,&pNx->hci_private->frame_sz); /* Send U-RSET */
        printf("\nResponse UA-Frame:");
        for(i = 0; i < pNx->hci_private->frame_sz; i++)  
        {
            if((i % 10) == 0)
            {
                printf("\n\t\t");
            }
            printf("%02X ", pNx->hci_private->frame_buf[i]);
        }
        if(memcmp(pNx->hci_private->frame_buf,UA,4) == 0)
        {
            printf("\n\t\t--0x%02x  PNx chip hard reset succeed\n",pNx->hci_private->frame_buf[1]);
        }
        else
        {
            printf("\n\t\t--0x%02x  PNx chip hard reset fail\n",pNx->hci_private->frame_buf[1]);
        }    
        return NFC_RES_OK;
    }
    pNx->pnx_print(pNx,"NFC is already started.\n");
    return NFC_RES_ERR;
}

NFC_STATUS Pnx_Set_Ast(PNX_HANDLE pNx,uint8_t *i_buf,const uint8_t i_len)
{
    int i = 0;
    NFC_STATUS res = NFC_RES_ERR;
    general_hci_table* p_hci = NULL;
    uint8_t hci_buf[MAX_FRAME_LEN] = {0};
    uint8_t hci_resp[MAX_FRAME_LEN] = {0};
    uint8_t hci_resplen = 0;
    uint8_t threshold_loop1 = 0x03;
    uint8_t threshold_loop2 = 0x05;
    uint8_t threshold_loop3 = 0x0E;
    uint8_t tolerance_loop1 = 0x11;
    uint8_t tolerance_loop2 = 0x11;
    uint8_t tolerance_loop3 = 0x03;
    uint32_t temp = 0;
    uint8_t buffer[MAX_FRAME_LEN] = {0};

    Pnx_Get_Pvc(pNx);
    if(pNx->fw_version > 108)
    {
        printf("Firmware %d does not support antenna self test\n",pNx->fw_version);
        return NFC_RES_ERR;
    }
    pNx->_started = FALSE;
    Pnx_Hci_Start(pNx);
    pNx->r_buf = hci_resp;
    pNx->r_len = &hci_resplen;
    switch (i_len)
    {
        case 6:
        tolerance_loop3 = i_buf[5];
        case 5:
        tolerance_loop2 = i_buf[4];
        case 4:
        tolerance_loop1 = i_buf[3];
        case 3:
        threshold_loop3 = i_buf[2];
        case 2:
        threshold_loop2 = i_buf[1];
        case 1:
        threshold_loop1 = i_buf[0];
        break;
        case 0:
        break;
        default:
        return NFC_RES_SYNTAX;
        break;
    }

    /* Timing settings loop 1 & 2 & 3*/
    p_hci = (general_hci_table*)&nfc_hci_tab[NFC_HCI_SET_TIMING];
    memcpy(hci_buf, &p_hci->hci, p_hci->sz);
    /* Timing registrys are located in registies 9CA0 to 9CA8 */
    for(i = 0;i < 9;i++)
    {
        hci_buf[5] = 0xA0 + i;
        if(i == 7)
        hci_buf[6]=0x07;
        else if(i == 8)
        hci_buf[6] = 0x01;
        else
        hci_buf[6] = 0x10;
        pNx->i_buf = hci_buf;
        pNx->i_len = p_hci->sz;
        res = Pnx_Hci_Send(pNx);
        if(res != NFC_RES_OK)
        {
            printf("Timing settings failed.\n");
            return NFC_RES_ERR;
        }
    }

    /* Write loop 1 config */
    p_hci = (general_hci_table*)&nfc_hci_tab[NFC_HCI_SET_AST_GSP1];
    memcpy(hci_buf, &p_hci->hci, p_hci->sz);
    pNx->i_buf = hci_buf;
    pNx->i_len = p_hci->sz;
    res = Pnx_Hci_Send(pNx);
    if(res != NFC_RES_OK)
    {
        printf("Setting AST-GSP1 failed\n");
        return NFC_RES_ERR;
    }

    /* Write loop 2 config */
    p_hci = (general_hci_table*)&nfc_hci_tab[NFC_HCI_SET_AST_GSP2];
    memcpy(hci_buf, &p_hci->hci, p_hci->sz);
    pNx->i_buf = hci_buf;
    pNx->i_len = p_hci->sz;
    res = Pnx_Hci_Send(pNx);
    if(res != NFC_RES_OK)
    {
        printf("Setting AST-GSP2 failed\n");
        return NFC_RES_ERR;
    }

    /* Write loop 3 config */
    p_hci = (general_hci_table*)&nfc_hci_tab[NFC_HCI_SET_AST_CURRENT];
    memcpy(hci_buf, &p_hci->hci, p_hci->sz);
    pNx->i_buf = hci_buf;
    pNx->i_len = p_hci->sz;
    res = Pnx_Hci_Send(pNx);
    if(res != NFC_RES_OK)
    {
        printf("Setting AST-CURRENT failed\n");
        return NFC_RES_ERR;
    }

    /* Write tolerance loop 1 & 2 & 3*/
    p_hci = (general_hci_table*)&nfc_hci_tab[NFC_HCI_SET_TOLERANCE];
    memcpy(hci_buf, &p_hci->hci, p_hci->sz);

    for(i = 0;i < 3;i++)
    {
        hci_buf[5] = 0xF5 + i;
        if (i == 2)
        hci_buf[6] = tolerance_loop3;
        else if (i == 1)
        hci_buf[6] = tolerance_loop2;
        else
        hci_buf[6] = tolerance_loop1;
        pNx->i_buf = hci_buf;
        pNx->i_len = p_hci->sz;
        res = Pnx_Hci_Send(pNx);
        if(res != NFC_RES_OK)
        {
            printf("Tolerance settings failed.\n");
            return NFC_RES_ERR;
        }
    }

    p_hci = (general_hci_table*)&nfc_hci_tab[NFC_HCI_SELF_TEST_ANTENNA];
    memcpy(hci_buf, &p_hci->hci, p_hci->sz);
    hci_buf[3] = threshold_loop1;
    hci_buf[4] = threshold_loop2;
    hci_buf[5] = threshold_loop3;
    pNx->i_buf = hci_buf;
    pNx->i_len = p_hci->sz;
    res = Pnx_Hci_Send(pNx);
    if(res != NFC_RES_OK)
    {
        printf("Self test command failed\n\n");
        return NFC_RES_ERR;
    }
    if(hci_resplen < 9)
    {
        printf("Wrong resplen from Antenna selftest(%d)\n",hci_resplen);
        return NFC_RES_ERR;
    }

    //memcpy(r_buf, hci_resp + 4, 4);
    //*r_len = 4;
    printf("\nAntenna self test response:");
    for(i = 0; i < hci_resplen - 5; i++)
    {
        if((i % 10) == 0)
        {
            printf("\n\t\t\t");
        }
        printf("%02X ", hci_resp[4+i]);
        //((uint8_t*)&temp)[i] = hci_resp[4+i];
    }
    if(0x00 == hci_resp[4])
    {
        printf("\n\t\t\t--0x%02x  No failure",hci_resp[4]);
    }
    else if(0x79 == hci_resp[4])
    {
        printf("\n\t\t\t--0x%02x  1st condition failed (RF sensitivity on TX1),",hci_resp[4]);
        printf("\n\t\t\t        but does not have any signification for");
        printf("\n\t\t\t        pass/fail of 2nd and 3 rd conditions");
    } 
    else if(0x7a == hci_resp[4])
    {
        printf("\n\t\t\t--0x%02x  1st condition passed, 2nd condition failed ",hci_resp[4]);
        printf("\n\t\t\t        (RF sensitivity on TX2), but does not have any");
        printf("\n\t\t\t        signification for pass/fail of 3 rd condition");
    } 
    else if(0x7b == hci_resp[4])
    {
        printf("\n\t\t\t--0x%02x  1st and 2nd conditions passed, 3rd condition",hci_resp[4]);
        printf("\n\t\t\t        failed (Current detection level on TX1 & TX2)");
    } 
    else
    {
        printf("\n\t\t\t--0x    Unknow error");
    }

    printf("\n\t\t\t--0x%02x  Measured Values",hci_resp[5]);
    printf("\n\t\t\t--0x%02x  Measured Values",hci_resp[6]);
    printf("\n\t\t\t--0x%02x  Measured Values",hci_resp[7]);
    printf("\n\t\t\t--0x%02x  Measured Values\n",hci_resp[8]);

    return NFC_RES_OK;
}

NFC_STATUS Pnx_Set_Sst(PNX_HANDLE pNx,uint8_t *r_buf)
{
    int i =0;
    NFC_STATUS res = NFC_RES_ERR;
    general_hci_table* p_hci = NULL;
    uint8_t hci_buf[MAX_FRAME_LEN] = {0};
    uint8_t hci_resp[MAX_FRAME_LEN]= {0};
    uint8_t hci_resplen = 0;

    Pnx_Hci_Start(pNx);
    pNx->r_buf = hci_resp;
    pNx->r_len = &hci_resplen;
    p_hci = (general_hci_table*)&nfc_hci_tab[NFC_HCI_TEST_SWP];
    memcpy(hci_buf, &p_hci->hci, p_hci->sz);
    pNx->i_buf = hci_buf;
    pNx->i_len = p_hci->sz;
    res = Pnx_Hci_Send(pNx);
    if(res != NFC_RES_OK)
    {
        printf("swp test command failed\n");
        return NFC_RES_ERR;
    }

    if(hci_resplen < 7)
    {
        printf("Wrong resplen from swp selftest\n");
        return NFC_RES_ERR;
    }

    printf("\nSWP self test response:");
    for(i = 0; i < hci_resplen - 5; i++)
    {
        if((i % 10) == 0)
        {
            printf("\n\t\t\t");
        }
        printf("%02X ", hci_resp[4+i]);
        //((uint8_t*)&temp)[i] = hci_resp[4+i];
    }
    if(0x01 == hci_resp[4])
    {
        printf("\n\t\t\t--0x%02x  SWP self test failed",hci_resp[4]);
    }
    if(0x00 == hci_resp[4])
    {
        printf("\n\t\t\t--0x%02x  SWP self test no failure",hci_resp[4]);
    } 
    if(0x01 == hci_resp[5])
    {
        printf("\n\t\t\t--0x%02x  PmuVcc present",hci_resp[5]);
    } 
    if(0x00 == hci_resp[5])
    {
        printf("\n\t\t\t--0x%02x  PmuVcc not present",hci_resp[5]);
    } 
    if(hci_resp[4] > 0x01 || hci_resp[5] > 0x01)
    {
        printf("\n\t\t\t--0x    Unknow error");
    }
    printf("\n");
    return NFC_RES_OK;
}

NFC_STATUS Pnx_Get_Tag(PNX_HANDLE pNx,uint8_t *r_buf)
{
    int i =0;
    NFC_STATUS res = NFC_RES_ERR;
    general_hci_table* p_hci;
    uint8_t hci_buf[MAX_FRAME_LEN];
    uint8_t hci_resp[MAX_FRAME_LEN];
    uint8_t hci_resplen = 0;

    Pnx_Hci_Start(pNx);
    if(Pnx_Hci_Dispatch(pNx,NFC_HCI_CLEARALLPIPES,0,(uint8_t*)&pNx->hci_private->frame_buf,&pNx->hci_private->frame_sz) == NFC_RES_ERR)  return NFC_RES_ERR;     /* Clear all pipes */
    if(Pnx_Hci_Dispatch(pNx,NFC_HCI_OPEN_PIPE_ADMIN,0,(uint8_t*)&pNx->hci_private->frame_buf,&pNx->hci_private->frame_sz) == NFC_RES_ERR) return NFC_RES_ERR;   /* Open Administration pipe */

    pNx->r_buf = hci_resp;
    pNx->r_len = &hci_resplen;
    p_hci = (general_hci_table*)&nfc_hci_tab[NFC_HCI_OPEN_PIPE_LINK_MGNT];
    memcpy(hci_buf, &p_hci->hci, p_hci->sz);
    hci_buf[1] = 0x80;
    pNx->i_buf = hci_buf;
    pNx->i_len = p_hci->sz;
    res = Pnx_Hci_Send(pNx);
    if(res != NFC_RES_OK)
    {
        printf("Create pipe for Reader A failed\n");
        return NFC_RES_ERR;
    }

    p_hci = (general_hci_table*)&nfc_hci_tab[NFC_HCI_CREATE_PIPE_READER_A];
    memcpy(hci_buf, &p_hci->hci, p_hci->sz);
    pNx->i_buf = hci_buf;
    pNx->i_len = p_hci->sz;
    res = Pnx_Hci_Send(pNx);
    if(res != NFC_RES_OK)
    {
        printf("Create pipe for Reader A failed\n");
        return NFC_RES_ERR;
    }

    p_hci = (general_hci_table*)&nfc_hci_tab[NFC_HCI_OPEN_PIPE];
    memcpy(hci_buf, &p_hci->hci, p_hci->sz);
    hci_buf[1] = 0x82;
    pNx->i_buf = hci_buf;
    pNx->i_len = p_hci->sz;
    res = Pnx_Hci_Send(pNx);
    if(res != NFC_RES_OK)
    {
        printf("Open pipe for Reader A failed\n");
        return NFC_RES_ERR;
    }

    p_hci = (general_hci_table*)&nfc_hci_tab[NFC_HCI_CREATE_PIPE_READER_B];
    memcpy(hci_buf, &p_hci->hci, p_hci->sz);
    pNx->i_buf = hci_buf;
    pNx->i_len = p_hci->sz;
    res = Pnx_Hci_Send(pNx);
    if(res != NFC_RES_OK)
    {
        printf("Create pipe for Reader B failed\n");
        return NFC_RES_ERR;
    }

    p_hci = (general_hci_table*)&nfc_hci_tab[NFC_HCI_OPEN_PIPE];
    memcpy(hci_buf, &p_hci->hci, p_hci->sz);
    hci_buf[1] = 0x83;
    pNx->i_buf = hci_buf;
    pNx->i_len = p_hci->sz;
    res = Pnx_Hci_Send(pNx);
    if(res != NFC_RES_OK)
    {
        printf("Open pipe for Reader B failed\n");
        return NFC_RES_ERR;
    }

    p_hci = (general_hci_table*)&nfc_hci_tab[NFC_HCI_CREATE_PIPE_READER_FELICA];
    memcpy(hci_buf, &p_hci->hci, p_hci->sz);
    pNx->i_buf = hci_buf;
    pNx->i_len = p_hci->sz;
    res = Pnx_Hci_Send(pNx);
    if(res != NFC_RES_OK)
    {
        printf("Create pipe for Reader Felica failed\n");
        return NFC_RES_ERR;
    }

    p_hci = (general_hci_table*)&nfc_hci_tab[NFC_HCI_OPEN_PIPE];
    memcpy(hci_buf, &p_hci->hci, p_hci->sz);
    hci_buf[1] = 0x84;
    pNx->i_buf = hci_buf;
    pNx->i_len = p_hci->sz;
    res = Pnx_Hci_Send(pNx);
    if(res != NFC_RES_OK)
    {
        printf("Open pipe for Reader Felica failed\n");
        return NFC_RES_ERR;
    }

    p_hci = (general_hci_table*)&nfc_hci_tab[NFC_HCI_CREATE_PIPE_NFCINITIATOR];
    memcpy(hci_buf, &p_hci->hci, p_hci->sz);
    pNx->i_buf = hci_buf;
    pNx->i_len = p_hci->sz;
    res = Pnx_Hci_Send(pNx);
    if(res != NFC_RES_OK)
    {
        printf("Create pipe for NFC Initiator failed\n");
        return NFC_RES_ERR;
    }

    p_hci = (general_hci_table*)&nfc_hci_tab[NFC_HCI_OPEN_PIPE];
    memcpy(hci_buf, &p_hci->hci, p_hci->sz);
    hci_buf[1] = 0x85;
    pNx->i_buf = hci_buf;
    pNx->i_len = p_hci->sz;
    res = Pnx_Hci_Send(pNx);
    if(res != NFC_RES_OK)
    {
        printf("Open pipe for NFC Initiator failed\n");
        return NFC_RES_ERR;
    }

    p_hci = (general_hci_table*)&nfc_hci_tab[NFC_HCI_SET_NFC_NEXT_MODE_SPEED];
    memcpy(hci_buf, &p_hci->hci, p_hci->sz);
    pNx->i_buf = hci_buf;
    pNx->i_len = p_hci->sz;
    res = Pnx_Hci_Send(pNx);
    if(res != NFC_RES_OK)
    {
        printf("Set NFC mode speed failed\n");
        return NFC_RES_ERR;
    }

    p_hci = (general_hci_table*)&nfc_hci_tab[NFC_HCI_CREATE_PIPE_PL_MNGT];
    memcpy(hci_buf, &p_hci->hci, p_hci->sz);
    pNx->i_buf = hci_buf;
    pNx->i_len = p_hci->sz;
    res = Pnx_Hci_Send(pNx);
    if(res != NFC_RES_OK)
    {
        printf("Create pipe for PL MNGT failed\n");
        return NFC_RES_ERR;
    }

    p_hci = (general_hci_table*)&nfc_hci_tab[NFC_HCI_OPEN_PIPE];
    memcpy(hci_buf, &p_hci->hci, p_hci->sz);
    hci_buf[1] = 0x86;
    pNx->i_buf = hci_buf;
    pNx->i_len = p_hci->sz;
    res = Pnx_Hci_Send(pNx);
    if(res != NFC_RES_OK)
    {
        printf("Open pipe for PL MNGT failed\n");
        return NFC_RES_ERR;
    }

    p_hci = (general_hci_table*)&nfc_hci_tab[NFC_HCI_CREATE_PIPE_READER_JEWEL];
    memcpy(hci_buf, &p_hci->hci, p_hci->sz);
    pNx->i_buf = hci_buf;
    pNx->i_len = p_hci->sz;
    res = Pnx_Hci_Send(pNx);
    if(res != NFC_RES_OK)
    {
        printf("Create pipe for reader Jewel failed\n");
        return NFC_RES_ERR;
    }

    p_hci = (general_hci_table*)&nfc_hci_tab[NFC_HCI_OPEN_PIPE];
    memcpy(hci_buf, &p_hci->hci, p_hci->sz);
    hci_buf[1] = 0x87;
    pNx->i_buf = hci_buf;
    pNx->i_len = p_hci->sz;
    res = Pnx_Hci_Send(pNx);
    if(res != NFC_RES_OK)
    {
        printf("Open pipe for PL MNGT failed\n");
        return NFC_RES_ERR;
    }

    p_hci = (general_hci_table*)&nfc_hci_tab[NFC_HCI_CREATE_PIPE_READER_ISO15693];
    memcpy(hci_buf, &p_hci->hci, p_hci->sz);
    pNx->i_buf = hci_buf;
    pNx->i_len = p_hci->sz;
    res = Pnx_Hci_Send(pNx);
    if(res != NFC_RES_OK)
    {
        printf("Create pipe for reader ISO15693 failed\n");
        return NFC_RES_ERR;
    }

    p_hci = (general_hci_table*)&nfc_hci_tab[NFC_HCI_OPEN_PIPE];
    memcpy(hci_buf, &p_hci->hci, p_hci->sz);
    hci_buf[1] = 0x88;
    pNx->i_buf = hci_buf;
    pNx->i_len = p_hci->sz;
    res = Pnx_Hci_Send(pNx);
    if(res != NFC_RES_OK)
    {
        printf("Open pipe for PL MNGT failed\n");
        return NFC_RES_ERR;
    }

    p_hci = (general_hci_table*)&nfc_hci_tab[NFC_HCI_SET_RD_PHASE];
    memcpy(hci_buf, &p_hci->hci, p_hci->sz);
    hci_buf[1] = 0x86;
    pNx->i_buf = hci_buf;
    pNx->i_len = p_hci->sz;
    res = Pnx_Hci_Send(pNx);
    if(res != NFC_RES_OK)
    {
        printf("Set RD_PHASE failed\n");
        return NFC_RES_ERR;
    }

    p_hci = (general_hci_table*)&nfc_hci_tab[NFC_HCI_SET_RD_PHASE2];
    memcpy(hci_buf, &p_hci->hci, p_hci->sz);
    hci_buf[1] = 0x86;
    pNx->i_buf = hci_buf;
    pNx->i_len = p_hci->sz;
    res = Pnx_Hci_Send(pNx);
    if(res != NFC_RES_OK)
    {
        printf("Set RD_PHASE2 failed\n");
        return NFC_RES_ERR;
    }
    p_hci = (general_hci_table*)&nfc_hci_tab[NFC_HCI_STOP_TYPE_A_ACT];
    memcpy(hci_buf, &p_hci->hci, p_hci->sz);
    hci_buf[1] = 0x82;
    pNx->i_buf = hci_buf;
    pNx->i_len = p_hci->sz;
    res = Pnx_Hci_Send(pNx);
    if(res != NFC_RES_OK)
    {
        printf("Set RD_PHASE2 failed\n");
        return NFC_RES_ERR;
    }

    p_hci = (general_hci_table*)&nfc_hci_tab[NFC_HCI_SEND_A_READER_REQ];
    memcpy(hci_buf, &p_hci->hci, p_hci->sz);
    pNx->i_buf = hci_buf;
    pNx->i_len = p_hci->sz;
    res = Pnx_Hci_Send(pNx);
    if(res != NFC_RES_OK)
    {
        printf("Send EVT_READER_REQUESTED timed out, no tag detected\n");
        *r_buf = 0;
        return NFC_RES_ERR;
    }
#if 0
    p_hci = (general_hci_table*)&nfc_hci_tab[NFC_HCI_SEND_B_READER_REQ];
    memcpy(hci_buf, &p_hci->hci, p_hci->sz);
    pNx->i_buf = hci_buf;
    pNx->i_len = p_hci->sz;
    res = Pnx_Hci_Send(pNx);
    if(res != NFC_RES_OK)
    {
        printf("Send EVT_READER_REQUESTED timed out, no tag detected\n");
        *r_buf = 0;
        return NFC_RES_OK;
    }
    p_hci = (general_hci_table*)&nfc_hci_tab[NFC_HCI_SEND_EVT_READER_REQ];
    memcpy(hci_buf, &p_hci->hci, p_hci->sz);
    pNx->i_buf = hci_buf;
    pNx->i_len = p_hci->sz;
    res = Pnx_Hci_Send(pNx);
    if(res != NFC_RES_OK)
    {
        printf("Send EVT_READER_REQUESTED timed out, no tag detected\n");
        *r_buf = 0;
        return NFC_RES_OK;
    }
    p_hci = (general_hci_table*)&nfc_hci_tab[NFC_HCI_SEND_I_READER_REQ];
    memcpy(hci_buf, &p_hci->hci, p_hci->sz);
    pNx->i_buf = hci_buf;
    pNx->i_len = p_hci->sz;
    res = Pnx_Hci_Send(pNx);
    if(res != NFC_RES_OK)
    {
        printf("Send EVT_READER_REQUESTED timed out, no tag detected\n");
        *r_buf = 0;
        return NFC_RES_OK;
    }
    p_hci = (general_hci_table*)&nfc_hci_tab[NFC_HCI_SEND_J_READER_REQ];
    memcpy(hci_buf, &p_hci->hci, p_hci->sz);
    pNx->i_buf = hci_buf;
    pNx->i_len = p_hci->sz;
    res = Pnx_Hci_Send(pNx);
    if(res != NFC_RES_OK)
    {
        printf("Send EVT_READER_REQUESTED timed out, no tag detected\n");
        *r_buf = 0;
        return NFC_RES_OK;
    }
    p_hci = (general_hci_table*)&nfc_hci_tab[NFC_HCI_SEND_V_READER_REQ];
    memcpy(hci_buf, &p_hci->hci, p_hci->sz);
    pNx->i_buf = hci_buf;
    pNx->i_len = p_hci->sz;
    res = Pnx_Hci_Send(pNx);
    if(res != NFC_RES_OK)
    {
        printf("Send EVT_READER_REQUESTED timed out, no tag detected\n");
        *r_buf = 0;
        return NFC_RES_OK;
    }
#endif
    if(hci_resp[2] == 0x82)
    {
        printf("\n\t\t--ISO14443 Type A card detected");
        if(hci_resp[3] == 0x00 && hci_resp[4] == 0x03)
        {
            printf("\n\t\t--Collision occurred");
        }
        else if(hci_resp[3] == 0x50 && hci_resp[4] == 0x00)
        {
            printf("\n\t\t--One card detected");
        }
        p_hci = (general_hci_table*)&nfc_hci_tab[NFC_HCI_M_A_U];
        memcpy(hci_buf, &p_hci->hci, p_hci->sz);
        pNx->i_buf = hci_buf;
        pNx->i_len = p_hci->sz;
        res = Pnx_Hci_Send(pNx);
        if(res != NFC_RES_OK)
        {
            printf("Send EVT_READER_REQUESTED timed out, no tag detected\n");
            *r_buf = 0;
            return NFC_RES_OK;
        }
        for(i = 0; i < hci_resplen - 5; i++)
        {
            if(i==0)
            {
                printf("\n\t\t--UID(%d)\n\t\t  0x",hci_resplen - 5);
            }
            printf("%02X", hci_resp[4+i]);
        }
        printf("\n");	
    }
    else if(hci_resp[2] == 0x83)
    {
        printf("\n\t\t--ISO14443 Type B card detected");
        if(hci_resp[3] == 0x00 && hci_resp[4] == 0x03)
        {
            printf("\n\t\t--Collision occurred");
        }
        else if(hci_resp[3] == 0x50 && hci_resp[4] == 0x00)
        {
            printf("\n\t\t--One card detected");
        }
        p_hci = (general_hci_table*)&nfc_hci_tab[NFC_HCI_M_B_P];
        memcpy(hci_buf, &p_hci->hci, p_hci->sz);
        pNx->i_buf = hci_buf;
        pNx->i_len = p_hci->sz;
        res = Pnx_Hci_Send(pNx);
        if(res != NFC_RES_OK)
        {
            printf("Send EVT_READER_REQUESTED timed out, no tag detected\n");
            *r_buf = 0;
            return NFC_RES_OK;
        }
        for(i = 0; i < hci_resplen - 5; i++)//hci_resplen - 5
        {
            if(i==0)
            {
                printf("\n\t\t--Application data(from ATQB)\n\t\t  0x");
            }
            printf("%02X", hci_resp[4+i]);
        }
        printf("\n");
    }
    else if(hci_resp[2] == 0x84)
    {
        printf("\n\t\t--JIS6319 Felica card detected");
        if(hci_resp[3] == 0x00 && hci_resp[4] == 0x03)
        {
            printf("\n\t\t--Collision occurred");
        }
        else if(hci_resp[3] == 0x50 && hci_resp[4] == 0x00)
        {
            printf("\n\t\t--One card detected");
        }
        p_hci = (general_hci_table*)&nfc_hci_tab[NFC_HCI_M_F_C];
        memcpy(hci_buf, &p_hci->hci, p_hci->sz);
        pNx->i_buf = hci_buf;
        pNx->i_len = p_hci->sz;
        res = Pnx_Hci_Send(pNx);
        if(res != NFC_RES_OK)
        {
            printf("Send EVT_READER_REQUESTED timed out, no tag detected\n");
            *r_buf = 0;
            return NFC_RES_OK;
        }
        for(i = 0; i < hci_resplen - 5; i++)//hci_resplen - 5
        {
            if(i==0)
            {
                printf("\n\t\t--Felica System code\n\t\t  0x");
            }
            printf("%02X", hci_resp[4+i]);
        }
        printf("\n");
    }
    else if(hci_resp[2] == 0x85)
    {
        printf("\n\t\t--ISO18092 Initiator detected");
        if(hci_resp[3] == 0x00 && hci_resp[4] == 0x03)
        {
            printf("\n\t\t--Collision occurred");
        }
        else if(hci_resp[3] == 0x50 && hci_resp[4] == 0x00)
        {
            printf("\n\t\t--One card detected");
        }
        p_hci = (general_hci_table*)&nfc_hci_tab[NFC_HCI_M_I_T];
        memcpy(hci_buf, &p_hci->hci, p_hci->sz);
        pNx->i_buf = hci_buf;
        pNx->i_len = p_hci->sz;
        res = Pnx_Hci_Send(pNx);
        if(res != NFC_RES_OK)
        {
            printf("Send EVT_READER_REQUESTED timed out, no tag detected\n");
            *r_buf = 0;
            return NFC_RES_OK;
        }
        for(i = 0; i < hci_resplen - 5; i++)//hci_resplen - 5
        {
            if(i==0)
            {
                printf("\n\t\t--NFC-I NFCID3\n\t\t  0x");
            }
            printf("%02X", hci_resp[4+i]);
        }
        printf("\n");
    }
    else if(hci_resp[2] == 0x87)
    {
        printf("\n\t\t--ISO14443 Jewel/Topaz card detected");
        if(hci_resp[3] == 0x00 && hci_resp[4] == 0x03)
        {
            printf("\n\t\t--Collision occurred");
        }
        else if(hci_resp[3] == 0x50 && hci_resp[4] == 0x00)
        {
            printf("\n\t\t--One card detected");
        }
        p_hci = (general_hci_table*)&nfc_hci_tab[NFC_HCI_M_J_T];
        memcpy(hci_buf, &p_hci->hci, p_hci->sz);
        pNx->i_buf = hci_buf;
        pNx->i_len = p_hci->sz;
        res = Pnx_Hci_Send(pNx);
        if(res != NFC_RES_OK)
        {
            printf("Send EVT_READER_REQUESTED timed out, no tag detected\n");
            *r_buf = 0;
            return NFC_RES_OK;
        }
        for(i = 0; i < hci_resplen - 5; i++)//hci_resplen - 5
        {
        if(i==0)
        {
            printf("\n\t\t--Jewel/Topaz code\n\t\t  0x");
        }
            printf("%02X", hci_resp[4+i]);
        }
        printf("\n");
    }
    else if(hci_resp[2] == 0x88)
    {
        printf("\n\t\t--ISO15693 card detected");
        if(hci_resp[3] == 0x00 && hci_resp[4] == 0x03)
        {
            printf("\n\t\t--Collision occurred");
        }
        else if(hci_resp[3] == 0x50 && hci_resp[4] == 0x00)
        {
            printf("\n\t\t--One card detected");
        }
        p_hci = (general_hci_table*)&nfc_hci_tab[NFC_HCI_M_R_I];
        memcpy(hci_buf, &p_hci->hci, p_hci->sz);
        pNx->i_buf = hci_buf;
        pNx->i_len = p_hci->sz;
        res = Pnx_Hci_Send(pNx);
        if(res != NFC_RES_OK)
        {
            printf("Send EVT_READER_REQUESTED timed out, no tag detected\n");
            *r_buf = 0;
            return NFC_RES_OK;
        }
        for(i = hci_resplen - 7; i > 0; i--)
        {
            if(i==hci_resplen - 7)
            {
                printf("\n\t\t--ISO15693 read inventory\n\t\t  0x");
            }
            printf("%02X", hci_resp[5+i]);
        }
        printf("\n");
    }
    else
    {
        printf("\n\t\t--Error card detected");
    }
    return NFC_RES_OK;
}

NFC_STATUS Pnx_Set_Prb(PNX_HANDLE pNx, uint8_t tech, uint8_t rate)
{
    NFC_STATUS res = NFC_RES_ERR;
    general_hci_table* p_hci = NULL;
    uint8_t hci_buf[MAX_FRAME_LEN] = {0};
    uint8_t hci_resp[MAX_FRAME_LEN] = {0};
    uint8_t hci_resplen = 0;

    Pnx_Hci_Start(pNx);
    p_hci = (general_hci_table*)&nfc_hci_tab[NFC_HCI_SELF_TEST_PBRS];
    memcpy(hci_buf, &p_hci->hci, p_hci->sz);
    hci_buf[3]= tech;
    hci_buf[4]= rate;
    pNx->i_buf = hci_buf;
    pNx->i_len = p_hci->sz;
    pNx->r_buf = hci_resp;
    pNx->r_len = &hci_resplen;
    res = Pnx_Hci_Send(pNx);
    return res;
}
NFC_STATUS Pnx_Set_Swp(PNX_HANDLE pNx,uint8_t sw)
{
    NFC_STATUS res = NFC_RES_ERR;
    int i =0;
    uint32_t temp = 0;
    uint8_t r_buf[MAX_FRAME_LEN] = {0};
    uint8_t r_len = 0;

    Pnx_Hci_Start(pNx);
    /* Create SWP mgnt pipe */
    res=Pnx_Hci_Dispatch(pNx,NFC_HCI_CREATE_PIPE_SWP_MGNT,0,(uint8_t*)&pNx->hci_private->frame_buf,&pNx->hci_private->frame_sz);
    res=Pnx_Hci_Dispatch(pNx,NFC_HCI_OPEN_PIPE_SWP_MGNT,pNx->hci_private->frame_buf[8],(uint8_t*)&pNx->hci_private->frame_buf,&pNx->hci_private->frame_sz);
    /* SWP On/Off Test*/
    general_hci_table* p_hci = NULL;
    uint8_t hci_buf[MAX_FRAME_LEN]={0};
    p_hci=(general_hci_table*)&nfc_hci_tab[NFC_EVT_SWP_SWITCH_MODE];
    memcpy(&hci_buf[0],&p_hci->hci,p_hci->sz);
    if (pNx->hci_private->frame_buf[8] >0) 
    {
        hci_buf[1]= 0x80 | pNx->hci_private->frame_buf[8];
    }
    hci_buf[4] = sw;
    pNx->i_buf = hci_buf;
    pNx->i_len = p_hci->sz;
    pNx->r_buf = r_buf;
    pNx->r_len = &r_len;
    res=Pnx_Hci_Send(pNx);

    if(res != NFC_RES_OK)
    {
        printf("swp on/off test command failed\n");
        return NFC_RES_ERR;
    }
    if(sw == 1)
    {
        printf("\n\t\t--SWP Enable Succeed"); 
        printf("\n\t\t  Please use an external reader to exchange data with SWP SE");
        printf("\n\t\t  For example Get Challenge T=CL(00 84 00 00 08)\n");   
    }
    else
    {
        printf("\n\t\t--SWP Disable Succeed\n");  
    }
    return NFC_RES_OK;
}

NFC_STATUS Pnx_Get_APDU(PNX_HANDLE pNx)
{
    NFC_STATUS res = NFC_RES_ERR;
    uint8_t score[MAX_FRAME_LEN] = {0};
    uint8_t c =0;
    uint8_t i =0;
    uint8_t digi_check =0;
    uint8_t j =0;
    uint8_t k =0;
    uint8_t r_buf[MAX_FRAME_LEN] = {0};
    uint8_t r_len = 0;
    uint8_t hci_buf[MAX_FRAME_LEN]={0x80,0x83,0x10,0x05};

    printf("\n--SMX Wired Mode Enable Succeed"); 
    printf("\n  Please Enter The JCOP APDUs,example(# 00 84 00 00 08)\n");
    while(1)
    {
        printf("\n$ 'q' or 'Q' for quit APDU exchanges");
        printf("\n# ");
        i = 0;
        digi_check = 0;
        do
        {
            scanf("%x",&score[i]);
            i++;
            c=getchar();
            if(c=='q' || c=='Q')
            {
                return NFC_RES_OK;
            }
            if(c=='\n')
            {
                for(j=0;j<i;j++)
                {
                    if(score[j] < 0x0 || score[j] > 0xff)
                    {
                        digi_check =1;
                        break;
                    }
                }
                if(i>MAX_FRAME_LEN || i<= 0 || digi_check ==1)
                {
                    printf("\n# Invalid argument");
                }
                else
                {
                    for(j=0;j<i;j++)
                    {
                        pNx->pnx_print(pNx,"0x%x ",score[j]);
                        hci_buf[4+j] = score[j];
                    }

                    pNx->i_buf = hci_buf;
                    pNx->i_len = 4+i;
                    pNx->r_buf = r_buf;
                    pNx->r_len = &r_len;
                    res=Pnx_Hci_Send(pNx);

                    if(res != NFC_RES_OK)
                    {
                        printf("\n# Transaction Failed");
                    } 

                    for(k = 0; k < r_len-5; k++)
                    {
                        if((k % 30) == 0)
                        {
                            if(k==0)
                            {
                                printf("\n@ ");
                            }
                            else
                            {
                                printf("\n  ");
                            }
                        }
                        printf("%02X", r_buf[4+k]);
                    } 
                    printf("\n");
                }
                break;
            }
        }while(1);
    } 
}
NFC_STATUS Pnx_Set_Smx(PNX_HANDLE pNx,uint8_t sw)
{
    NFC_STATUS res = NFC_RES_ERR;
    uint8_t i =0;
    uint32_t temp = 0;
    uint8_t r_buf[MAX_FRAME_LEN] = {0};
    uint8_t r_len = 0;
    general_hci_table* p_hci = NULL;
    uint8_t hci_buf[MAX_FRAME_LEN]={0};

    Pnx_Hci_Start(pNx);
    if(sw == 0)
    {
        for(i=NFC_SMX_HCI_Open_Pipe_Admin;i<=NFC_HCI_READES_UID;i++)
        {

            p_hci=(general_hci_table*)&nfc_hci_tab[i];
            memcpy(&hci_buf[0],&p_hci->hci,p_hci->sz);
            pNx->i_buf = hci_buf;
            pNx->i_len = p_hci->sz;
            pNx->r_buf = r_buf;
            pNx->r_len = &r_len;
            res=Pnx_Hci_Send(pNx);
        }
        if(res != NFC_RES_OK)
        {
            printf("SMX Wired/Virtual Test Failed\n");
            return NFC_RES_ERR;
        } 
        for(i = 0; i < r_len-5; i++)
        {
            if((i % 10) == 0)
            {
                printf("\n--Embedded SE UID(%d)\n  0x",r_len - 5);
            }
            printf("%02X", r_buf[4+i]);
        }  

        p_hci=(general_hci_table*)&nfc_hci_tab[NFC_HCI_reads_Appl_Data];
        memcpy(&hci_buf[0],&p_hci->hci,p_hci->sz);
        pNx->i_buf = hci_buf;
        pNx->i_len = p_hci->sz;
        pNx->r_buf = r_buf;
        pNx->r_len = &r_len;
        res=Pnx_Hci_Send(pNx);
        if(res != NFC_RES_OK)
        {
            printf("SMX Wired/Virtual Test Failed\n");
            return NFC_RES_ERR;
        } 
        for(i = 0; i < r_len-5; i++)
        {
            if((i % 10) == 0)

            {
                printf("\n--Application Data(%d)\n  0x",r_len - 5);
            }
            printf("%02X", r_buf[4+i]);
        }  

        p_hci=(general_hci_table*)&nfc_hci_tab[NFC_HCI_reads_SAK];
        memcpy(&hci_buf[0],&p_hci->hci,p_hci->sz);
        pNx->i_buf = hci_buf;
        pNx->i_len = p_hci->sz;
        pNx->r_buf = r_buf;
        pNx->r_len = &r_len;
        res=Pnx_Hci_Send(pNx);
        if(res != NFC_RES_OK)
        {
            printf("SMX Wired/Virtual Test Failed\n");
            return NFC_RES_ERR;
        } 
        for(i = 0; i < r_len-5; i++)
        {
            if((i % 10) == 0)
            {
                printf("\n--SmartMX SAK(%d)\n  0x",r_len - 5);
            }
            printf("%02X", r_buf[4+i]);
        }  

        p_hci=(general_hci_table*)&nfc_hci_tab[NFC_HCI_reads_Fwi];
        memcpy(&hci_buf[0],&p_hci->hci,p_hci->sz);
        pNx->i_buf = hci_buf;
        pNx->i_len = p_hci->sz;
        pNx->r_buf = r_buf;
        pNx->r_len = &r_len;
        res=Pnx_Hci_Send(pNx);
        if(res != NFC_RES_OK)
        {
            printf("SMX Wired/Virtual Test Failed\n");
            return NFC_RES_ERR;
        } 
        for(i = 0; i < r_len-5; i++)
        {
            if((i % 10) == 0)
            {
                printf("\n--SmartMX FWI(%d)\n  0x",r_len - 5);
            }
            printf("%02X", r_buf[4+i]);
        }  

        p_hci=(general_hci_table*)&nfc_hci_tab[NFC_APDU_Select_Card_Manager_JCOP];
        memcpy(&hci_buf[0],&p_hci->hci,p_hci->sz);
        pNx->i_buf = hci_buf;
        pNx->i_len = p_hci->sz;
        pNx->r_buf = r_buf;
        pNx->r_len = &r_len;
        res=Pnx_Hci_Send(pNx);
        general_hci_table* p_hci = NULL;
        uint8_t hci_buf[MAX_FRAME_LEN]={0};
        p_hci=(general_hci_table*)&nfc_hci_tab[NFC_Get_CPLC_from_JCOP];
        memcpy(&hci_buf[0],&p_hci->hci,p_hci->sz);
        pNx->i_buf = hci_buf;
        pNx->i_len = p_hci->sz;
        pNx->r_buf = r_buf;
        pNx->r_len = &r_len;
        res=Pnx_Hci_Send(pNx);
        if(res != NFC_RES_OK)
        {
            printf("SMX Wired/Virtual Test Failed\n");
            return NFC_RES_ERR;
        } 
        for(i = 0; i < r_len-5; i++)
        {
            if((i % 10) == 0)
            {
                if(i==0)
                printf("\n--SmartMX CPLC(%d)\n  0x",r_len - 5);
                else
                printf("\n    ");
            }
            printf("%02X", r_buf[4+i]);
        }  
        Pnx_Get_APDU(pNx);
    }
    else
    {
        for(i=NFC_SMX_HCI_Open_Pipe_Admin;i<=NFC_SMX_HCI_ADMINISTRATION_GATE;i++)
        {

            p_hci=(general_hci_table*)&nfc_hci_tab[i];
            memcpy(&hci_buf[0],&p_hci->hci,p_hci->sz);
            pNx->i_buf = hci_buf;
            pNx->i_len = p_hci->sz;
            pNx->r_buf = r_buf;
            pNx->r_len = &r_len;
            res=Pnx_Hci_Send(pNx);
        }
        if(res != NFC_RES_OK)
        {
            printf("SMX Wired/Virtual Test Failed\n");
            return NFC_RES_ERR;
        } 
        for(i=NFC_HCI_NFC_WI_GATE;i<=NFC_EVT_SE_Switch_virtual;i++)
        {

            p_hci=(general_hci_table*)&nfc_hci_tab[i];
            memcpy(&hci_buf[0],&p_hci->hci,p_hci->sz);
            pNx->i_buf = hci_buf;
            pNx->i_len = p_hci->sz;
            pNx->r_buf = r_buf;
            pNx->r_len = &r_len;
            res=Pnx_Hci_Send(pNx);
        }
        if(res != NFC_RES_OK)
        {
            printf("SMX Wired/Virtual Test Failed\n");
            return NFC_RES_ERR;
        }
        printf("\n\t\t--SMX Virtual Mode Enable Succeed"); 
        printf("\n\t\t  Please use an external reader to exchange data with SMX");
        printf("\n\t\t  For example Get Challenge T=Cl(00 84 00 00 08)\n");  
    }	     
    return NFC_RES_OK;
}

