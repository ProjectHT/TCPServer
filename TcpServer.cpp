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
static int alphanumLength = sizeof(alphanum) - 1;
/*************************************************************************************************/
#ifndef LOG_TAG_TCP_CLIENT_OF_SERVER
#define LOG_TAG_TCP_CLIENT_OF_SERVER "Log::Client : "
#endif
/*************************************************************************************************/
#ifndef LOG_TAG_TCP_SERVER
#define LOG_TAG_TCP_SERVER "Log::Server : "
#endif
/*************************************************************************************************/
char genRandom() {
    return alphanum[rand() % alphanumLength];
}
/*************************************************************************************************/
string Random(int length) {
    srand(time(0));
    string str;
    for(unsigned int i = 0; i < length; ++i) {
        str += genRandom();
    }
    return str;
}
/*************************************************************************************************/
void *TCPSERVERTHREAD(void *threadid) {
    TcpServer* p_server = (TcpServer*)threadid;
    listen(p_server->getSocket(),100);
    while(p_server->isRunning()) {
        cout << LOG_TAG_TCP_SERVER << "Wait new client connect to server" << endl;
        struct sockaddr_in cli_addr;
        socklen_t clilen = sizeof(cli_addr);
        int newsockfd = accept(p_server->getSocket(), (struct sockaddr *) &cli_addr, &clilen);
        if(newsockfd < 0) {
            continue;
        }
        p_server->ListClient.push_back(TcpServer::MyClient(p_server, p_server->GetNewID(), newsockfd));
        int t_index = p_server->ListClient.size() - 1;
        if(p_server->ListClient.at(t_index).start()) {
            cout << LOG_TAG_TCP_SERVER << "Add new client with id[" << p_server->ListClient.at(t_index).ID << "]" << endl;
        } else {
            p_server->ListClient.erase(p_server->ListClient.begin()+t_index);
        }
        cout << LOG_TAG_TCP_SERVER << "Number client : " << p_server->ListClient.size() << endl;
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
    p_client->disconnect();
    close(p_client->Socket);
    for(int i = 0; i < p_client->p_server->ListClient.size(); i++) {
        if(p_client->ID.compare(p_client->p_server->ListClient.at(i).ID) == 0) {
            p_client->p_server->ListClient.erase(p_client->p_server->ListClient.begin()+i);
            break;
        }
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
TcpServer::MyClient::~MyClient() {
    if(this->running) {
        stop();
        sleep(5);
    }
}
/*************************************************************************************************/
void TcpServer::MyClient::hadData(void* data, int length) {
    cout << LOG_TAG_TCP_CLIENT_OF_SERVER << "Had msg from id[" << this->ID << "]" << endl;
    this->p_server->hadData(this, data, length);
}
/*************************************************************************************************/
void TcpServer::MyClient::disconnect() {
    cout << LOG_TAG_TCP_CLIENT_OF_SERVER << "Disconnect with id[" << this->ID << "]" << endl;
    this->p_server->disconnect(this);
}
/*************************************************************************************************/
bool TcpServer::MyClient::start() {
    if(this->running == false) {
        pthread_t thread;
        this->running = true;
        if(pthread_create(&thread, NULL, TCPMYCLIENTTHREAD, (void*)(this))) {
            this->running = false;
            return false;
        }
    }
    return true;
}
/*************************************************************************************************/
void TcpServer::MyClient::stop() {
    if(this->running) {
        this->running = false;
    }
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
bool TcpServer::sendData(MyClient* t_client,string data) {
    if(send(t_client->Socket, data.c_str(), strlen(data.c_str()), 0) == -1) {
        return false;
    } else {
        return true;
    }
}
/*************************************************************************************************/
bool TcpServer::sendData(MyClient* t_client,void* data, int length) {
    if(send(t_client->Socket, data, length, 0) == -1) {
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
/*************************************************************************************************/
bool TcpServer::isRunning() {
    return this->running;
}
/*************************************************************************************************/
void TcpServer::stopClient(MyClient* t_client) {
    t_client->stop();
    for(int i = 0; i < ListClient.size(); i++) {
        if(t_client->ID.compare(ListClient.at(i).ID) == 0) {
            if(ListClient.at(i).isRunning()) {
                ListClient.at(i).stop();
                break;
            }
        }
    }
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
