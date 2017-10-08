/* Copyright (C) 2014 Murata Manufacturing Co.,Ltd., MIT License
 *  muRata, SWITCH SCIENCE Wi-FI module TypeYD-SNIC UART.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "SNIC_WifiInterface.h"
#include "SNIC_UartMsgUtil.h"

#define UART_CONNECT_BUF_SIZE   512
unsigned char gCONNECT_BUF[UART_CONNECT_BUF_SIZE];
static char ip_addr[17] = "\0";

C_SNIC_WifiInterface::C_SNIC_WifiInterface( PinName tx, PinName rx, PinName cts, PinName rts, PinName reset, PinName alarm, int baud)
{
    mUART_tx     = tx;
    mUART_rx     = rx;
    mUART_cts    = cts;
    mUART_rts    = rts;;
    mUART_baud   = baud;
    mModuleReset = reset;
}

void C_SNIC_WifiInterface::create( PinName tx, PinName rx, PinName cts, PinName rts, PinName reset, PinName alarm, int baud)
{
    mUART_tx     = tx;
    mUART_rx     = rx;
    mUART_cts    = cts;
    mUART_rts    = rts;;
    mUART_baud   = baud;
    mModuleReset = reset;
}

C_SNIC_WifiInterface::~C_SNIC_WifiInterface()
{
}

int C_SNIC_WifiInterface::init()
{   
    C_SNIC_Core               *snic_core_p  = C_SNIC_Core::getInstance();
    C_SNIC_UartCommandManager *uartCmdMgr_p = snic_core_p->getUartCommand();
    
    /* Initialize UART */
    snic_core_p->initUart( mUART_tx, mUART_rx, mUART_baud );

    /* Module reset */
    snic_core_p->resetModule( mModuleReset );
    
    wait(1);
    /* Initialize SNIC API */
    // Get buffer for response payload from MemoryPool
    tagMEMPOOL_BLOCK_T *payload_buf_p = snic_core_p->allocCmdBuf();
    if( payload_buf_p == NULL )
    {
        DEBUG_PRINT("snic_init payload_buf_p NULL\r\n");
        return -1;
    }

    C_SNIC_Core::tagSNIC_INIT_REQ_T req;
    // Make request
    req.cmd_sid  = UART_CMD_SID_SNIC_INIT_REQ;
    req.seq      = mUartRequestSeq++;
    req.buf_size[0] = 0x08;
    req.buf_size[1] = 0x00;

    unsigned char *command_array_p = snic_core_p->getCommandBuf();
    unsigned int  command_len;
    // Preparation of command
    command_len = snic_core_p->preparationSendCommand( UART_CMD_ID_SNIC, req.cmd_sid, (unsigned char *)&req
                            , sizeof(C_SNIC_Core::tagSNIC_INIT_REQ_T), payload_buf_p->buf, command_array_p );

    // Send uart command request
    snic_core_p->sendUart( command_len, command_array_p );

    int ret;
    // Wait UART response
    ret = uartCmdMgr_p->wait();
    if( ret != 0 )
    {
        DEBUG_PRINT( "snic_init failed\r\n" );
        snic_core_p->freeCmdBuf( payload_buf_p );
        return -1;
    }
    
    if( uartCmdMgr_p->getCommandStatus() != 0 )
    {
        DEBUG_PRINT("snic_init status:%02x\r\n", uartCmdMgr_p->getCommandStatus());
        snic_core_p->freeCmdBuf( payload_buf_p );
        return -1;
    }
    snic_core_p->freeCmdBuf( payload_buf_p );
    
    return ret;
}

