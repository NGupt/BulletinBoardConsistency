#include "article.h"
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
#define BUF_LEN 10
//int sock_s = -1;

thread insert_listen_thread;
int insert_listen_fd;


std::vector <std::string> backupList_IP;
std::vector <int> backupList_port;

char *c_ip;
char *o_ip;
bool NewServer::isCoordinator(string ip) {
    return ip == coordinator_ip ;
}

// int NewServer::send_servers_new(string s_ip, int s_port, const char *servers) {
//   // //std::string temp_ip(s_ip, strlen(s_ip));
//     const char *ip = s_ip.c_str();
//     const char *port = (to_string(s_port)).c_str();

//     struct addrinfo sendaddr;
//     struct addrinfo *res = 0;
//     int bytes = 0;

//     memset(&sendaddr, 0, sizeof(sendaddr));
//     sendaddr.ai_family = AF_UNSPEC;
//     sendaddr.ai_socktype = SOCK_DGRAM;
//     sendaddr.ai_protocol = 0;
//     sendaddr.ai_flags = AI_ADDRCONFIG;

//     if (getaddrinfo(ip, port, &sendaddr, &res) != 0) {
//         perror("addrinfo()");
//     }

//     int sock_s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
//      if (sock_s == -1) {
//        freeaddrinfo(res);
//      perror("socket creation()");
//      }
     
//     if ((bytes=sendto(sock_s, servers, strlen(servers), 0, res->ai_addr, res->ai_addrlen)) == -1) {
//         perror("sendto()");
//         freeaddrinfo(res);
//     }


//     // int recv_len;
//     // char recbuf[BUF_LEN];
//     // recv_len = recvfrom(sock_s, recbuf, BUF_LEN, 0, res->ai_addr,
//     // 			&res->ai_addrlen);
//     //  if (recv_len == -1)
//     //   {
//     //  	freeaddrinfo(res);
//     // 	close(sock_s);
//     //  	return -1;
//     //    }

//     // // //print details of the client/peer and the data received
//     //  printf("Received Data: %s\n" , recv_len);

//     // freeaddrinfo(res);
//     // close(sock_s);
    
//     //freeaddrinfo(res);

//     cout << "sending " << servers << "ip " << ip << s_port << endl;
//     close(sock_s);
//     return 0;
// }

