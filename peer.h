#pragma once

#include "communicate.h"
#include "article.h"
#include <set>
#include <thread>
#include "socket.h"
class PeerClient {

public:
    CLIENT *pclnt; //coordinator
    int sock;
    UdpSocket *sock_fd;
    char articles[MAXPOOLLENGTH];
    char servers[MAXSERVERS];
    string server_ip;
    int server_port;
    string coordinator_ip;
    int coordinator_port;
    ArticlePool articlePool;
    //UdpSocket sock_fd;//(const char *address, int &port);
    // int data;
    //int timeStamp;
    void decodeServerList(char *);
    char *encodeServerList();
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
    //simple join a server
    int joinServerSimple(string ip, int port);
    //join server and notify
    int joinServer(string ip, int port);
    bool isCoordinator();
    bool isCoordinator(string ip, int port);
    char* listen_for_articles(int port);
    char* listen_for_servers(int port);
    int send_articles(string ip, int port, const char *articles);
    int send_servers(string ip, int port, const char *servers);
    bool articleThread;
    bool serverListThread;
    std::thread c_article_thread; //thread for coordinator to send article pool
    std::thread c_servers_thread; //thread for coordinator to send server list
    //PeerClient(string ip, int port);
    PeerClient(string ip, int server_port, string coordinator_ip, int coordinator_port);
    ~PeerClient();
};

void ListenArticles(PeerClient *now);

void ListenServers(PeerClient *now);
