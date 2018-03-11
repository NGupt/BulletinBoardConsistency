#pragma once
#include <iostream>
#include <map>
#include <vector>
using namespace std;


map<pair<int,int>, bool> serverIsCoordinator;
pair<int,int> coordinator;
class PeerServer;

map<pair<int,int>, PeerServer*> serverMap; 

class PeerServer { 
    pair<int,int> addr;
    int data;
    int timeStamp;
public:
    //server pointer, whether is coordinator
    PeerServer(int ip, int port);

    void chooseNewCoordinator();

    void printPeerServerList();

    void printData();

    void updateFromClient(int newData);

    void updateCoordinatorData(int newData);

    void notifyAllPeerServer(int newData);

    void updateFromCoordinator(int newData);
};


