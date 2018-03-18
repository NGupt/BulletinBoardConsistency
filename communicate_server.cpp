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
#include <arpa/inet.h>
#include <netinet/in.h>
#include "peer.h"
#include <iostream>
#include "article.h"
#include <cstring>
#include <netdb.h>
#include <unistd.h>
using namespace std;
using std::to_string;

#ifndef SIG_PF
#define SIG_PF void(*)(int)
#endif

PeerClient * now;

bool PeerClient::isCoordinator(string ip) {
    return ip == coordinator_ip;
}

server_list PeerClient::buildServerList() {
    server_list res;
    res.server_list_val = new node[serverList.size()];
    res.server_list_len = serverList.size();
    int pos = 0;
    for (int i = 0; i < serverList.size(); i++) {
        node *p = res.server_list_val + pos;
        p->ip = new char[serverList[i].first.length() + 1];
        strcpy(p->ip, serverList[i].first.c_str());
        p->port = serverList[i].second;
        pos++;
    }
    return res;
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

//join a serve to coordinator
int PeerClient::joinServer(string ip, int port) {
    serverList.push_back(make_pair(ip, port));
    outputServerList(this);
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
    if (isCoordinator(o_ip)) {
        cout << "is coordinator" << endl;
    } else {
        cout << "is not coordinator, joining server to " << c_ip << endl;
    }
    join_server(o_ip, port);
    get_server_list();
    std::cout << ".....Completed Server creation.....\n";
}

PeerClient::~PeerClient() {
    if (pclnt){
    clnt_destroy(pclnt);
  }
}

int PeerClient::post(char * content) {
    int output;
    if(isCoordinator(server_ip)){
        std::string myString(content, strlen(content));
        //post to articlePool
        output = articlePool.post(myString);
        //send article to other servers;
    } else {
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
  if(isCoordinator(server_ip)){
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
  if(isCoordinator(server_ip)){
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
  if(isCoordinator(server_ip)){
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

//get the current articlePool
ArticlePoolStruct PeerClient::getLocalArticle() {
    return articlePool.getArticle();
}

//receive an article from another server
int PeerClient::receiveArticle(ArticlePoolStruct pool) {
    articlePool = ArticlePool(pool);
    return 1;
}

server_list PeerClient::get_server_list(){
  if(isCoordinator(server_ip)){
    outputServerList(this);
  } else {
    auto output = get_server_list_1(pclnt);
    if (output == (server_list *) NULL) {
      clnt_perror (pclnt, "call failed");
    }
    std::cout << "server_list is :" << endl;
    for (int i = 0; i < output->server_list_len; i++) {
        std::cout << (output->server_list_val + i)->ip << ":" << (output->server_list_val+i)->port << endl;
    }
  }
}

int PeerClient::join_server(IP ip, int port){
  if(isCoordinator(server_ip)){
    joinServer(ip, port);
  } else {
    auto output = join_server_1(ip, port, pclnt);
    if (output == (int *) NULL) {
      clnt_perror (pclnt, "call failed");
    } else {
      std::cout << "join server " << *output << std::endl;
    }
  }
  return 0;
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

server_list *
get_server_list_1_svc(struct svc_req *rqstp)
{
	static server_list  result = now->buildServerList();

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
