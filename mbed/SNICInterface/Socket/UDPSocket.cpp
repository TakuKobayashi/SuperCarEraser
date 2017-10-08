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
#include "Socket/UDPSocket.h"
#include <cstring>

UDPSocket::UDPSocket() {
}

UDPSocket::~UDPSocket()
{
}

int UDPSocket::init(void) 
{
    return 0;
}

// Server initialization
int UDPSocket::bind(short port) 
{
    int ret;
    C_SNIC_Core               *snic_core_p  = C_SNIC_Core::getInstance();
    C_SNIC_UartCommandManager *uartCmdMgr_p = snic_core_p->getUartCommand();
    
    FUNC_IN();
    // Get local ip address.
    // Get buffer for response payload from MemoryPool
    tagMEMPOOL_BLOCK_T *payload_buf_p = snic_core_p->allocCmdBuf();
    if( payload_buf_p == NULL )
    {
        DEBUG_PRINT("UDP bind payload_buf_p NULL\r\n");
        FUNC_OUT();
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
        DEBUG_PRINT( "UDP bind failed\r\n" );
        snic_core_p->freeCmdBuf( payload_buf_p );
        FUNC_OUT();
        return -1;
    }
    
    if( uartCmdMgr_p->getCommandStatus() != UART_CMD_RES_SNIC_SUCCESS )
    {
        DEBUG_PRINT("UDP bind status:%02x\r\n", uartCmdMgr_p->getCommandStatus());
        snic_core_p->freeCmdBuf( payload_buf_p );
        FUNC_OUT();
        return -1;
    }
    
    unsigned int local_addr = (payload_buf_p->buf[9]  << 24)
                            | (payload_buf_p->buf[10] << 16)
                            | (payload_buf_p->buf[11] << 8)
                            | (payload_buf_p->buf[12]);


    C_SNIC_Core::tagSNIC_UDP_CREATE_SOCKET_REQ_T create_req;
    
    // Make request
    create_req.cmd_sid  = UART_CMD_SID_SNIC_UDP_CREATE_SOCKET_REQ;
    create_req.seq      = mUartRequestSeq++;
    create_req.bind     = 1;
    // set ip addr ( byte order )
    C_SNIC_UartMsgUtil::convertIntToByteAdday( local_addr, (char *)create_req.local_addr );
    create_req.local_port[0] = ( (port & 0xFF00) >> 8 );
    create_req.local_port[1] = (port & 0xFF);

    // Preparation of command
    command_len = snic_core_p->preparationSendCommand( UART_CMD_ID_SNIC, create_req.cmd_sid, (unsigned char *)&create_req
                            , sizeof(C_SNIC_Core::tagSNIC_UDP_CREATE_SOCKET_REQ_T), payload_buf_p->buf, command_array_p );
    // Send uart command request
    snic_core_p->sendUart( command_len, command_array_p );

    // Wait UART response
    ret = uartCmdMgr_p->wait();
    if( ret != 0 )
    {
        DEBUG_PRINT( "UDP bind failed\r\n" );
        snic_core_p->freeCmdBuf( payload_buf_p );
        FUNC_OUT();
        return -1;
    }

    if( uartCmdMgr_p->getCommandStatus() != 0 )
    {
        DEBUG_PRINT("UDP bind status:%02x\r\n", uartCmdMgr_p->getCommandStatus());
        snic_core_p->freeCmdBuf( payload_buf_p );
        FUNC_OUT();
        return -1;
    }
    mSocketID = payload_buf_p->buf[3];
    
    // Initialize connection information
    C_SNIC_Core::tagCONNECT_INFO_T *con_info_p = snic_core_p->getConnectInfo( mSocketID );
    if( con_info_p->recvbuf_p == NULL )
    {
        DEBUG_PRINT( "create recv buffer[socket:%d]\r\n", mSocketID);
        con_info_p->recvbuf_p = new CircBuffer<char>(SNIC_UART_RECVBUF_SIZE);
    }
    con_info_p->is_connected = true;
    con_info_p->is_received  = false;

    C_SNIC_Core::tagSNIC_UDP_START_RECV_REQ_T recv_start_req;
    
    // Make request
    recv_start_req.cmd_sid         = UART_CMD_SID_SNIC_UDP_START_RECV_REQ;
    recv_start_req.seq             = mUartRequestSeq++;
    recv_start_req.socket_id       = mSocketID;
    recv_start_req.recv_bufsize[0] = ( (SNIC_UART_RECVBUF_SIZE & 0xFF00) >> 8 );
    recv_start_req.recv_bufsize[1] = (SNIC_UART_RECVBUF_SIZE & 0xFF);

    // Preparation of command
    command_len = snic_core_p->preparationSendCommand( UART_CMD_ID_SNIC, recv_start_req.cmd_sid, (unsigned char *)&recv_start_req
                            , sizeof(C_SNIC_Core::tagSNIC_UDP_START_RECV_REQ_T), payload_buf_p->buf, command_array_p );
    // Send uart command request
    snic_core_p->sendUart( command_len, command_array_p );

    // Wait UART response
    ret = uartCmdMgr_p->wait();
    if( ret != 0 )
    {
        DEBUG_PRINT( "UDP recv start failed\r\n" );
        snic_core_p->freeCmdBuf( payload_buf_p );
        FUNC_OUT();
        return -1;
    }

    if( uartCmdMgr_p->getCommandStatus() != 0 )
    {
        DEBUG_PRINT("UDP recv start status:%02x\r\n", uartCmdMgr_p->getCommandStatus());
        snic_core_p->freeCmdBuf( payload_buf_p );
        FUNC_OUT();
        return -1;
    }

    snic_core_p->freeCmdBuf( payload_buf_p );
    FUNC_OUT();
    return 0;
}

