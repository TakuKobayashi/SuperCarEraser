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
#include "TCPSocketConnection.h"
#include <cstring>

TCPSocketConnection::TCPSocketConnection()
{
}

TCPSocketConnection::~TCPSocketConnection()
{
}

int TCPSocketConnection::connect( const char *host_p, unsigned short port)
{
    int ret;
    C_SNIC_Core               *snic_core_p  = C_SNIC_Core::getInstance();
    C_SNIC_UartCommandManager *uartCmdMgr_p = snic_core_p->getUartCommand();

    FUNC_IN();
    // Socket create
    ret = createSocket();
    if( ret != 0 )
    {
        DEBUG_PRINT("createSocket error : %d\r\n", ret);
        FUNC_OUT();
        return -1;
    }

    int ip_addr = resolveHostName( host_p );
    //lcd_printf("connect to [%s](%08x)\r\n", host_p, ip_addr);
    if( ( ip_addr == 0) || (ip_addr == -1) )
    {
        DEBUG_PRINT("connect resolveHostName failed\r\n");
        FUNC_OUT();
        return -1;
    }
        
    // Get buffer for response payload from MemoryPool
    tagMEMPOOL_BLOCK_T *payload_buf_p = snic_core_p->allocCmdBuf();
    if( payload_buf_p == NULL )
    {
        DEBUG_PRINT("connect payload_buf_p NULL\r\n");
        FUNC_OUT();
        return -1;
    }
    
    // IP address convert to number from strings.     
//    unsigned int ip_addr = addrToInteger(ip_addr_p);

    // 
    C_SNIC_Core::tagSNIC_TCP_CONNECT_TO_SERVER_REQ_T req;
    // Make request
    req.cmd_sid      = UART_CMD_SID_SNIC_TCP_CONNECT_TO_SERVER_REQ;
    req.seq          = mUartRequestSeq++;
    req.socket_id    = mSocketID;
    
    // set ip addr ( byte order )
    C_SNIC_UartMsgUtil::convertIntToByteAdday( ip_addr, (char *)req.remote_addr );
    req.remote_port[0] = ( (port & 0xFF00) >> 8 );
    req.remote_port[1] = (port & 0xFF);
    req.recv_bufsize[0] = ( (SNIC_UART_RECVBUF_SIZE & 0xFF00) >> 8 );
    req.recv_bufsize[1] = (SNIC_UART_RECVBUF_SIZE & 0xFF);
    req.timeout         = 60;

    unsigned char *command_array_p = snic_core_p->getCommandBuf();
    unsigned int  command_len;
    // Preparation of command
    command_len = snic_core_p->preparationSendCommand( UART_CMD_ID_SNIC, req.cmd_sid, (unsigned char *)&req
                            , sizeof(C_SNIC_Core::tagSNIC_TCP_CONNECT_TO_SERVER_REQ_T), payload_buf_p->buf, command_array_p );

    uartCmdMgr_p->setCommandSID( UART_CMD_SID_SNIC_TCP_CONNECTION_STATUS_IND );

    // Send uart command request
    snic_core_p->sendUart( command_len, command_array_p );

    // Wait UART response
    ret = uartCmdMgr_p->wait();
    if( ret != 0 )
    {
        DEBUG_PRINT( "connect failed\r\n" );
        snic_core_p->freeCmdBuf( payload_buf_p );
        FUNC_OUT();
        return -1;
    }
    
    if( uartCmdMgr_p->getCommandStatus() != UART_CMD_RES_SNIC_CONNECTION_UP )
    {
        DEBUG_PRINT("connect status:%02x\r\n", uartCmdMgr_p->getCommandStatus());
        snic_core_p->freeCmdBuf( payload_buf_p );
        FUNC_OUT();
        return -1;
    }

    snic_core_p->freeCmdBuf( payload_buf_p );

    // Initialize connection information
    C_SNIC_Core::tagCONNECT_INFO_T *con_info_p = snic_core_p->getConnectInfo( mSocketID );
    if( con_info_p->recvbuf_p == NULL )
    {
        DEBUG_PRINT( "create recv buffer[socket:%d]\r\n", mSocketID);
        con_info_p->recvbuf_p = new CircBuffer<char>(SNIC_UART_RECVBUF_SIZE);
    }
    con_info_p->is_connected = true;
    con_info_p->is_received  = false;
    FUNC_OUT();
    return 0;
}

bool TCPSocketConnection::is_connected(void)
{
    C_SNIC_Core                    *snic_core_p  = C_SNIC_Core::getInstance();
    C_SNIC_Core::tagCONNECT_INFO_T *con_info_p = snic_core_p->getConnectInfo( mSocketID );
    return con_info_p->is_connected;
}

