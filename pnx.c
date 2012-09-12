//============================================================================
//  File Name: pnx.c 
//
//  2012 Copyright (c), NXP Semiconductors. All rights reserved.
//
//  This source code is NXP Semiconductors N.V. proprietary and confidential
//  information
//
//  Description:
//      PNX application entrance 
//      Distribution commands by the user
//
//  Modification History:
//      Revision 1.0  2012/02/06 15:38:44  xiaohua.wang@nxp.com
//      Initial draft
//
//      Revision 1.1  2012/02/06 20:51:22  xiaohua.wang@nxp.com
//      Verified on Micosoft VS 2008
//      
//      Revision 1.2  2012/02/07 09:51:17  xiaohua.wang@nxp.com
//      Added these into Android.mk
//      ---------------------------
//      LOCAL_PATH:= $(call my-dir)
//      include $(CLEAR_VARS)
//      LOCAL_SRC_FILES:= \
//                  cmd.c \
//                  hci.c \
//                  opt.c \
//                  pnx.c
//      LOCAL_MODULE:= pnx
//      include $(BUILD_EXECUTABLE)
//      ---------------------------
//
//      Revision 1.3  2012/02/09 15:19:02  xiaohua.wang@nxp.com
//      Verified on Google Nexus S
//
//      Revision 1.4  2012/02/17 13:07:38  xiaohua.wang@nxp.com
//      Add PBRS test command
//
//      Revision 1.5  2012/03/15 11:31:09  xiaohua.wang@nxp.com
//      Add Polling Loop command
//
//      Revision 1.6  2012/03/21 16:08:37  xiaohua.wang@nxp.com
//      Add supported for Google Galaxy Nexus
//
//=============================================================================

#include <pnx.h>

#define NODE_INI "/system/bin/node.ini"
#define MAXLINE 100
static uint8_t node_ini[256] = {"/dev/pn544"};

NFC_STATUS Pnx_Uart_Reset(PNX_HANDLE pNx,long level)
{
    static const char NFC_POWER_PATH[] = "/sys/devices/platform/nfc-power/nfc_power";
    int sz;
    int fd = -1;
    int ret = NFC_RES_ERR;
    char buffer[2];

    //printf("_uart_reset, VEN level = %ld\n", level);

    if (snprintf(buffer, sizeof(buffer), "%u", (unsigned int)level) != 1) {
        pNx->pnx_print(pNx,"Bad nfc power level (%u)\n", (unsigned int)level);
        goto out;
    }

    fd = open(NFC_POWER_PATH, O_WRONLY);
    if (fd < 0) {
        pNx->pnx_print(pNx,"open(%s) for write failed: %s (%d)\n", NFC_POWER_PATH,
                strerror(errno), errno);
        goto out;
    }
    sz = write(fd, &buffer, sizeof(buffer) - 1);
    if (sz < 0) {
        pNx->pnx_print(pNx,"write(%s) failed: %s (%d)\n", NFC_POWER_PATH, strerror(errno),
             errno);
        goto out;
    }
    ret = NFC_RES_OK;
#if 0
    if (level == 2) {
        libnfc_firmware_mode = 1;
    } else {
        libnfc_firmware_mode = 0;
    }
#endif
out:
    if (fd >= 0) {
        close(fd);
    }
    return ret;
}