int C_SNIC_WifiInterface::getFWVersion( unsigned char *version_p )
{
    C_SNIC_Core               *snic_core_p  = C_SNIC_Core::getInstance();
    C_SNIC_UartCommandManager *uartCmdMgr_p = snic_core_p->getUartCommand();
    
    // Get buffer for response payload from MemoryPool
    tagMEMPOOL_BLOCK_T *payload_buf_p = snic_core_p->allocCmdBuf();
    if( payload_buf_p == NULL )
    {
        DEBUG_PRINT("getFWVersion payload_buf_p NULL\r\n");
        return -1;
    }

    C_SNIC_Core::tagGEN_FW_VER_GET_REQ_T req;
    // Make request
    req.cmd_sid = UART_CMD_SID_GEN_FW_VER_GET_REQ;
    req.seq     = mUartRequestSeq++;
    
    unsigned char *command_array_p = snic_core_p->getCommandBuf();
    unsigned int  command_len;
    // Preparation of command
    command_len = snic_core_p->preparationSendCommand( UART_CMD_ID_GEN, req.cmd_sid, (unsigned char *)&req
                        , sizeof(C_SNIC_Core::tagGEN_FW_VER_GET_REQ_T), payload_buf_p->buf, command_array_p );

    int ret;
    
    // Send uart command request
    snic_core_p->sendUart( command_len, command_array_p );
    
    // Wait UART response
    ret = uartCmdMgr_p->wait();
    if( ret != 0 )
    {
        DEBUG_PRINT( "getFWversion failed\r\n" );
        snic_core_p->freeCmdBuf( payload_buf_p );
        return -1;
    }
    
    if( uartCmdMgr_p->getCommandStatus() == 0 )
    {
        unsigned char version_len = payload_buf_p->buf[3];
        memcpy( version_p, &payload_buf_p->buf[4], version_len );
    }
    snic_core_p->freeCmdBuf( payload_buf_p );
    return 0;
}

int C_SNIC_WifiInterface::connect(const char *ssid_p, unsigned char ssid_len, E_SECURITY sec_type
                            , const char *sec_key_p, unsigned char sec_key_len)
{
    C_SNIC_Core               *snic_core_p  = C_SNIC_Core::getInstance();
    C_SNIC_UartCommandManager *uartCmdMgr_p = snic_core_p->getUartCommand();

    // Parameter check(SSID)
    if( (ssid_p == NULL) || (ssid_len == 0) )
    {
        DEBUG_PRINT( "connect failed [ parameter NG:SSID ]\r\n" );
        return -1;
    }
    
    // Parameter check(Security key)
    if( (sec_type != e_SEC_OPEN) && ( (sec_key_len == 0) || (sec_key_p == NULL) ) )
    {
        DEBUG_PRINT( "connect failed [ parameter NG:Security key ]\r\n" );
        return -1;
    }
    
    // Get buffer for response payload from MemoryPool
    tagMEMPOOL_BLOCK_T *payload_buf_p = snic_core_p->allocCmdBuf();
    if( payload_buf_p == NULL )
    {
        DEBUG_PRINT("connect payload_buf_p NULL\r\n");
        return -1;
    }

    unsigned char *buf = &gCONNECT_BUF[0];
    unsigned int  buf_len = 0;
    unsigned int  command_len;

    memset( buf, 0, UART_CONNECT_BUF_SIZE );
    // Make request
    buf[0] = UART_CMD_SID_WIFI_JOIN_REQ;
    buf_len++;
    buf[1] = mUartRequestSeq++;
    buf_len++;
    // SSID
    memcpy( &buf[2], ssid_p, ssid_len );
    buf_len += ssid_len;
    buf_len++;
    
    // Security mode
    buf[ buf_len ] = (unsigned char)sec_type;
    buf_len++;

    // Security key
    if( sec_type != e_SEC_OPEN )
    {
        buf[ buf_len ] = sec_key_len;
        buf_len++;
        if( sec_key_len > 0 )
        {
            memcpy( &buf[buf_len], sec_key_p, sec_key_len );
            buf_len += sec_key_len;
        }
    }

    unsigned char *command_array_p = snic_core_p->getCommandBuf();
    // Preparation of command
    command_len = snic_core_p->preparationSendCommand( UART_CMD_ID_WIFI, UART_CMD_SID_WIFI_JOIN_REQ, buf
                        , buf_len, payload_buf_p->buf, command_array_p );

    // Send uart command request
    snic_core_p->sendUart( command_len, command_array_p );
    
    int ret;
    // Wait UART response
    ret = uartCmdMgr_p->wait();
    if(uartCmdMgr_p->getCommandStatus() != UART_CMD_RES_WIFI_ERR_ALREADY_JOINED)
    {
        DEBUG_PRINT( "Already connected\r\n" );
    }
    else
    {
        if( ret != 0 )
        {
            DEBUG_PRINT( "join failed\r\n" );
            snic_core_p->freeCmdBuf( payload_buf_p );
            return -1;
        }
    }
    
    if(uartCmdMgr_p->getCommandStatus() != 0)
    {
        DEBUG_PRINT("join status:%02x\r\n", uartCmdMgr_p->getCommandStatus());
        snic_core_p->freeCmdBuf( payload_buf_p );
        return -1;
    }
    snic_core_p->freeCmdBuf( payload_buf_p );

    return ret;
}

