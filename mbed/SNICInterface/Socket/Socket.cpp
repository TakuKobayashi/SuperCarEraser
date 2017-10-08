/* Copyright (C) 2012 mbed.org, MIT License
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
/* Copyright (C) 2014 Murata Manufacturing Co.,Ltd., MIT License
 *  port to the muRata, SWITCH SCIENCE Wi-FI module TypeYD SNIC-UART.
 */
#include "Socket.h"
#include <cstring>

char gSOCKET_SEND_BUF[2048] __attribute__((section("AHBSRAM1")));

Socket::Socket()
{
    mSocketID = -1;
}

void Socket::set_blocking(bool blocking, unsigned int timeout) {
    _blocking = blocking;
    _timeout = timeout;
}

Socket::~Socket() {
//    close(); //Don't want to leak
}

int Socket::set_option(int level, int optname, const void *optval, socklen_t optlen) {
    return 0;
}

int Socket::get_option(int level, int optname, void *optval, socklen_t *optlen) {
    return 0;
}

int Socket::close(bool shutdown)
{
    C_SNIC_Core               *snic_core_p  = C_SNIC_Core::getInstance();
    C_SNIC_UartCommandManager *uartCmdMgr_p = snic_core_p->getUartCommand();
    
    FUNC_IN();
    // Get buffer for response payload from MemoryPool
    tagMEMPOOL_BLOCK_T *payload_buf = snic_core_p->allocCmdBuf();
    if( payload_buf == NULL )
    {
        DEBUG_PRINT("socket close payload_buf NULL\r\n");
        FUNC_OUT();
        return -1;
    }

    C_SNIC_Core::tagSNIC_CLOSE_SOCKET_REQ_T req;
    
    // Make request
    req.cmd_sid   = UART_CMD_SID_SNIC_CLOSE_SOCKET_REQ;
    req.seq       = mUartRequestSeq++;
    req.socket_id = mSocketID;

    unsigned char *command_array_p = snic_core_p->getCommandBuf();
    unsigned int  command_len;
    // Preparation of command
    command_len = snic_core_p->preparationSendCommand( UART_CMD_ID_SNIC, req.cmd_sid, (unsigned char *)&req
                            , sizeof(C_SNIC_Core::tagSNIC_CLOSE_SOCKET_REQ_T), payload_buf->buf, command_array_p );

    // Send uart command request
    snic_core_p->sendUart( command_len, command_array_p );

    int ret;
    // Wait UART response
    ret = uartCmdMgr_p->wait();
    if( ret != 0 )
    {
        DEBUG_PRINT( "socket close failed\r\n" );
        snic_core_p->freeCmdBuf( payload_buf );
        FUNC_OUT();
        return -1;
    }
    
    if( uartCmdMgr_p->getCommandStatus() != 0 )
    {
        DEBUG_PRINT("socket close status:%02x\r\n", uartCmdMgr_p->getCommandStatus());
        snic_core_p->freeCmdBuf( payload_buf );
        FUNC_OUT();
        return -1;
    }
    snic_core_p->freeCmdBuf( payload_buf );
    FUNC_OUT();
    return 0;
}

