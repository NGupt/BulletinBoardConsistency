#ifndef PEER_SERVER_H
#define PEER_SERVER_H
#pragma once
#include <iostream>
#include <map>
#include <vector>
using namespace std;


class PeerServer { 
    pair<string,int> addr;
    int data;
    int timeStamp;
public:
    //server pointer, whether is coordinator
    PeerServer(string ip, int port);

    void chooseNewCoordinator();

    void printPeerServerList();

    void printData();

    void updateFromClient(int newData);

    void updateCoordinatorData(int newData);

    void notifyAllPeerServer(int newData);

    void updateFromCoordinator(int newData);
};


#endif
