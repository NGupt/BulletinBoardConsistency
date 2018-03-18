#pragma once

#include "communicate.h"
#include "article.h"
#include <set>
#include <thread>

class PeerClient {

public:
    CLIENT *pclnt; //coordinator
    string server_ip;
    int server_port;
    string coordinator_ip;
    int coordinator_port;
    ArticlePool articlePool;
    vector<pair<string, int>> serverList;
    vector<CLIENT *> pclnts; //serverlists
    int send_flag(int flag);
    ArticlePoolStruct get_article();
    ArticlePoolStruct getLocalArticle();
    server_list buildServerList();
    void sendServerListToAll();
    int receiveServerList(server_list servers);
    int receiveArticle(ArticlePoolStruct pool);
    int send_article(ArticlePoolStruct);
    int send_server_list(server_list servers);
    int post(char * content);
    string read();
    ArticleContent choose(int index);
    int reply(char * content, int index);
    server_list get_server_list();
    int join_server(IP ip, int port);
    int joinServer(string ip, int port);
    bool isCoordinator(string ip);
    PeerClient(string ip, int server_port, string coordinator_ip, int coordinator_port);
    ~PeerClient();
};