NFC_STATUS Pnx_Node(void)
{
    uint8_t i = 0;
    uint8_t flag = 0;
    FILE *fp_inp = NULL;
    uint8_t arr[MAXLINE+1] = {0};

    fp_inp =fopen(NODE_INI, "r");
    if(fp_inp != 0)
    {
       close(fp_inp);
       system("rm -r /system/bin/node.ini");
    }
    close(fp_inp);
    system("ls -l /dev >>/system/bin/node.ini");
    fp_inp = fopen(NODE_INI, "r");
    if(fp_inp == 0) 
    {
        printf("Node File " NODE_INI " Open Error\n");	
        return NFC_RES_ERR;
    }
    while ((fgets (arr, MAXLINE, fp_inp)) != NULL)
    {
         for(i=0;i<strlen(arr);i++)
         {
             if((arr[i] == 'n') && (arr[i+1] == 'f') && \
               (arr[i+2] == 'c') && (arr[i+3] == ' '))
             {
                 flag ++ ;
             }
         }
         if(flag >= 2)
         {
             for(i=strlen(arr);i>0;i--)
             {
                 if(arr[i] == ' ')
                 {
                     uint8_t buffer[MAXLINE] = {0};

                     memset(node_ini,0,sizeof(node_ini));
                     memcpy(buffer,&(arr[i+1]),strlen(arr)-i-2);
                     sprintf(node_ini,"/dev/%s",buffer);
                     break;
                 }   
             }
             break;
         }
         else
         {
             flag = 0;   
         } 
    }
    close(fp_inp);
    return NFC_RES_OK;
}

NFC_STATUS Pnx_Open(PNX_HANDLE *pNx)
{
    int ret = 0;
   
    pnx_info_struct *deviceinfo = malloc(sizeof(pnx_info_struct));

    deviceinfo->hide = FALSE;
    Pnx_Node();
   
    if ((ret = (deviceinfo->fp = open(node_ini, O_RDWR /*| O_NONBLOCK | O_NOCTTY*/))) < 0) 
    {
        printf("%s open error retcode = %d, errno = %d\n",node_ini, ret, errno);
        goto _ERROR_;
    }

    deviceinfo->verbose = FALSE;
    deviceinfo->fw_version = NULL;
    deviceinfo->_started = FALSE;
    deviceinfo->hci_crc = Pnx_Hci_Crc;
    deviceinfo->pnx_print = Pnx_Print;
    deviceinfo->pnx_hci_trace = Pnx_Hci_Trace;
    deviceinfo->pnx_verbose_check = Pnx_Verbose_Check;
    deviceinfo->hci_private = malloc(sizeof(hci_info_struct));
    deviceinfo->hci_private->n_r = 0x0;
    deviceinfo->hci_private->n_s = 0x0;
    deviceinfo->hci_private->_ack = 0x0;
    memset(deviceinfo->hci_private->frame_buf,0,sizeof(deviceinfo->hci_private->frame_buf));
    deviceinfo->hci_private->frame_sz = 0x0;
    deviceinfo->i_buf = NULL;
    deviceinfo->i_len = 0x0;
    deviceinfo->r_buf = NULL;
    deviceinfo->r_len = 0x0;
    *pNx = (PNX_HANDLE)deviceinfo;

    return NFC_RES_OK;
    
_ERROR_:
    free(deviceinfo);
    
    return NFC_RES_ERR;
}

NFC_STATUS Pnx_Close(PNX_HANDLE pNx)
{
    pnx_info_struct *deviceInfo = pNx;
    hci_info_struct *hciInfo = deviceInfo->hci_private;

    pNx->pnx_print(pNx,"\n#%s#\n",node_ini);
    close(deviceInfo->fp);
    
    if(hciInfo)
    {
        free(hciInfo);
    }
    if(deviceInfo)
    {
        free(deviceInfo);
    }
    pNx = NULL;
    
    return NFC_RES_OK;
}
  
int main(int argc, char* argv[])
{
    PNX_HANDLE pnx = NULL;

    if(Pnx_Open(&pnx) != NFC_RES_OK)
    {
        printf("Pnx_Open [FAIL]\n");
        return -1;
    }
    pnx->argc = argc;
    pnx->argv = argv;
    
    if(Pnx_Cmd_Handle(pnx) != NFC_RES_OK)
    {
        printf("Pnx_Cmd_Handle [FAIL]\n");
        return -1;
    }
    
    Pnx_Close(pnx);
    return 0;
}

