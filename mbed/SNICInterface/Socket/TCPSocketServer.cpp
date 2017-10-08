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
#include "TCPSocketServer.h"
#include "SNIC_Core.h"

#include <cstring>

TCPSocketServer::TCPSocketServer()
{
}

TCPSocketServer::~TCPSocketServer()
{
}

int TCPSocketServer::bind(unsigned short port) 
{
    int ret;
    C_SNIC_Core               *snic_core_p  = C_SNIC_Core::getInstance();
    C_SNIC_UartCommandManager *uartCmdMgr_p = snic_core_p->getUartCommand();
    
    snic_core_p->lockAPI();
    // Get local ip address.
    // Get buffer for response payload from MemoryPool
    tagMEMPOOL_BLOCK_T *payload_buf_p = snic_core_p->allocCmdBuf();
    if( payload_buf_p == NULL )
    {
        DEBUG_PRINT("bind payload_buf_p NULL\r\n");
        snic_core_p->unlockAPI();
        return -1;
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
    ret = uartCmdMgr_p->wait();
    if( ret != 0 )
    {
        DEBUG_PRINT( "bind failed\r\n" );
        snic_core_p->freeCmdBuf( payload_buf_p );
        snic_core_p->unlockAPI();
        return -1;
    }
    
    if( uartCmdMgr_p->getCommandStatus() != UART_CMD_RES_SNIC_SUCCESS )
    {
        DEBUG_PRINT("bind status:%02x\r\n", uartCmdMgr_p->getCommandStatus());
        snic_core_p->freeCmdBuf( payload_buf_p );
        snic_core_p->unlockAPI();
        return -1;
    }
    
    snic_core_p->freeCmdBuf( payload_buf_p );
    snic_core_p->unlockAPI();
    
    unsigned int local_addr = (payload_buf_p->buf[9]  << 24)
                            | (payload_buf_p->buf[10] << 16)
                            | (payload_buf_p->buf[11] << 8)
                            | (payload_buf_p->buf[12]);
   
    // Socket create
    ret = createSocket( 1, local_addr, port );
    if( ret != 0 )
    {
        DEBUG_PRINT("bind error : %d\r\n", ret);
        return -1;
    }

    return 0;
}

int TCPSocketServer::listen(int max)
{
    C_SNIC_Core               *snic_core_p  = C_SNIC_Core::getInstance();
    C_SNIC_UartCommandManager *uartCmdMgr_p = snic_core_p->getUartCommand();
    
    snic_core_p->lockAPI();
    // Get buffer for response payload from MemoryPool
    tagMEMPOOL_BLOCK_T *payload_buf_p = snic_core_p->allocCmdBuf();
    if( payload_buf_p == NULL )
    {
        DEBUG_PRINT("listen payload_buf_p NULL\r\n");
        snic_core_p->unlockAPI();
        return -1;
    }

    C_SNIC_Core::tagSNIC_TCP_CREATE_CONNECTION_REQ_T req;
    // Make request
    req.cmd_sid         = UART_CMD_SID_SNIC_TCP_CREATE_CONNECTION_REQ;
    req.seq             = mUartRequestSeq++;
    req.socket_id       = mSocketID;
    req.recv_bufsize[0] = ( (SNIC_UART_RECVBUF_SIZE & 0xFF00) >> 8 );
    req.recv_bufsize[1] = (SNIC_UART_RECVBUF_SIZE & 0xFF);
    req.max_client      = max;
    
    unsigned char *command_array_p = snic_core_p->getCommandBuf();
    unsigned int  command_len;
    // Preparation of command
    command_len = snic_core_p->preparationSendCommand( UART_CMD_ID_SNIC, req.cmd_sid, (unsigned char *)&req
                        , sizeof(C_SNIC_Core::tagSNIC_TCP_CREATE_CONNECTION_REQ_T), payload_buf_p->buf, command_array_p );

    int ret;
    
    // Send uart command request
    snic_core_p->sendUart( command_len, command_array_p );
    
    // Wait UART response
    ret = uartCmdMgr_p->wait();
    if( ret != 0 )
    {
        DEBUG_PRINT( "listen failed\r\n" );
        snic_core_p->freeCmdBuf( payload_buf_p );
        snic_core_p->unlockAPI();
        return -1;
    }
    
    if( uartCmdMgr_p->getCommandStatus() != 0 )
    {
        DEBUG_PRINT("listen status:%02x\r\n", uartCmdMgr_p->getCommandStatus());
        snic_core_p->freeCmdBuf( payload_buf_p );
        snic_core_p->unlockAPI();
        return -1;
    }

    snic_core_p->freeCmdBuf( payload_buf_p );
    snic_core_p->unlockAPI();
    return 0;
}

int TCPSocketServer::accept(TCPSocketConnection& connection)
{
    C_SNIC_Core *snic_core_p = C_SNIC_Core::getInstance();
    int          i;
    int          ret = -1;
    bool         accepted = false;
    
    C_SNIC_Core::tagCONNECT_INFO_T *con_info_p;
    
    do
    {    
        for( i = 0; i < MAX_SOCKET_ID+1; i++ )
        {
        // Get connection information
            con_info_p = snic_core_p->getConnectInfo( i );
            if( (con_info_p->is_connected == true)
                && (con_info_p->is_accept == true)
                && (con_info_p->parent_socket == mSocketID) )
            {
                // Set socket id
                connection.setAcceptSocket( i );
                ret = 0;
                accepted = true;
                break;
            }   
        }
    } while( accepted == false );
    con_info_p->is_accept = false;
    
    char remote_addr[ 20 ] = {'\0'};
    sprintf( remote_addr, "%d.%d.%d.%d"
            , (con_info_p->from_ip >> 24 ) & 0xff 
            , (con_info_p->from_ip >> 16 ) & 0xff 
            , (con_info_p->from_ip >> 8 )  & 0xff 
            ,  con_info_p->from_ip         & 0xff );
    
    connection.set_address( remote_addr, (int)con_info_p->from_port );
    
    return ret;
}
