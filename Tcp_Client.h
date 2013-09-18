/*
 * Tcp_Client.h
 *
 *  Created on: Aug 13, 2013
 *      Author: ashish
 */

#ifndef TCP_CLIENT_H_
#define TCP_CLIENT_H_

/**
    C++ client example using sockets
*/
#include<iostream>    //cout
#include<stdio.h> //printf
#include<string.h>    //strlen
#include<string>  //string
#include<sys/socket.h>    //socket
#include<arpa/inet.h> //inet_addr
#include<netdb.h> //hostent

#include <stdlib.h>
#include <unistd.h>

using namespace std;
/**
    TCP Client class
*/
class tcp_client
{
private:
    int sock;
    std::string address;
    int port;
    struct sockaddr_in server;

public:
    tcp_client();
    bool conn(string, int);
    bool send_data(string data);
    string receive(int);
    void closeSocket();
};


#endif /* TCP_CLIENT_H_ */
