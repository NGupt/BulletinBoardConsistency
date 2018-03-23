#pragma once

#include "article.h"
#include <vector>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <thread>

#define MAX_STR = 16; //TODO- Check the main project folder to update to right value

class QuoSer : public PeerClient {

public :
    QuoSer();
    ~QuoSer();

    int queue_up(int art_id, string content);
    int WriteVote(ArticlePool art);
    int fetchWriteVote(int id, int mod_time);
    int ReadVote(int id, int mod_time);
    int fetchReadVote(int id, int mod_time);

private:
    CLIENT *pclnt;
    char ip[MAX_STR];

    std::mutex crit ;
    std::unordered_map<int, std::shared_ptr<std::mutex> > readlock;
    std::unordered_map<int, std::shared_ptr<std::mutex> > writelock;


    std::vector<CLIENT *> sub;
    std::mutex update;
    std::vector<ArticlePool> updates;
    std::thread t_update;

    bool isCoordinator() const;

    int synchronizer(ArticlePool id);

    /* Create lock if it doesn't exist, then try to lock.
     * Returns status*/
    bool ReadLock(int id);
    bool WriteLock(int id);

    /* Return true if respective read/write lock is set */
    bool isReader(int id);
    bool isWriter(int id);

    /*Some infite loop for updates*/
    int Loop();

};
