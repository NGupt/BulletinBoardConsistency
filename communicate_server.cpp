/*
 * This is sample code generated by rpcgen.
 * These are only templates and you can use them
 * as a guideline for developing your own functions.
 */

#include "communicate.h"
#include <stdio.h>
#include <stdlib.h>
#include <rpc/pmap_clnt.h>
#include <string.h>
#include <memory.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "peer.h"
#include <iostream>
#include "article.h"
using namespace std;

#ifndef SIG_PF
#define SIG_PF void(*)(int)
#endif


PeerClient * now;

// extern map<pair<string, int>, PeerServer*> simulateUDP;

//////////////////////////////////// peer client ////////////////////
//class PeerClient;

//
// PeerClient::PeerClient(string ip, int port){
//   this->server_ip = ip;
//   this->server_port = port;
//   this->coordinator_ip = ip;
//   this->coordinator_port = port;
//   now = this;
//   std::cout << ".....Completed peer coordinator creation.....\n";
// }
// 
int PeerClient::joinServerSimple(string ip, int port) {
    if (serverList.find(make_pair(ip, port)) == serverList.end()) {
        serverList.insert(make_pair(ip, port));
    }
    return 1;
}

bool PeerClient::isCoordinator() {
  return isCoordinator(server_ip, server_port);
}
bool PeerClient::isCoordinator(string ip, int port) {
    return ip == coordinator_ip && port == coordinator_port;
}

server_list PeerClient::buildServerList() {
    server_list res;
    res.server_list_val = new node[serverList.size()];
    res.server_list_len = serverList.size();
    int pos = 0;
    for (set<pair<string, int>>::iterator it = serverList.begin(); it != serverList.end(); it++) {
        node *p = res.server_list_val + pos;
        p->ip = new char[it->first.length() + 1];
        strcpy(p->ip, it->first.c_str());
        p->port = it->second;
        pos++;
    }
    return res;
}

int PeerClient::receiveServerList(server_list servers) {
  serverList.clear();
  for (int i = 0; i < servers.server_list_len; i++) {
      IP ip = (servers.server_list_val + i)->ip;
      string ips(ip, strlen(ip));
      serverList.insert(make_pair(ips, (servers.server_list_val+i)->port));
  }
  return 1;
}

void outputServerList(PeerClient *p) {
    //output server list;
    cout << "outputing server list:" << endl;
    server_list servers = p->buildServerList();
    for (int i = 0; i < servers.server_list_len; i++) {
        cout << (servers.server_list_val+i)->ip << " " << (servers.server_list_val+i)->port << endl;
    }
    cout << endl;
}

CLIENT * generateClient(char * currentIp) {
  CLIENT *pclnt = clnt_create(currentIp, COMMUNICATE_PROG, COMMUNICATE_VERSION, "udp");
  if (pclnt == NULL) {
    clnt_pcreateerror (currentIp);
    exit (1);
  }
  return pclnt;
}



void PeerClient::sendServerListToAll() {
    server_list servers = buildServerList();
    for (auto it = serverList.begin(); it != serverList.end(); it++) {
        if (isCoordinator(it->first, it->second)) continue;
        char *currentIp = new char[it->first.length() + 1];
        strcpy(currentIp, it->first.c_str());
        CLIENT * c = generateClient(currentIp);
        send_server_list_1(servers, c);
        if (c) clnt_destroy(c);
    }
}


//join a serve to coordinator
int PeerClient::joinServer(string ip, int port) {
    joinServerSimple(ip, port);
    outputServerList(this);
    sendServerListToAll();
}

PeerClient::PeerClient(string ip, int port, string coordinator_ip, int coordinator_port){
    this->server_ip = ip;
    this->server_port = port;
    this->coordinator_ip = coordinator_ip;
    this->coordinator_port = coordinator_port;
    now = this;
    char * c_ip = new char [coordinator_ip.length()+1];
    strcpy (c_ip, coordinator_ip.c_str());
    char * o_ip = new char [ip.length()+1];
    strcpy (o_ip, ip.c_str());
    pclnt = clnt_create (c_ip, COMMUNICATE_PROG, COMMUNICATE_VERSION, "udp");
    if (pclnt == NULL) {
      clnt_pcreateerror (c_ip);
      exit (1);
    }
    if (isCoordinator()) {
        cout << "is coordinator" << endl;
        joinServerSimple(o_ip, port);
    } else {
        cout << "is not coordinator, call join_server_1 at " << c_ip << endl;
        join_server_1(o_ip, port, pclnt);
        cout << "after join server" << endl;
        server_list servers = buildServerList();
        auto output = send_server_list_1(servers, pclnt);
        outputServerList(this);
    }
    std::cout << ".....Completed Server creation.....\n";
}

