/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   TcpServer.cpp
 * Author: phamh
 * 
 * Created on November 6, 2018, 10:48 AM
 */
/*************************************************************************************************/
#include "TcpServer.h"
/*************************************************************************************************/
static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
/*************************************************************************************************/
string Random(int length) {
    string str(length,0);
    generate_n(str.begin(), length, alphanum);
    return str;
}
/*************************************************************************************************/
void *TCPSERVERTHREAD(void *threadid) {
    TcpServer* p_server = (TcpServer*)threadid;
    listen(p_server->getSocket(),100);
    while(p_server->isRunning()) {
        struct sockaddr_in cli_addr;
        socklen_t clilen = sizeof(cli_addr);
        int newsockfd = accept(p_server->getSocket(), (struct sockaddr *) &cli_addr, &clilen);
        if(newsockfd < 0) {
            continue;
        }
        Client m_client;
        m_client.Socket = newsockfd;
        m_client.ID = p_server->GetNewID();
    }
    pthread_exit(NULL);
}
/*************************************************************************************************/
void *TCPMYCLIENTTHREAD(void *threadid) {
    TcpServer::MyClient* p_client = (TcpServer::MyClient*)threadid;
    int size = 0;
    char *buffer = new char[TCP_SERVER_LIMIT_DATA];
    while(p_client->isRunning()) {
        int size = read(p_client->Socket, buffer, TCP_SERVER_LIMIT_DATA);
        if(size > 0) {
            p_client->hadData(buffer, size);
        }
        usleep(1);
    }
    pthread_exit(NULL);
}
/*************************************************************************************************/
TcpServer::MyClient::MyClient(TcpServer* m_server, string m_id, int m_socket) {
    this->p_server = m_server;
    this->ID = m_id;
    this->Socket = m_socket;
}
/*************************************************************************************************/
void TcpServer::MyClient::hadData(void*, int) {
    this->p_server->hadData(this, void*, int);
}
/*************************************************************************************************/
void TcpServer::MyClient::disconnect() {
    this->p_server->disconnect(this);
}
/*************************************************************************************************/
bool TcpServer::MyClient::start() {
    pthread_t thread;
    this->running = true;
    if(pthread_create(&thread, NULL, TCPMYCLIENTTHREAD, (void*)(this))) {
        this->running = false;
        return false;
    }
    return true;
}
/*************************************************************************************************/
TcpServer::TcpServer(int port) {
    this->m_port = port;
    this->ListClient.clear();
    this->running = false;
}
/*************************************************************************************************/
TcpServer::~TcpServer() {
}
/*************************************************************************************************/
bool TcpServer::sendData(Client& client,string data) {
    if(send(client.Socket, data.c_str(), strlen(data.c_str()), 0) == -1) {
        return false;
    } else {
        return true;
    }
}
/*************************************************************************************************/
bool TcpServer::sendData(Client& client,void* data, int length) {
    if(send(client.Socket, data, length, 0) == -1) {
        return false;
    } else {
        return true;
    }
}
/*************************************************************************************************/
bool TcpServer::start() {
    this->m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(this->m_socket < 0) {
        return false;
    }
    struct sockaddr_in server_address;
    bzero((char *) &server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(this->m_port);
    if (bind(this->m_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        return false;
    }
    return true;
}
/*************************************************************************************************/
int TcpServer::getSocket() {
    return this->m_socket;
}
bool TcpServer::isRunning() {
    return this->running;
}
/*************************************************************************************************/
string TcpServer::GetNewID() {
    if(this->ListClient.size() > 0) {
        while(1) {
            string str = Random(TCP_SERVER_LENGTH_ID);
            bool flag_loop = false;
            for(int i = 0; i < this->ListClient.size(); i++) {
                if(str.compare(this->ListClient[i].ID) == 0) {
                    flag_loop = true;
                    break;
                }
            }
            if(flag_loop == false) {
                return str;
            }
        }
    } else {
        return Random(TCP_SERVER_LENGTH_ID);
    }
}
/*************************************************************************************************/