int NewServer::listen_for(NewServer *s,string s_ip, int port){
    struct sockaddr_in si_other, client_addr;
    const char *ip = s_ip.c_str();
    int bytes = 0;
    socklen_t slen = sizeof(si_other);
    //     if ((sock_l = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
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
    // if (bind(s->sock, (struct sockaddr *) &client_addr, sizeof(client_addr)) < 0) {
    //     fprintf(stderr, "could not bind \n");

    // }
    char servers[MAXSERVERS];
    // Clear the buffer by filling null, it might have previously received data
    memset(servers, '\0', MAXSERVERS);

    cout << "bound" <<endl;

    // Try to receive some data; This is a blocking call
    while (1) {
    if (recvfrom(s->insert_listen_fd, servers, MAXSERVERS, 0, (struct sockaddr *) &si_other, &slen) < 0) {
        perror("recvfrom()");
    //    cout << "recv error bytes: " << bytes << endl;
    }
    
    cout << "listened "  << " " << servers <<endl;
    }
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
    c_ip = new char [coordinator_ip.length()+1];
    strcpy (c_ip, coordinator_ip.c_str());
    o_ip = new char [ip.length()+1];
    strcpy (o_ip, ip.c_str());


    sockaddr_in si_me;
    printf("Begin listening to requests from the other servers...\n");
    insert_listen_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (insert_listen_fd == -1)
      {
	printf("Error: creating socket\n");
	throw;
      }


    memset(&si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_addr.s_addr= htonl(INADDR_ANY);

    bool isThisCoord = isCoordinator(o_ip);
    if (isThisCoord)
      {
	// This is the Coordinator
	si_me.sin_port = htons(coordinator_port); // listens to backups
	printf("Coordinator started.\n");
      }
    else
      {
	// This is not the Coordinator

	si_me.sin_port = htons(server_port); // listens to the coordinator
	printf("Backup server started.\n");
      }

    if (bind(insert_listen_fd, (struct sockaddr*)&si_me, sizeof(si_me)) == -1)     {
	close(insert_listen_fd);
	insert_listen_fd = -1;
	printf("Error: binding socket\n");
	throw;
      }
    insert_listen_thread = thread(listen_for, this,c_ip, server_port);
    insert_listen_thread.detach();
     printf("Initialization complete\n");
    //sleep(10);
    //send_servers_new(o_ip, server_port, "aaaaaaaaa");
    //send_servers_new(c_ip, coordinator_port, "aaaaaaaaa");
    
}
    
    // printf("Starting Server\n");

    // sockaddr_in si_me;

    // printf("Begin listening to requests from the other servers...\n");
    // insert_listen_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    // if (insert_listen_fd == -1)
    //   {
    // 	printf("Error: creating socket\n");
    // 	throw;
    //   }

    // memset(&si_me, 0, sizeof(si_me));
    // si_me.sin_family = AF_INET;
    // si_me.sin_addr.s_addr= htonl(INADDR_ANY);
    
    // pclnt = clnt_create (c_ip, COMMUNICATE_PROG, COMMUNICATE_VERSION, "udp");
    //  if (pclnt == NULL) {
    //     clnt_pcreateerror (c_ip);
    //     exit (1);
    //   }
//     int optval = 1;
//     //listen_for(NewServer *s,c_ip,port);

//     sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP) ;
//   if (sock == -1) {

//     printf("Error createing socket");
//     throw;
//   }
//   setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const void *) &optval, sizeof(int));
  
 
//     if (isCoordinator(o_ip)) {
//         cout << "is coordinator" << endl;
//         addToServerList(o_ip, coordinator_port);
//       //  listen_for(this, o_ip, coordinator_port);//server_port
//       sleep(30);
//         send_servers_new(o_ip, server_port, "aaaaaaaaa");

//     } else {
//       //  ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
//       //     perror("socket()");
//       // }
//       //    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const void *) &optval, sizeof(int));
//         addToServerList(c_ip, coordinator_port);//server_port
//         addToServerList(o_ip, server_port);//server_port
//       //  send_servers_new(c_ip, coordinator_port, "aaaaaaaaa"); //send server ka server_list to coordinator //server_port
//         cout << "after sending my list" << endl;
//         //listen_for(this, c_ip, server_port);
//       c_servers = std::thread(listen_for, this, c_ip, server_port);
//      c_servers.detach();
//     }
//     std::cout << ".....Completed Server creation.....\n";
// }

NewServer::~NewServer() {
  if(c_servers.joinable()){
    c_servers.join();
    //close(sock);

  }
}

int NewServer::insert(int art_id, string content)
{
  int result;
  // if this server is the coordinator
  if (isCoordinator(o_ip)){
      // insert message has been received from a client or another server
      // update self
    // result = ArticlePool
      // if (result == 0) {
      // 	  return 0;
      // 	}

      // // tell backups to update
    //   // and wait for acknowledgement

    //   //printf("Updating %s\n",*ip);
    //   //}
       for (int i=0; i<backupList_IP.size(); i++)
     	{
     	  printf("Updating %s:%d\n",backupList_IP[i].c_str(), backupList_port[i]);
     	  result = updateServer(art_id, content, (char*)backupList_IP[i].c_str(), backupList_port[i]);
	   if (result == -1) {
     	      printf("Update to backup failed. %s:%d\n", (char*)backupList_IP[i].c_str(), backupList_port[i]);
     	      return -1;
	   }
	      else{
		// connect to coordinator
		// send to coordinator
		// wait for confirmation that update has completed

		/* Send register msg */
		printf("Updating %s:%d\n",c_ip, coordinator_port);
		result = updateServer(art_id, articleMap[art_id]->content, (char*)c_ip, coordinator_port);
		if (result == -1)
		  {
		    printf("Update to backup failed. %s:%d\n", c_ip, coordinator_port);
		    return -1;
		  }


	      }
	      printf("Insert Complete.\n");
	      // send confirmation to client
	      return result;
       }
  }
}

int NewServer::updateServer(int art_id, string content, char *backup_IP, int backup_port) {

  int result;
  char buf[BUF_LEN];
  snprintf(buf, BUF_LEN, "Insert;%d;%s", art_id, articleMap[art_id]->content);
  
  /* Send register msg */
  
  result = udp_send_confirm(backup_IP, backup_port, buf, BUF_LEN);
  if (result < 0)
    {
      printf("ERROR: Insert: Sending update\n");
      return -1;
    }
  
  return result;
}


int NewServer::udp_send_confirm(const char *ip, int port, const char *buf,
					  const int buf_size)
{
  int fd;
  struct addrinfo hints;
  struct addrinfo* res;
  int recv_len;
	    
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_protocol = 0;
  hints.ai_flags = AI_ADDRCONFIG;

  if (getaddrinfo(ip, std::to_string(static_cast<long long>(port)).c_str(), &hints, &res) != 0)
    {
      return -1;
    }
  
  fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (fd == -1)
    {
      freeaddrinfo(res);
      return -1;
    }
  
  int s = sendto(fd, buf, buf_size, 0, res->ai_addr, res->ai_addrlen);
  if (s == -1)
    {
      freeaddrinfo(res);
      close(fd);
      return -1;
    }

	    /* Begin listening for confirmation of insert */
  printf("INFO: Insert: listening for confirmation\n");
  char recbuf[BUF_LEN];
  recv_len = recvfrom(fd, recbuf, BUF_LEN, 0, res->ai_addr,
		      &res->ai_addrlen);
  if (recv_len == -1)
    {
      freeaddrinfo(res);
      close(fd);
      return -1;
    }

	    //print details of the client/peer and the data received
  printf("Received Data: %s\n" , buf);
  
  freeaddrinfo(res);
  close(fd);

  return s;
}
	  


  // if (pclnt){
  //   clnt_destroy(pclnt);
  // }
//}