int Socket::createSocket( unsigned char bind, unsigned int local_addr, unsigned short port )
{
    C_SNIC_Core               *snic_core_p  = C_SNIC_Core::getInstance();
    C_SNIC_UartCommandManager *uartCmdMgr_p = snic_core_p->getUartCommand();
    
    FUNC_IN();
    // Get buffer for response payload from MemoryPool
    tagMEMPOOL_BLOCK_T *payload_buf = snic_core_p->allocCmdBuf();
    if( payload_buf == NULL )
    {
        DEBUG_PRINT("createSocket payload_buf NULL\r\n");
        FUNC_OUT();
        return -1;
    }

    C_SNIC_Core::tagSNIC_TCP_CREATE_SOCKET_REQ_T req;
    int req_len = 0;
    
    // Make request
    req.cmd_sid  = UART_CMD_SID_SNIC_TCP_CREATE_SOCKET_REQ;
    req_len++;
    req.seq      = mUartRequestSeq++;
    req_len++;
    req.bind     = bind;
    req_len++;
    if( bind != 0 )
    {
        // set ip addr ( byte order )
        C_SNIC_UartMsgUtil::convertIntToByteAdday( local_addr, (char *)req.local_addr );
        req.local_port[0] = ( (port & 0xFF00) >> 8 );
        req.local_port[1] = (port & 0xFF);

        req_len = sizeof(C_SNIC_Core::tagSNIC_TCP_CREATE_SOCKET_REQ_T);
    }

    unsigned char *command_array_p = snic_core_p->getCommandBuf();
    unsigned int  command_len;
    // Preparation of command
    command_len = snic_core_p->preparationSendCommand( UART_CMD_ID_SNIC, req.cmd_sid, (unsigned char *)&req
                            , req_len, payload_buf->buf, command_array_p );
    // Send uart command request
    snic_core_p->sendUart( command_len, command_array_p );

    int ret;
    // Wait UART response
    ret = uartCmdMgr_p->wait();
    if( ret != 0 )
    {
        DEBUG_PRINT( "createSocket failed\r\n" );
        snic_core_p->freeCmdBuf( payload_buf );
        FUNC_OUT();
        return -1;
    }

    if( uartCmdMgr_p->getCommandStatus() != 0 )
    {
        DEBUG_PRINT("createSocket status:%02x\r\n", uartCmdMgr_p->getCommandStatus());
        snic_core_p->freeCmdBuf( payload_buf );
        FUNC_OUT();
        return -1;
    }
    mSocketID = payload_buf->buf[3];
    snic_core_p->freeCmdBuf( payload_buf );
    FUNC_OUT();
    return 0;
}

int Socket::resolveHostName( const char *host_p )
{
    C_SNIC_Core               *snic_core_p  = C_SNIC_Core::getInstance();
    C_SNIC_UartCommandManager *uartCmdMgr_p = snic_core_p->getUartCommand();
    int ip_addr = 0;

    if( host_p == NULL )
    {
        DEBUG_PRINT("resolveHostName parameter error\r\n");
        return -1;
    }
    
    // Get buffer for response payload from MemoryPool
    tagMEMPOOL_BLOCK_T *payload_buf = snic_core_p->allocCmdBuf();
    if( payload_buf == NULL )
    {
        DEBUG_PRINT("resolveHostName payload_buf NULL\r\n");
        return -1;
    }
    
    unsigned char *buf_p = (unsigned char *)getSocketSendBuf();
    unsigned int  buf_len = 0;

    memset( buf_p, 0, UART_REQUEST_PAYLOAD_MAX );
    // Make request
    buf_p[0] = UART_CMD_SID_SNIC_RESOLVE_NAME_REQ;
    buf_len++;
    buf_p[1] = mUartRequestSeq++;
    buf_len++;
    // Interface 
    buf_p[2] = 0;
    buf_len++;
    
    // Host name length
    int hostname_len = strlen(host_p);
    buf_p[3] = (unsigned char)hostname_len;
    buf_len++;
    memcpy( &buf_p[4], host_p, hostname_len );
    buf_len += hostname_len;
    
    unsigned char *command_array_p = snic_core_p->getCommandBuf();
    unsigned int   command_len;
    command_len = snic_core_p->preparationSendCommand( UART_CMD_ID_SNIC, UART_CMD_SID_SNIC_RESOLVE_NAME_REQ, buf_p
                        , buf_len, payload_buf->buf, command_array_p );
    // Send uart command request
    snic_core_p->sendUart( command_len, command_array_p );
    
    int ret;
    // Wait UART response
    ret = uartCmdMgr_p->wait();
    if( ret != 0 )
    {
        DEBUG_PRINT( "resolveHostName failed\r\n" );
        snic_core_p->freeCmdBuf( payload_buf );
        return -1;
    }
    
    // check status
    if( uartCmdMgr_p->getCommandStatus() == 0 )
    {
        ip_addr = ((payload_buf->buf[3] << 24) & 0xFF000000)
                | ((payload_buf->buf[4] << 16) & 0xFF0000)
                | ((payload_buf->buf[5] << 8)  & 0xFF00)
                | (payload_buf->buf[6]);
    }

    snic_core_p->freeCmdBuf( payload_buf );
    return ip_addr;
}

char *Socket::getSocketSendBuf()
{
    return gSOCKET_SEND_BUF;
}
