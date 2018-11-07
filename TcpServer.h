/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   TcpServer.h
 * Author: phamh
 *
 * Created on November 6, 2018, 10:48 AM
 */
/*************************************************************************************************/
#ifndef TCPSERVER_H
#define TCPSERVER_H
/*************************************************************************************************/
#include <stdio.h>
#include <vector>
#include <random>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <cstring>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
/*************************************************************************************************/
using namespace std;
/*************************************************************************************************/
#ifndef TCP_SERVER_LIMIT_DATA
#define TCP_SERVER_LIMIT_DATA 64*1024
#endif
#ifndef TCP_SERVER_LENGTH_ID
#define TCP_SERVER_LENGTH_ID 10
#endif
/*************************************************************************************************/
class Client {
public:
    string ID;
    int Socket;
    Client();
    virtual ~Client();
};
/*************************************************************************************************/
class TcpServer {
public:
    class MyClient:public Client{
    public:
        TcpServer* p_server;
        /*********************************************************************************************/
        MyClient(TcpServer*, string, int);
        virtual ~MyClient();
        /*********************************************************************************************/
        void hadData(void*, int);
        /*********************************************************************************************/
        void disconnect();
        /*********************************************************************************************/
        bool isRunning();
        /*********************************************************************************************/
        bool start();
        void stop();
        /*********************************************************************************************/
    private:
        bool running;
    };
    TcpServer(int);
    virtual ~TcpServer();
    /*********************************************************************************************/
    vector<Client> ListClient;
    /*********************************************************************************************/
    bool start();
    void stop();
    /*********************************************************************************************/
    bool sendData(Client&,string);
    bool sendData(Client&,void*, int);
    /*********************************************************************************************/
    virtual void hadData(Client&, void*, int) = 0;
    virtual void disconnect(Client&) = 0;
    /*********************************************************************************************/
    int getSocket();
    bool isRunning();
    /*********************************************************************************************/
    string GetNewID();
    /*********************************************************************************************/
private:
    unsigned int m_port;
    int m_socket;
    bool running;
    /*********************************************************************************************/
};
/*************************************************************************************************/
#endif /* TCPSERVER_H */
/*************************************************************************************************/