int C_SNIC_WifiInterface::disconnect()
{
    C_SNIC_Core               *snic_core_p  = C_SNIC_Core::getInstance();
    C_SNIC_UartCommandManager *uartCmdMgr_p = snic_core_p->getUartCommand();
    
    // Get buffer for response payload from MemoryPool
    tagMEMPOOL_BLOCK_T *payload_buf_p = snic_core_p->allocCmdBuf();
    if( payload_buf_p == NULL )
    {
        DEBUG_PRINT("disconnect payload_buf_p NULL\r\n");
        return -1;
    }

    C_SNIC_Core::tagWIFI_DISCONNECT_REQ_T req;
    // Make request
    req.cmd_sid = UART_CMD_SID_WIFI_DISCONNECT_REQ;
    req.seq = mUartRequestSeq++;
    
    unsigned char *command_array_p = snic_core_p->getCommandBuf();
    unsigned int  command_len;
    // Preparation of command
    command_len = snic_core_p->preparationSendCommand( UART_CMD_ID_WIFI, req.cmd_sid, (unsigned char *)&req
                        , sizeof(C_SNIC_Core::tagWIFI_DISCONNECT_REQ_T), payload_buf_p->buf, command_array_p );

    // Send uart command request
    snic_core_p->sendUart( command_len, command_array_p );
    
    int ret;
    // Wait UART response
    ret = uartCmdMgr_p->wait();
    if( ret != 0 )
    {
        DEBUG_PRINT( "disconnect failed\r\n" );
        snic_core_p->freeCmdBuf( payload_buf_p );
        return -1;
    }
    
    if( uartCmdMgr_p->getCommandStatus() != 0 )
    {
        DEBUG_PRINT("disconnect status:%02x\r\n", uartCmdMgr_p->getCommandStatus());
        ret = -1;
    }
    snic_core_p->freeCmdBuf( payload_buf_p );
    return ret;
}

