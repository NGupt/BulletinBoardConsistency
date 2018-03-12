#ifndef PEER_SERVER_H
#define PEER_SERVER_H
#pragma once
#include <iostream>
#include <map>
#include <vector>
#include <set>
#include "article.h"
using namespace std;


class PeerServer { 
    string ip;
    int port;
    string coordinator_ip;
    int coordinator_port;
    ArticlePool articlePool;
    int data;
    int timeStamp;
    set<pair<string, int>> serverList;
public:
    //server pointer, whether is coordinator
    PeerServer(string ip, int port);
    PeerServer(string coordinator_ip, int coordinator_port, string ip, int port);

    pair<string,int> chooseNewCoordinator();
    pair<string,int> findCoordinator();
    void printPeerServerList();
    void printData();


    set<pair<string,int>> getServerList();
    bool isCoordinator();
    bool isCoordinator(const pair<string,int> &ipAndPort);
    void changeCoordinator(string coordinator_ip, int coordinator_port);

    //------------------data replicas related function--------------//
    void updateCoordinatorData(string origin_ip, int origin_port);
    void updateFromServer(ArticlePool newData, string origin_ip, int origin_port);
    void notifyDataChange(string origin_ip, int origin_port);
    void notifyAllPeerServer(string origin_ip, int origin_port);
    void updateSimple(ArticlePool newData);


    //-------------------server list related functions-------------//
    //store the peerServer into a simulated map, TODO 
    void joinNewServer(string server_ip, int server_port, PeerServer *p);
    //join a server to server list based on whether this server is coordinator
    void joinServer(string server_ip, int server_port);
    //coordinator: add a new server to serverlist for all server
    void joinServerToAll(string server_ip, int server_port);
    //simply add a new server to serverlist
    void joinServerSimple(string server_ip, int server_port);
    //normal server: notify the coordinator that there's a new server 
    void notifyCoordinatorJoinServer(string server_ip, int server_port);

    //delete a new server to serverlist for all server
    void deleteServer(string server_ip, int server_port);
    void deleteServerToAll(string server_ip, int server_port);
    void deleteServerSimple(string server_ip, int server_port);
    void notifyCoordinatorDeleteServer(string server_ip, int server_port);


    //--------------------article related functions--------//
	Article* choose(int index);
	int reply(string article, int index);
	int post(string article);
	string read();
    ArticlePool deepCopy();

    //------------others----------------------//
    int getTimeStamp();
    void close();


};

//extern map<pair<string, int>, PeerServer*> simulateUDP;


#endif
