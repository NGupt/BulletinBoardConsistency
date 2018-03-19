#pragma once

#include "article.h"
#include "communicate.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include <cstring>
#include <netdb.h>
#include <unistd.h>
#include <set>
#include <thread>
#include <vector>
#include <sys/types.h>
#include <map>


class PeerClient {

public:
    CLIENT *pclnt; //coordinator
    string server_ip;
    int server_port;
    string coordinator_ip;
    int coordinator_port;
    ArticlePool articlePool;
    vector<pair<string, int>> serverList;
    int insert_listen_fd;
    bool isCoordinator(string server_ip);
    static void listen_from(PeerClient *s, string remote_ip, int port);
//    int udp_send_confirm(const char *ip, int port, const char *buf, const int buf_size);
//    int updateServer(int art_id, string content, char *backup_IP, int backup_port);
    static int insert(PeerClient *p, int art_id, string content);
    ArticlePoolStruct get_article();
    ArticlePoolStruct getLocalArticle();
    server_list buildServerList();
    int post(char * content);
    string read();
    ArticleContent choose(int index);
    int reply(char * content, int index);
    server_list get_server_list();
    int join_server(IP ip, int port);
    int joinServer(string ip, int port);
    int udp_send_confirm(const char *ip, int port, const char *buf, const int buf_size);
    int updateServer(int art_id, string content, char *backup_IP, int backup_port);
    PeerClient(string ip, int server_port, string coordinator_ip, int coordinator_port);
    ~PeerClient();
};