// -1 if unsuccessful, else number of bytes written
int UDPSocket::sendTo(Endpoint &remote, char *packet, int length)
{
    C_SNIC_Core               *snic_core_p  = C_SNIC_Core::getInstance();
    C_SNIC_UartCommandManager *uartCmdMgr_p = snic_core_p->getUartCommand();

    osThreadId tid = Thread::gettid();
    
//  pc.printf("send[%08x] len:%d(%04x)\r\n", tid, length, length);
    
#if 0   // TODO: Not wait for command response(Tentative)    
    snic_core_p->lockAPI();
#endif
    FUNC_IN();

#if 0   // TODO: Not wait for command response(Tentative)    
    // Get buffer for response payload from MemoryPool
    tagMEMPOOL_BLOCK_T *payload_buf_p = snic_core_p->allocCmdBuf();
    if( payload_buf_p == NULL )
    {
        DEBUG_PRINT("connect payload_buf_p NULL\r\n");
        FUNC_OUT();
        snic_core_p->unlockAPI();
        return -1;
    }
#endif

    C_SNIC_Core::tagSNIC_UDP_SEND_FROM_SOCKET_REQ_T req;
    // Make request
    req.cmd_sid       = UART_CMD_SID_SNIC_UDP_SEND_FROM_SOCKET_REQ;
    req.seq           = mUartRequestSeq++;

    int addr_temp;
    addr_temp = C_SNIC_UartMsgUtil::addrToInteger( remote.get_address() );
    C_SNIC_UartMsgUtil::convertIntToByteAdday( addr_temp, (char *)req.remote_ip );
    req.remote_port[0]  = ( (remote.get_port() & 0xFF00) >> 8 );
    req.remote_port[1]  = (remote.get_port() & 0xFF);
    req.payload_len[0]  = ( (length & 0xFF00) >> 8 );
    req.payload_len[1]  = (length & 0xFF);
    req.socket_id       = mSocketID;
    req.connection_mode = 1;
    
    // Initialize connection information
    C_SNIC_Core::tagCONNECT_INFO_T *con_info_p = snic_core_p->getConnectInfo( mSocketID );
    if( con_info_p != NULL )
    {
        con_info_p->from_ip   = addr_temp;
        con_info_p->from_port = remote.get_port();
    }

    int req_size = sizeof(C_SNIC_Core::tagSNIC_UDP_SEND_FROM_SOCKET_REQ_T);
    
    char *send_buf_p = getSocketSendBuf();
    memcpy( send_buf_p, &req, req_size );
    memcpy( &send_buf_p[req_size], packet, length );
    
    unsigned char *command_array_p = snic_core_p->getCommandBuf();
    unsigned int   command_len;

    // Make all command request
    command_len = C_SNIC_UartMsgUtil::makeRequest( UART_CMD_ID_SNIC, (unsigned char *)send_buf_p, req_size + length, command_array_p );

    // Send uart command request
    snic_core_p->sendUart( command_len, command_array_p );

#if 0   // TODO: Not wait for command response(Tentative)
    // Wait UART response
    int ret = uartCmdMgr_p->wait();
    if( ret != 0 )
    {
        DEBUG_PRINT( "send failed\r\n" );
        snic_core_p->freeCmdBuf( payload_buf_p );
        FUNC_OUT();
        snic_core_p->unlockAPI();
        return -1;
    }
    
    if( uartCmdMgr_p->getCommandStatus() != UART_CMD_RES_SNIC_SUCCESS )
    {
        DEBUG_PRINT("send status:%02x\r\n", uartCmdMgr_p->getCommandStatus());
        snic_core_p->freeCmdBuf( payload_buf_p );
        FUNC_OUT();
        snic_core_p->unlockAPI();
        return -1;
    }
    snic_core_p->freeCmdBuf( payload_buf_p );
#endif
    
    FUNC_OUT();
#if 0   // TODO: Not wait for command response(Tentative)    
    snic_core_p->unlockAPI();
#endif
    // SNIC_SEND_FROM_SOCKET_REQ
    wait(0.05);

    return length;
//    return 0;
}

// -1 if unsuccessful, else number of bytes received
int UDPSocket::receiveFrom(Endpoint &remote, char *data_p, int length)
{
    FUNC_IN();
    if( (data_p == NULL) || (length < 1) )
    {
        DEBUG_PRINT("UDPSocket::receiveFrom parameter error\r\n");
        FUNC_OUT();
        return -1;
    }

    C_SNIC_Core                    *snic_core_p  = C_SNIC_Core::getInstance();
    // Initialize connection information
    C_SNIC_Core::tagCONNECT_INFO_T *con_info_p = snic_core_p->getConnectInfo( mSocketID );
    if( con_info_p->recvbuf_p == NULL )
    {
        DEBUG_PRINT("UDPSocket::receiveFrom Conncection info error\r\n");
        FUNC_OUT();
        return -1;
    }

    char remote_ip[20] = {'\0'};
    sprintf( remote_ip, "%d.%d.%d.%d"
        , (con_info_p->from_ip >>24) & 0x000000ff
        , (con_info_p->from_ip >>16) & 0x000000ff
        , (con_info_p->from_ip >>8)  & 0x000000ff
        , (con_info_p->from_ip)      & 0x000000ff );
    remote.set_address( remote_ip, con_info_p->from_port );
    
    con_info_p->mutex.lock();
    con_info_p->is_receive_complete = true;
    con_info_p->mutex.unlock();
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
    int i;
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
