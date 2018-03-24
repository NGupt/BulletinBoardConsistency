#pragma once

#include "article.h"
#include "peer.h"
#include "quorum.h"
#include "communicate.h"
#include <vector>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <thread>

class QuoServer : public PeerClient {

public :
    QuoServer(string ip, int server_port, string coordinator_ip, int coordinator_port);
    ~QuoServer();

    int queue_up(int art_id, string content);
    int WriteVote(ArticlePool art);
    int fetchWriteVote(int id, int mod_time);
    int ReadVote(ArticlePool articlePool);
    int fetchReadVote(int id, int mod_time);

    int synchronizer(ArticlePool art);
    map<int, Article*> art_tree;
    int self_version = 0;
    void udp_receive_vote(QuoServer *s,string r_ip, int port);
    int udp_send_vote(const char *ip, int port, const char *buf, const int buf_size);
private:
    char ip[MAXIP];
    CLIENT *pclnt; //coordinator

    std::mutex subscriber_lock;
    std::mutex crit ;
    std::unordered_map<int, std::shared_ptr<std::mutex> > readlock;
    std::unordered_map<int, std::shared_ptr<std::mutex> > writelock;


    std::vector<PeerClient *> subscribers;
    std::mutex update;
    std::vector<ArticlePool> updates;
    std::thread t_update;


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
