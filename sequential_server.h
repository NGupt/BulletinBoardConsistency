#pragma once

#include "article.h"
#include "communicate.h"
#include "peer.h"
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

using namespace std;
using std::to_string;

class SequentialServer;

class SequentialServer : public ArticlePool {
  string server_ip;
  int server_port;
  string coordinator_ip;
  int coordinator_port;

public:
    int insert_listen_fd;
    bool isCoordinator(string server_ip);
    static void listen_from(SequentialServer *s, string remote_ip, int port);
//    int udp_send_confirm(const char *ip, int port, const char *buf, const int buf_size);
//    int updateServer(int art_id, string content, char *backup_IP, int backup_port);
    static int insert(PeerClient *p, int art_id, string content);
    SequentialServer(string ip, int server_port, string coordinator_ip, int coordinator_port);
    ~SequentialServer();
};