int C_SNIC_WifiInterface::scan( const char *ssid_p, unsigned char *bssid_p
                        , void (*result_handler_p)(tagSCAN_RESULT_T *scan_result) )
{
    C_SNIC_Core               *snic_core_p  = C_SNIC_Core::getInstance();
    C_SNIC_UartCommandManager *uartCmdMgr_p = snic_core_p->getUartCommand();

    // Get buffer for response payload from MemoryPool
    tagMEMPOOL_BLOCK_T *payload_buf_p = snic_core_p->allocCmdBuf();
    if( payload_buf_p == NULL )
    {
        DEBUG_PRINT("scan payload_buf_p NULL\r\n");
        return -1;
    }
    
    C_SNIC_Core::tagWIFI_SCAN_REQ_T req;
    unsigned int  buf_len = 0;
    
    memset( &req, 0, sizeof(C_SNIC_Core::tagWIFI_SCAN_REQ_T) );
    // Make request
    req.cmd_sid = UART_CMD_SID_WIFI_SCAN_REQ;
    buf_len++;
    req.seq = mUartRequestSeq++;
    buf_len++;
    
    // Set scan type(Active scan)
    req.scan_type = 0;
    buf_len++;
    // Set bss type(any)
    req.bss_type = 2;
    buf_len++;
    // Set BSSID
    if( bssid_p != NULL )
    {
        memcpy( req.bssid, bssid_p, BSSID_MAC_LENTH );
    }
    buf_len += BSSID_MAC_LENTH;
    // Set channel list(0)
    req.chan_list = 0;
    buf_len++;
    //Set SSID
    if( ssid_p != NULL )
    {
        strcpy( (char *)req.ssid, ssid_p );
        buf_len += strlen(ssid_p);
    }
    buf_len++;

    unsigned char *command_array_p = snic_core_p->getCommandBuf();
    unsigned int  command_len;
    // Preparation of command
    command_len = snic_core_p->preparationSendCommand( UART_CMD_ID_WIFI, req.cmd_sid, (unsigned char *)&req
                        , buf_len, payload_buf_p->buf, command_array_p );

    // Set scan result callback 
    uartCmdMgr_p->setScanResultHandler( result_handler_p );
    
    // Send uart command request
    snic_core_p->sendUart( command_len, command_array_p );

    int ret;
    // Wait UART response
    ret = uartCmdMgr_p->wait();
    DEBUG_PRINT( "scan wait:%d\r\n", ret );
    if( ret != 0 )
    {
        DEBUG_PRINT( "scan failed\r\n" );
        snic_core_p->freeCmdBuf( payload_buf_p );
        return -1;
    }
    
    if( uartCmdMgr_p->getCommandStatus() != 0 )
    {
        DEBUG_PRINT("scan status:%02x\r\n", uartCmdMgr_p->getCommandStatus());
        snic_core_p->freeCmdBuf( payload_buf_p );
        return -1;
    }

    snic_core_p->freeCmdBuf( payload_buf_p );

    return ret;
}

int C_SNIC_WifiInterface::wifi_on( const char *country_p )
{
    C_SNIC_Core               *snic_core_p  = C_SNIC_Core::getInstance();
    C_SNIC_UartCommandManager *uartCmdMgr_p = snic_core_p->getUartCommand();

    // Parameter check
    if( country_p == NULL )
    {
        DEBUG_PRINT("wifi_on parameter error\r\n");
        return -1;
    }
    
    // Get buffer for response payload from MemoryPool
    tagMEMPOOL_BLOCK_T *payload_buf_p = snic_core_p->allocCmdBuf();
    if( payload_buf_p == NULL )
    {
        DEBUG_PRINT("wifi_on payload_buf_p NULL\r\n");
        return -1;
    }

    C_SNIC_Core::tagWIFI_ON_REQ_T req;
    // Make request
    req.cmd_sid = UART_CMD_SID_WIFI_ON_REQ;
    req.seq = mUartRequestSeq++;
    memcpy( req.country, country_p, COUNTRYC_CODE_LENTH );
    
    unsigned char *command_array_p = snic_core_p->getCommandBuf();
    unsigned int  command_len;
    // Preparation of command
    command_len = snic_core_p->preparationSendCommand( UART_CMD_ID_WIFI, req.cmd_sid, (unsigned char *)&req
                        , sizeof(C_SNIC_Core::tagWIFI_ON_REQ_T), payload_buf_p->buf, command_array_p );

    // Send uart command request
    snic_core_p->sendUart( command_len, command_array_p );
    
    int ret;
    // Wait UART response
    ret = uartCmdMgr_p->wait();
    if( ret != 0 )
    {
        DEBUG_PRINT( "wifi_on failed\r\n" );
        snic_core_p->freeCmdBuf( payload_buf_p );
        return -1;
    }
    
    if( uartCmdMgr_p->getCommandStatus() != 0 )
    {
        DEBUG_PRINT("wifi_on status:%02x\r\n", uartCmdMgr_p->getCommandStatus());
        snic_core_p->freeCmdBuf( payload_buf_p );
        return -1;
    }
    snic_core_p->freeCmdBuf( payload_buf_p );

    return ret;
}

