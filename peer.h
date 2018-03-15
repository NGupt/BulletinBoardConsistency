#pragma once

#include "communicate.h"
#include "article.h"
#include <set>
class PeerClient {
  string server_ip;
	int server_port;
	string coordinator_ip;
	int coordinator_port;
	ArticlePool articlePool;
	// int data;
	//int timeStamp;
	set<pair<string, int>> serverList;
public:
    CLIENT *pclnt;
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
    //simple join a server
    int joinServerSimple(string ip, int port);
    //join server and notify
    int joinServer(string ip, int port);
    bool isCoordinator();
    bool isCoordinator(string ip, int port);
    //PeerClient(string ip, int port);
    PeerClient(string ip, int server_port, string coordinator_ip, int coordinator_port);
    ~PeerClient();
};
