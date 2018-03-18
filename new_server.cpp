#include "new_server.h"
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
using namespace std;
using std::to_string;
using std::thread;

int sock;
#define MAXSERVERS 10
//int sock_s = -1;

bool NewServer::isCoordinator(string ip) {
    return ip == coordinator_ip ;
}

int NewServer::send_servers_new(string s_ip, int s_port, const char *servers) {
  // //std::string temp_ip(s_ip, strlen(s_ip));
    const char *ip = s_ip.c_str();
    const char *port = (to_string(s_port)).c_str();

    struct addrinfo sendaddr;
    struct addrinfo *res = 0;
    int bytes = 0;

    memset(&sendaddr, 0, sizeof(sendaddr));
    sendaddr.ai_family = AF_UNSPEC;
    sendaddr.ai_socktype = SOCK_DGRAM;
    sendaddr.ai_protocol = 0;

    if (getaddrinfo(ip, port, &sendaddr, &res) != 0) {
        perror("addrinfo()");
    }

    int sock_s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
     if (sock_s == -1) {
     perror("socket creation()");
       freeaddrinfo(res);
     }
    if ((bytes=sendto(sock_s, servers, strlen(servers), 0, res->ai_addr, res->ai_addrlen)) == -1) {
        perror("sendto()");
        freeaddrinfo(res);
    }
    //freeaddrinfo(res);

    cout << "sending " << servers << "ip " << ip << s_port << endl;
    close(sock_s);
    return 0;
}

int NewServer::listen_for(NewServer *s,string s_ip, int port){
    struct sockaddr_in si_other, client_addr;
    const char *ip = s_ip.c_str();
    int bytes = 0;
    socklen_t slen = sizeof(si_other);
    // if ((sock_l = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    //     perror("socket()");
    // }
    // setsockopt(sock_l, SOL_SOCKET, SO_REUSEADDR, (const void *) &optval, sizeof(int));

    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(port);
    if (inet_aton(ip, &si_other.sin_addr) == 0) {
        fprintf(stderr, "inet_aton failed\n");
    }

    bzero((char *) &client_addr, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    client_addr.sin_port = htons((unsigned short) port);
    if (bind(s->sock, (struct sockaddr *) &client_addr, sizeof(client_addr)) < 0) {
        fprintf(stderr, "could not bind \n");

    }
    char servers[MAXSERVERS];
    // Clear the buffer by filling null, it might have previously received data
    memset(servers, '\0', MAXSERVERS);

    cout << "bound" <<endl;

    // Try to receive some data; This is a blocking call
    if (recvfrom(s->sock, servers, MAXSERVERS, 0, (struct sockaddr *) &si_other, &slen) < 0) {
        perror("recvfrom()");
    //    cout << "recv error bytes: " << bytes << endl;
    }

    cout << "listened "  << " " << servers <<endl;
    return 0;
}


int NewServer::addToServerList(string ip, int port) {
    serverList.push_back(make_pair(ip, port));
    return 1;
}

NewServer::NewServer(string ip, int server_port, string coordinator_ip, int coordinator_port){
    this->server_ip = ip;
    this->server_port = server_port;
    this->coordinator_ip = coordinator_ip;
    this->coordinator_port = coordinator_port;
    char * c_ip = new char [coordinator_ip.length()+1];
    strcpy (c_ip, coordinator_ip.c_str());
    char * o_ip = new char [ip.length()+1];
    strcpy (o_ip, ip.c_str());

    // pclnt = clnt_create (c_ip, COMMUNICATE_PROG, COMMUNICATE_VERSION, "udp");
    //  if (pclnt == NULL) {
    //     clnt_pcreateerror (c_ip);
    //     exit (1);
    //   }
    int optval = 1;

    if (isCoordinator(o_ip)) {
        cout << "is coordinator" << endl;
        addToServerList(o_ip, coordinator_port);
      //  listen_for(this, o_ip, coordinator_port);//server_port
      sleep(30);
        send_servers_new(o_ip, server_port, "aaaaaaaaa");

    } else {
      if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
          perror("socket()");
      }
      setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const void *) &optval, sizeof(int));
        addToServerList(c_ip, coordinator_port);//server_port
        addToServerList(o_ip, server_port);//server_port
      //  send_servers_new(c_ip, coordinator_port, "aaaaaaaaa"); //send server ka server_list to coordinator //server_port
        cout << "after sending my list" << endl;
        //listen_for(this, c_ip, server_port);
      c_servers = std::thread(listen_for, this, c_ip, server_port);
     c_servers.detach();
    }
    std::cout << ".....Completed Server creation.....\n";
}

NewServer::~NewServer() {
  if(c_servers.joinable()){
   c_servers.join();
    close(sock);

  }
}
  // if (pclnt){
  //   clnt_destroy(pclnt);
  // }
//}