int TCPSocketConnection::send(char* data_p, int length)
{
    C_SNIC_Core               *snic_core_p  = C_SNIC_Core::getInstance();
    C_SNIC_UartCommandManager *uartCmdMgr_p = snic_core_p->getUartCommand();

    FUNC_IN();
    // Get buffer for response payload from MemoryPool
    tagMEMPOOL_BLOCK_T *payload_buf_p = snic_core_p->allocCmdBuf();
    if( payload_buf_p == NULL )
    {
        DEBUG_PRINT("connect payload_buf_p NULL\r\n");
        FUNC_OUT();
        return -1;
    }
    
    C_SNIC_Core::tagSNIC_TCP_SEND_FROM_SOCKET_REQ_T req;
    // Make request
    req.cmd_sid       = UART_CMD_SID_SNIC_SEND_FROM_SOCKET_REQ;
    req.seq           = mUartRequestSeq++;
    req.socket_id     = mSocketID;
    req.option        = 0;
    req.payload_len[0]= ( (length & 0xFF00) >> 8 );
    req.payload_len[1]= (length & 0xFF);
    
    int req_size     = sizeof(C_SNIC_Core::tagSNIC_TCP_SEND_FROM_SOCKET_REQ_T);
    char *send_buf_p = getSocketSendBuf();
    memcpy( send_buf_p, &req, req_size );
    memcpy( &send_buf_p[req_size], data_p, length );
    
    unsigned char *command_array_p = snic_core_p->getCommandBuf();
    unsigned int   command_len;
    // Preparation of command
    command_len = snic_core_p->preparationSendCommand( UART_CMD_ID_SNIC, req.cmd_sid, (unsigned char *)send_buf_p
                            , req_size + length, payload_buf_p->buf, command_array_p );

    // Send uart command request
    snic_core_p->sendUart( command_len, command_array_p );

    // Wait UART response
    int ret = uartCmdMgr_p->wait();
    if( ret != 0 )
    {
        DEBUG_PRINT( "send failed\r\n" );
        snic_core_p->freeCmdBuf( payload_buf_p );
        FUNC_OUT();
        return -1;
    }
    
    if( uartCmdMgr_p->getCommandStatus() != UART_CMD_RES_SNIC_SUCCESS )
    {
        DEBUG_PRINT("send status:%02x\r\n", uartCmdMgr_p->getCommandStatus());
        snic_core_p->freeCmdBuf( payload_buf_p );
        FUNC_OUT();
        return -1;
    }
    snic_core_p->freeCmdBuf( payload_buf_p );

    // SNIC_SEND_FROM_SOCKET_REQ
    FUNC_OUT();
    return length;
}

int TCPSocketConnection::send_all(char *data_p, int length)
{
    return send( data_p, length );
}

int TCPSocketConnection::receive(char* data_p, int length)
{
    int i = 0;
    
    FUNC_IN();
    if( (data_p == NULL) || (length < 1) )
    {
        DEBUG_PRINT("TCPSocketConnection::receive parameter error\r\n");
        FUNC_OUT();
        return -1;
    }
    
    C_SNIC_Core                    *snic_core_p  = C_SNIC_Core::getInstance();
    // Initialize connection information
    C_SNIC_Core::tagCONNECT_INFO_T *con_info_p = snic_core_p->getConnectInfo( mSocketID );
    if( con_info_p->recvbuf_p == NULL )
    {
        DEBUG_PRINT("TCPSocketConnection::receive Conncection info error\r\n");
        FUNC_OUT();
        return -1;
    }

    // Check connection
    if( con_info_p->is_connected == false )
    {
        DEBUG_PRINT(" Socket id \"%d\" is not connected\r\n", mSocketID);
        FUNC_OUT();
        return -1;
    }
    con_info_p->is_receive_complete = true;
    if( con_info_p->is_received == false )
    {
        // Try receive
        Thread::yield();
        
        if( con_info_p->is_received == false )
        {
            // No data received.
            FUNC_OUT();
            return 0;
        }        
    }
    // Get packet data from buffer for receive.
    for (i = 0; i < length; i ++) 
    {
        if (con_info_p->recvbuf_p->dequeue(&data_p[i]) == false)
        {
            break;
        }
    }

    if( con_info_p->recvbuf_p->isEmpty() )
    {
        con_info_p->mutex.lock();
        con_info_p->is_received = false;
        con_info_p->mutex.unlock();
    }

    FUNC_OUT();
    return i;
}

int TCPSocketConnection::receive_all(char* data_p, int length)
{
    return receive( data_p, length );
}

void TCPSocketConnection::setAcceptSocket( int socket_id )
{
    FUNC_IN();
    mSocketID = socket_id;
}
