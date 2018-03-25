#pragma once

#include "article.h"
#include "peer.h"
//#include "quorum.h"
#include "communicate.h"
#include <vector>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <thread>

class QuoServer{

public :
string ip;
int port; string coordinator_ip; int coordinator_port;
  string server_ip;
  int server_port;
  thread update_thread;

    QuoServer(string ip, int server_port, string coordinator_ip, int coordinator_port);
    ~QuoServer();
    int join_server(IP ip, int port);
    server_list get_server_list();
    vector<pair<string, int>> serverList;

    void decode_articles(char *temp_articles);
    int queue_up(int art_id, string content);
    int WriteVote(ArticlePool art);
    int fetchWriteVote(int id, int mod_time);
    int ReadVote(ArticlePool articlePool);
    int fetchReadVote(int id, int mod_time);
  ArticlePool articlePool;
      thread insert_listen_thread;
  int insert_listen_fd;
  //bool isCoordinator(string server_ip);
    int synchronizer(ArticlePool art);
    map<int, Article*> art_tree;
    int self_version = 0;
    vector<pair<int,string>> ReadQuorumList;

    int udp_ask_vote(const char* ip, int port, const char* buf, const int buf_size);
    void udp_receive_vote(QuoServer *s,string r_ip, int port);
    int udp_send_vote(const char *ip, int port, const char *buf, const int buf_size);
    int udp_fwd_req(const char*, int, const char*, int, const char*, int port);
    static void listen_from(PeerClient *s, string remote_ip, int port);

private:
    //char ip[MAXIP];
    CLIENT *pclnt; //coordinator

    std::mutex subscriber_lock;
    std::mutex crit ;
    std::unordered_map<int, std::shared_ptr<std::mutex> > readlock;
    std::unordered_map<int, std::shared_ptr<std::mutex> > writelock;

  //  std::thread insert_listen_fd;
    std::vector<PeerClient *> subscribers;
    std::mutex update;
    std::vector<ArticlePool> updates;
    std::thread t_update;

bool isCoordinator(string ip);
    /* Create lock if it doesn't exist, then try to lock.
     * Returns status*/
    bool ReadLock(int id);
    bool WriteLock(int id);

    /* Return true if respective read/write lock is set */
    bool isReader(int id);
    bool isWriter(int id);

    /*Some infite loop for updates*/
    int Loop();

    int clearReadVote(int id);
    int getReadVote(int id);

};