PeerClient::~PeerClient() {
    if (pclnt){
    clnt_destroy(pclnt);
  }
}

int PeerClient::post(char * content) {
    int output;
    cout << 1 << endl;
    return 1;
    if(server_ip == coordinator_ip && server_port == coordinator_port){
        cout << 2 << endl;
        std::string myString(content, strlen(content));
        //post to articlePool
        output = articlePool.post(myString);
        cout << 3 << endl;

        //send article to other servers;
    } else {
        cout << 4 << endl;
        output = *post_1(content, pclnt);
    }
    if (output == 0) {
      std::cout << "Post the article " << content << " fails" << std::endl;
      //clnt_perror(clnt, "Cannot post");
    } else {
      std::cout << "Post the article " << output << " " << content << std::endl;
    }
    return output;
}

string PeerClient::read() {
  if(server_ip == coordinator_ip && server_port == coordinator_port){
    return articlePool.read();
  } else {
    auto output = read_1(pclnt);
    if (output == NULL) {
        clnt_perror(pclnt, "Cannot read");
    } else {
        std::cout << "Read from server\n" << *output << std::endl;
    }
    return *output;
  }
}

ArticleContent PeerClient::choose(int index) {
  if(server_ip == coordinator_ip && server_port == coordinator_port){
    static ArticleContent  result;
    result.content = new char[MAXSTRING];
    Article * resultArticle = articlePool.choose(index);
    if (resultArticle == NULL) {
      strcpy(result.content, "");
      result.index = 0;
      cout << "The article with id " << index << " doesn't exist in the server." << endl;
    } else {
      strcpy(result.content, resultArticle->content.c_str());
      result.index = resultArticle->index;
      cout << "The client choose the article: " << endl;
      cout << result.index << " " << result.content << endl;
    }
    return result;
  } else {
    auto output = choose_1(index, pclnt);
    if (output->index == 0) {
        std::cout << "Cannot choose article with id " << index << std::endl;
    } else {
        std::cout << "Choose the article:\n" << output->index << " " << output->content << std::endl;
    }
    return *output;
  }
}

int PeerClient::reply(char *content, int index) {
  if(server_ip == coordinator_ip && server_port == coordinator_port){
    std::string myString(content, strlen(content));
    return articlePool.reply(myString, index);
  } else {
    auto output = reply_1(content, index, pclnt);
    if (*output == 0) {
        std::cout << "Can't reply to article " << index << " with " << content << std::endl;
        //clnt_perror(clnt, "Cannot reply to ");
    } else {
        std::cout << "Reply the article " << index << " with the new article " << *output << " " << content << std::endl;
    }
    return *output;
  }
}

int PeerClient::send_flag(int flag){
  auto output = send_flag_1(flag, pclnt);
  if (output == (int *) NULL) {
    clnt_perror(pclnt, "call failed");
  } else {
    std::cout << "Sent flag " << *output << std::endl;
  }
}

//get the current articlePool
ArticlePoolStruct PeerClient::getLocalArticle() {
    return articlePool.getArticle();
}

//receive an article from another server
int PeerClient::receiveArticle(ArticlePoolStruct pool) {
    articlePool = ArticlePool(pool);
    return 1;
}

ArticlePoolStruct PeerClient::get_article(){
    //remote call
  auto output = get_article_1(pclnt);
  if (output == (ArticlePoolStruct *) NULL) {
    clnt_perror (pclnt, "call failed");
  } else {
    std::cout << "Get article " << output << std::endl;
  }
}


int PeerClient::send_article(ArticlePoolStruct pool){
    //remote call
  auto output = send_article_1(pool, pclnt);
  if (output == (int *) NULL) {
    clnt_perror (pclnt, "call failed");
  } else {
    std::cout << "send article " << *output << std::endl;
  }
}