int C_SNIC_WifiInterface::wifi_off()
{
    C_SNIC_Core               *snic_core_p  = C_SNIC_Core::getInstance();
    C_SNIC_UartCommandManager *uartCmdMgr_p = snic_core_p->getUartCommand();

    // Get buffer for response payload from MemoryPool
    tagMEMPOOL_BLOCK_T *payload_buf_p = snic_core_p->allocCmdBuf();
    if( payload_buf_p == NULL )
    {
        DEBUG_PRINT("wifi_off payload_buf_p NULL\r\n");
        return -1;
    }

    C_SNIC_Core::tagWIFI_OFF_REQ_T req;
    // Make request
    req.cmd_sid = UART_CMD_SID_WIFI_OFF_REQ;
    req.seq = mUartRequestSeq++;
    
    unsigned char *command_array_p = snic_core_p->getCommandBuf();
    unsigned int  command_len;
    // Preparation of command
    command_len = snic_core_p->preparationSendCommand( UART_CMD_ID_WIFI, req.cmd_sid, (unsigned char *)&req
                        , sizeof(C_SNIC_Core::tagWIFI_OFF_REQ_T), payload_buf_p->buf, command_array_p );

    // Send uart command request
    snic_core_p->sendUart( command_len, command_array_p );
    
    int ret;
    // Wait UART response
    ret = uartCmdMgr_p->wait();
    if( ret != 0 )
    {
        DEBUG_PRINT( "wifi_off failed\r\n" );
        snic_core_p->freeCmdBuf( payload_buf_p );
        return -1;
    }
    
    if( uartCmdMgr_p->getCommandStatus() != 0 )
    {
        DEBUG_PRINT("wifi_off status:%02x\r\n", uartCmdMgr_p->getCommandStatus());
        snic_core_p->freeCmdBuf( payload_buf_p );
        return -1;
    }
    snic_core_p->freeCmdBuf( payload_buf_p );

    return ret;
}

int C_SNIC_WifiInterface::getRssi( signed char *rssi_p )
{
    C_SNIC_Core               *snic_core_p  = C_SNIC_Core::getInstance();
    C_SNIC_UartCommandManager *uartCmdMgr_p = snic_core_p->getUartCommand();
    if( rssi_p == NULL )
    {
        DEBUG_PRINT("getRssi parameter error\r\n");
        return -1;
    }
    
    // Get buffer for response payload from MemoryPool
    tagMEMPOOL_BLOCK_T *payload_buf_p = snic_core_p->allocCmdBuf();
    if( payload_buf_p == NULL )
    {
        DEBUG_PRINT("getRssi payload_buf_p NULL\r\n");
        return -1;
    }

    C_SNIC_Core::tagWIFI_GET_STA_RSSI_REQ_T req;
    
    // Make request
    req.cmd_sid = UART_CMD_SID_WIFI_GET_STA_RSSI_REQ;
    req.seq     = mUartRequestSeq++;
    
    unsigned char *command_array_p = snic_core_p->getCommandBuf();
    unsigned int   command_len;
    command_len = snic_core_p->preparationSendCommand( UART_CMD_ID_WIFI, req.cmd_sid, (unsigned char *)&req
                        , sizeof(C_SNIC_Core::tagWIFI_GET_STA_RSSI_REQ_T), payload_buf_p->buf, command_array_p );

    int ret;
    // Send uart command request
    snic_core_p->sendUart( command_len, command_array_p );
    
    // Wait UART response
    ret = uartCmdMgr_p->wait();
    if( ret != 0 )
    {
        DEBUG_PRINT( "getRssi failed\r\n" );
        snic_core_p->freeCmdBuf( payload_buf_p );
        return -1;
    }
    
    *rssi_p = (signed char)payload_buf_p->buf[2];

    snic_core_p->freeCmdBuf( payload_buf_p );
    return 0;
}

