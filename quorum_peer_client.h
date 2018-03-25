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
    string coordinator_ip;
    int coordinator_port;
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
   // map<int, Article*> art_tree;
    vector<pair<int,string>> ReadQuorumList;
    vector<pair<int,string>> WriteQuorumList;

    int udp_ask_vote(const char* ip, int port, const char* buf, int buf_size);
    static void udp_recv_vote_req(QuoServer *s,string r_ip, int port);
    int udp_fwd_req(const char* target_serv_ip, int serv_port, const char* client_ip, int client_port, const char* buf, int buf_size);

private:
    CLIENT *pclnt; //coordinator

    std::mutex subscriber_lock;
    std::mutex crit ;
//    std::unordered_map<int, std::shared_ptr<std::mutex> > readlock;
    std::shared_ptr<std::mutex> readlock;
    std::shared_ptr<std::mutex> writelock;

    //std::unordered_map<int, std::shared_ptr<std::mutex> > writelock;

  //  std::thread insert_listen_fd;
    std::vector<QuoServer *> subscribers;
    std::mutex update;
    std::vector<ArticlePool> updates;
    std::thread t_update;

    bool isCoordinator(string ip);
    /* Create lock if it doesn't exist, then try to lock.
     * Returns status*/
    bool ReadLock(int id);
    bool WriteLock(int id);

    /* Return true if respective read/write lock is set */
    bool isReader(QuoServer *q);
    bool isWriter(QuoServer *q);

    /*Some infite loop for updates*/
    int Loop();

    int clearReadVote(QuoServer *q);
    int getReadVote(QuoServer *q);

};
