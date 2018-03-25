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
#include <iterator>
#include <algorithm> //for std::find
#include "exception.h"
#include <sys/time.h>
#include <mutex>

using namespace std;
using std::to_string;
using std::thread;

class PeerClient {

public:
    CLIENT *pclnt; //coordinator
    thread insert_listen_thread;
    thread update_thread;
    int num_confirmations;
    string server_ip;
    int server_port;
    string coordinator_ip;
    int coordinator_port;
    ArticlePool articlePool;
    vector<pair<string, int> > serverList;
    int insert_listen_fd;
    bool isCoordinator(string server_ip);
    static void listen_from(PeerClient *s, string remote_ip, int port);
//    int updateServer(int art_id, string content, char *backup_IP, int backup_port);
    static int updateAllServers(PeerClient *p, ArticlePool articlePool, int reply_index);
    void outputServerList(PeerClient *p);
    ArticlePoolStruct get_article();
    ArticlePoolStruct getLocalArticle();
    server_list buildServerList();
    int post(char * content);
    int send_flag(int flag);
    string read();
    ArticleContent choose(int index);
    int reply(char * content, int index);
    server_list get_server_list();
    int join_server(IP ip, int port);
    int joinServer(string ip, int port);
    int udp_send_confirm(const char *ip, int port, const char *buf, const int buf_size);
    void decode_articles(char *temp_articles);
    PeerClient(string ip, int server_port, string coordinator_ip, int coordinator_port);
    ~PeerClient();




    //Quorum
    vector<pair<int,pair<string,int>> > readQuorumList;
    vector<pair<int,pair<string,int>> > writeQuorumList;

    //first delimited part is READ/WRITE
    int udp_ask_vote(const char* ip, int port, const char* buf, int buf_size);
    //first delimited part is FWD_REQ
    int udp_fwd_req(const char* target_serv_ip, int serv_port, const char* client_ip, int client_port, const char* buf, int buf_size);
    //first delimited part is POOL
    char *udp_get_updated_pool(const char *ip, int port, const char *pool_content, const int buf_size);
    //first delimited part is SYNCHRONIZE
    int udp_synchronize(const char *ip, int port, const char *buf, int buf_size);
    //decodes the first delimited part and then rest of the decoding and response back
    static void udp_recv_vote_req(PeerClient *s,string r_ip, int port);
    int writeVote(ArticlePool art);
    int readVote(ArticlePool articlePool);

private:
    std::mutex subscriber_lock;
    std::mutex crit;
    std::shared_ptr<std::mutex> readlock;
    std::shared_ptr<std::mutex> writelock;
    //  std::thread insert_listen_fd;
    std::vector<PeerClient *> subscribers;
    char *pool_content;
    /* Create lock if it doesn't exist, then try to lock.
     * Returns status*/
    bool readLock(int id);
    bool writeLock(int id);

};

extern PeerClient *now;
