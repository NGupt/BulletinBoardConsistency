#include "article.h"
#include "communicate.h"
#include <stdio.h>
#include <stdlib.h>
#include <rpc/pmap_clnt.h>
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
//#include "article.h"

using namespace std;
using std::to_string;

#pragma once



class NewServer;

class NewServer : public ArticlePool {

  string server_ip;
  int server_port;
  string coordinator_ip;
  int coordinator_port;

public:
    int sock;
    CLIENT *pclnt; //coordinator
    vector<pair<string, int> > serverList;
    std::thread c_servers; //thread for coordinator to send server list
    bool isCoordinator(string ip);
    int insert_listen_fd;
    int send_servers_new(string ip, int port, const char *servers);
    static int listen_for(NewServer *s, string ip, int port);
    int addToServerList(string ip, int port);
    NewServer(string ip, int server_port, string coordinator_ip, int coordinator_port);
    ~NewServer();
    int udp_send_confirm(const char *ip, int port, const char *buf, const int buf_size);
    int updateServer(int art_id, string content, char *backup_IP, int backup_port);
    int insert(int art_id, string content);

};