int C_SNIC_WifiInterface::getWifiStatus( tagWIFI_STATUS_T *status_p)
{
    C_SNIC_Core               *snic_core_p  = C_SNIC_Core::getInstance();
    C_SNIC_UartCommandManager *uartCmdMgr_p = snic_core_p->getUartCommand();

    if( status_p == NULL )
    {
        DEBUG_PRINT("getWifiStatus parameter error\r\n");
        return -1;
    }
    
    // Get buffer for response payload from MemoryPool
    tagMEMPOOL_BLOCK_T *payload_buf_p = snic_core_p->allocCmdBuf();
    if( payload_buf_p == NULL )
    {
        DEBUG_PRINT("getWifiStatus payload_buf_p NULL\r\n");
        return -1;
    }

    C_SNIC_Core::tagWIFI_GET_STATUS_REQ_T req;
    // Make request
    req.cmd_sid = UART_CMD_SID_WIFI_GET_STATUS_REQ;
    req.seq     = mUartRequestSeq++;
    req.interface = 0;
    
    unsigned char *command_array_p = snic_core_p->getCommandBuf();
    unsigned int   command_len;
    command_len = snic_core_p->preparationSendCommand( UART_CMD_ID_WIFI, req.cmd_sid, (unsigned char *)&req
                        , sizeof(C_SNIC_Core::tagWIFI_GET_STATUS_REQ_T), payload_buf_p->buf, command_array_p );

    // Send uart command request
    snic_core_p->sendUart( command_len, command_array_p );
    
    int ret;
    // Wait UART response
    ret = uartCmdMgr_p->wait();
    if( ret != 0 )
    {
        DEBUG_PRINT( "getWifiStatus failed\r\n" );
        snic_core_p->freeCmdBuf( payload_buf_p );
        return -1;
    }
    
    // set status
    status_p->status = (E_WIFI_STATUS)payload_buf_p->buf[2];
    
    // set Mac address
    if( status_p->status != e_STATUS_OFF )
    {
        memcpy( status_p->mac_address, &payload_buf_p->buf[3], BSSID_MAC_LENTH );
    } 

    // set SSID
    if( ( status_p->status == e_STA_JOINED ) || ( status_p->status == e_AP_STARTED ) )
    {
        memcpy( status_p->ssid, &payload_buf_p->buf[9], strlen( (char *)&payload_buf_p->buf[9]) );
    } 

    snic_core_p->freeCmdBuf( payload_buf_p );
    return 0;
}

int C_SNIC_WifiInterface::setIPConfig( bool is_DHCP
    , const char *ip_p, const char *mask_p, const char *gateway_p )
{
    // Parameter check
    if( is_DHCP == false )
    {
        if( (ip_p == NULL) || (mask_p == NULL) ||(gateway_p == NULL) )
        {
            DEBUG_PRINT("setIPConfig parameter error\r\n");
            return -1;
        }            
    }

    C_SNIC_Core               *snic_core_p  = C_SNIC_Core::getInstance();
    C_SNIC_UartCommandManager *uartCmdMgr_p = snic_core_p->getUartCommand();

    // Get buffer for response payload from MemoryPool
    tagMEMPOOL_BLOCK_T *payload_buf_p = snic_core_p->allocCmdBuf();
    if( payload_buf_p == NULL )
    {
        DEBUG_PRINT("setIPConfig payload_buf_p NULL\r\n");
        return -1;
    }

    unsigned char *command_array_p = snic_core_p->getCommandBuf();
    unsigned int  command_len;
    if( is_DHCP == true )
    {
        C_SNIC_Core::tagSNIC_IP_CONFIG_REQ_DHCP_T req;
        // Make request
        req.cmd_sid   = UART_CMD_SID_SNIC_IP_CONFIG_REQ;
        req.seq       = mUartRequestSeq++;
        req.interface = 0;
        req.dhcp      = 1;
        
        // Preparation of command
        command_len = snic_core_p->preparationSendCommand( UART_CMD_ID_SNIC, req.cmd_sid, (unsigned char *)&req
                            , sizeof(C_SNIC_Core::tagSNIC_IP_CONFIG_REQ_DHCP_T), payload_buf_p->buf, command_array_p );
    }
    else
    {
        C_SNIC_Core::tagSNIC_IP_CONFIG_REQ_STATIC_T req;
        // Make request
        req.cmd_sid   = UART_CMD_SID_SNIC_IP_CONFIG_REQ;
        req.seq       = mUartRequestSeq++;
        req.interface = 0;
        req.dhcp      = 0;

        // Set paramter of address
        int addr_temp;
        addr_temp = C_SNIC_UartMsgUtil::addrToInteger( ip_p );
        C_SNIC_UartMsgUtil::convertIntToByteAdday( addr_temp, (char *)req.ip_addr );
        addr_temp = C_SNIC_UartMsgUtil::addrToInteger( mask_p );
        C_SNIC_UartMsgUtil::convertIntToByteAdday( addr_temp, (char *)req.netmask );
        addr_temp = C_SNIC_UartMsgUtil::addrToInteger( gateway_p );
        C_SNIC_UartMsgUtil::convertIntToByteAdday( addr_temp, (char *)req.gateway );

        // Preparation of command
        command_len = snic_core_p->preparationSendCommand( UART_CMD_ID_SNIC, req.cmd_sid, (unsigned char *)&req
                            , sizeof(C_SNIC_Core::tagSNIC_IP_CONFIG_REQ_STATIC_T), payload_buf_p->buf, command_array_p );
    }
    // Send uart command request
    snic_core_p->sendUart( command_len, command_array_p );
    
    int ret;
    // Wait UART response
    ret = uartCmdMgr_p->wait();
    if( ret != 0 )
    {
        DEBUG_PRINT( "setIPConfig failed\r\n" );
        snic_core_p->freeCmdBuf( payload_buf_p );
        return -1;
    }
    
    if( uartCmdMgr_p->getCommandStatus() != 0 )
    {
        DEBUG_PRINT("setIPConfig status:%02x\r\n", uartCmdMgr_p->getCommandStatus());
        snic_core_p->freeCmdBuf( payload_buf_p );
        return -1;
    }

    snic_core_p->freeCmdBuf( payload_buf_p );
    return ret;
}