int PeerClient::send_server_list(server_list servers){
  auto output = send_server_list_1(servers, pclnt);
  if (output == (int *) NULL) {
    clnt_perror (pclnt, "call failed");
  } else {
    std::cout << "send article " << *output << std::endl;
  }
}

server_list PeerClient::get_server_list(){
  auto output = get_server_list_1(pclnt);
  if (output == (server_list *) NULL) {
    clnt_perror (pclnt, "call failed");
  } else {
  //  std::cout << "get server lsit " << *output << std::endl;
  }
}

int PeerClient::join_server(IP ip, int port){
  //std::string myString(content, strlen(ip));
  auto output = join_server_1(ip, port, pclnt);
  if (output == (int *) NULL) {
    clnt_perror (pclnt, "call failed");
  } else {
    std::cout << "join server " << *output << std::endl;
  }
}
////////////////////////////peer client////////////////////////////////////////





////////////////////////////peer server/////////////////////////////////////
int *
post_1_svc(char *content,  struct svc_req *rqstp)
{
    cout << "in post_1_svc " << endl;
    //std::string myString(content, strlen(content));
    static int result = 0;
    //auto now = simulateUDP.find(make_pair("127.0.0.1", 1234))->second;
    auto res = now->post(content);
    result = res;
    if (result == 0) {
      cout << "Post fails. " << endl;
    } else {
       cout << "Post an article:" << endl;
       cout << result << " " << content << endl;
    }
  return &result;
}

char **
read_1_svc(struct svc_req *rqstp)
{
  static char * result = new char[MAXSTRING];
    //auto now = simulateUDP.find(make_pair("127.0.0.1", 1234))->second;
    string resultStr = now->read();
    strcpy(result, resultStr.c_str());
    cout << "Read from server:" << endl;
    cout << result << endl;
  return &result;
}

ArticleContent *
choose_1_svc(int index,  struct svc_req *rqstp)
{
  static ArticleContent  result;
    result = now->choose(index);
    // if (result == NULL) {
    //     strcpy(result.content, "");
    //     result.index = 0;
    //     cout << "The article with id " << index << " doesn't exist in the server." << endl;
    // } else {
    //     strcpy(result.content, resultArticle->content.c_str());
    //     result.index = resultArticle->index;
    //     cout << "The client choose the article: " << endl;
    //     cout << result.index << " " << result.content << endl;
    // }
  return &result;
}

int *
reply_1_svc(char *content, int index,  struct svc_req *rqstp)
{
  //string resultStr(content, strlen(content));
  static int result = 0;
  //auto now = simulateUDP.find(make_pair("127.0.0.1", 1234))->second;
  int res = now->reply(content, index);
  result = res;
  if (result == 0) {
      cout << "Can't reply to article with id " << index << "." << endl;
  } else {
      cout << "Reply article " << index << " with:";
      cout << result << " " << content << endl;
  }
  return &result;
}

int *
send_flag_1_svc(int flag,  struct svc_req *rqstp)
{
	static int  result = 1;

	return &result;
}

ArticlePoolStruct *
get_article_1_svc(struct svc_req *rqstp)
{
	static ArticlePoolStruct  result = now->getLocalArticle();
	return &result;
}

int *
send_article_1_svc(ArticlePoolStruct pool,  struct svc_req *rqstp)
{
    //updates' articlePool
    now->receiveArticle(pool);
	static int  result = 1;
	return &result;
}

int *
send_server_list_1_svc(server_list servers,  struct svc_req *rqstp)
{
	static int  result = now->receiveServerList(servers);
	return &result;
}


server_list *
get_server_list_1_svc(struct svc_req *rqstp)
{
	static server_list  result;

	/*
	 * insert server code here
	 */

	return &result;
}

int *
join_server_1_svc(IP arg1, int arg2,  struct svc_req *rqstp)
{
  cout << "join a server " << arg1 << " " << arg2 <<" into coordinator" << endl;
  string ips(arg1, strlen(arg1));
	static int  result = now->joinServer(ips, arg2);
	/*
	 * insert server code here
	 */

	return &result;
}
////////////////////////////peer server//////////////////////////////////
