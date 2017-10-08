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
#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include "Socket.h"
#include "Endpoint.h"

/**
    Interface class for TCP socket of using SNIC UART.
*/
class TCPSocketConnection : public Socket, public Endpoint {
    
public:
    /** TCP socket connection
    */
    TCPSocketConnection();
    virtual ~TCPSocketConnection();
    
    /** Connects this TCP socket to the server
        @param host The strings of ip address.
        @param port The host's port to connect to.
        @return 0 on success, -1 on failure.
        @note   This function is blocked until a returns.
                When you use it by UI thread, be careful. 
    */
    int connect( const char *ip_addr_p, unsigned short port );
    
    /** Check if the socket is connected
        @return true if connected, false otherwise.
    */
    bool is_connected(void);
    
    /** Send data to the remote host.
        @param data The buffer to send to the host.
        @param length The length of the buffer to send.
        @return the number of written bytes on success (>=0) or -1 on failure
     */
    int send(char *data_p, int length);

    /** Send data to the remote host.
        @param data The buffer to send to the host.
        @param length The length of the buffer to send.
        @return the number of written bytes on success (>=0) or -1 on failure
     */
    int send_all(char *data_p, int length);
    
    /** Receive data from the remote host.
        @param data The buffer in which to store the data received from the host.
        @param length The maximum length of the buffer.
        @return the number of received bytes on success (>=0) or -1 on failure
     */
    int receive(char *data_p, int length);
     
    /** Send data to the remote host.
        @param data The buffer to send to the host.
        @param length The length of the buffer to send.
        @return the number of written bytes on success (>=0) or -1 on failure
     */
    int receive_all(char *data_p, int length);
    
    void setAcceptSocket( int socket_id );
private:

};
#endif