char* C_SNIC_WifiInterface::getIPAddress() {
    C_SNIC_Core               *snic_core_p  = C_SNIC_Core::getInstance();
    C_SNIC_UartCommandManager *uartCmdMgr_p = snic_core_p->getUartCommand();
    
    snic_core_p->lockAPI();
    // Get local ip address.
    // Get buffer for response payload from MemoryPool
    tagMEMPOOL_BLOCK_T *payload_buf_p = snic_core_p->allocCmdBuf();
    if( payload_buf_p == NULL )
    {
        DEBUG_PRINT("getIPAddress payload_buf_p NULL\r\n");
        snic_core_p->unlockAPI();
        return 0;
    }
 
    C_SNIC_Core::tagSNIC_GET_DHCP_INFO_REQ_T req;
    // Make request
    req.cmd_sid      = UART_CMD_SID_SNIC_GET_DHCP_INFO_REQ;
    req.seq          = mUartRequestSeq++;
    req.interface    = 0;
    
    unsigned char *command_array_p = snic_core_p->getCommandBuf();
    unsigned int  command_len;
    // Preparation of command
    command_len = snic_core_p->preparationSendCommand( UART_CMD_ID_SNIC, req.cmd_sid, (unsigned char *)&req
                            , sizeof(C_SNIC_Core::tagSNIC_GET_DHCP_INFO_REQ_T), payload_buf_p->buf, command_array_p );
    // Send uart command request
    snic_core_p->sendUart( command_len, command_array_p );
    // Wait UART response
    int ret = uartCmdMgr_p->wait();
    if( ret != 0 )
    {
        DEBUG_PRINT( "getIPAddress failed\r\n" );
        snic_core_p->freeCmdBuf( payload_buf_p );
        snic_core_p->unlockAPI();
        return 0;
    }
    
    if( uartCmdMgr_p->getCommandStatus() != UART_CMD_RES_SNIC_SUCCESS )
    {
        snic_core_p->freeCmdBuf( payload_buf_p );
        snic_core_p->unlockAPI();
        return 0;
    }
    
    snic_core_p->freeCmdBuf( payload_buf_p );
    snic_core_p->unlockAPI();
 
    sprintf(ip_addr, "%d.%d.%d.%d\0", payload_buf_p->buf[9], payload_buf_p->buf[10], payload_buf_p->buf[11], payload_buf_p->buf[12]);
 
    return ip_addr;
}
