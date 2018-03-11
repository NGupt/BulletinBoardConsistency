#include <iostream>
#include <map>
#include <vector>
#include "peer_server.h"
using namespace std;


static map<pair<string,int>, bool> serverIsCoordinator;
static pair<string,int> coordinator;
static map<pair<string,int>, PeerServer*> serverMap; 
 
//server pointer, whether is coordinator
PeerServer::PeerServer(string ip, int port) {
    addr.first = ip;
    addr.second = port;
    if (serverIsCoordinator.empty()) {
        serverIsCoordinator[addr] = true;
        serverMap[addr] = this;
        coordinator = addr;
    } else {
        serverIsCoordinator[addr] = false;
        serverMap[addr] = this;
    }
}

void PeerServer::chooseNewCoordinator() {
    serverIsCoordinator.erase(serverIsCoordinator.find(coordinator));
    serverMap.erase(serverMap.find(coordinator));
    auto it = serverIsCoordinator.begin();
    it->second = true;
    coordinator = it->first;
}

void PeerServer::printPeerServerList() {
    for (auto it = serverIsCoordinator.begin(); it != serverIsCoordinator.end(); it++) {
        cout << it->first.first << ", " << it->first.second << " " << (it->second ? "isCoordinator" : "is not") << endl;
    }
    cout << endl;
}

void PeerServer::printData() {
    cout << "Print data from server (" << addr.first << " "  << addr.second << ")" << endl;
    cout << data << endl;
    cout << endl;
}

void PeerServer::updateFromClient(int newData) {
    data = newData;
    if (coordinator == addr) { 
        notifyAllPeerServer(newData);
    } else {
        updateCoordinatorData(newData);
    }
}

void PeerServer::updateCoordinatorData(int newData) {
    //To Do:
    //Find the coordinator based on coordinator ip and port
    PeerServer * c = serverMap.find(coordinator)->second;
    c->updateFromClient(newData);
}

void PeerServer::notifyAllPeerServer(int newData) {
    //To Do:
    //Find all server based on (ip, port) map
    for (auto it = serverMap.begin(); it != serverMap.end(); it++) { 
        if (it->second->addr != coordinator) {
            it->second->updateFromCoordinator(newData);
        }
    }
}

void PeerServer::updateFromCoordinator(int newData) {
    data = newData;
}


//int main() {
//    cout << "Initialize coordinator" << endl;
//    PeerServer coordinator(1, 1);
//    cout << "Print PeerServer List from coordinator:" << endl;
//    coordinator.printPeerServerList();
//    cout << "Initialize second server" << endl;
//    PeerServer secondPeerServer(2, 2);
//    PeerServer thirdPeerServer(3, 3);
//    cout << "Print PeerServer List from second server:" << endl;
//    secondPeerServer.printPeerServerList();
//    cout << "Print PeerServer List from coordinator:" << endl;
//    coordinator.printPeerServerList();
//    //cout << "Remove coordinator and choose new coordinator:" << endl;
//    //secondPeerServer.chooseNewCoordinator();
//    coordinator.printPeerServerList();
//    cout << "Update server (3,3) with new data: 4" << endl;
//    thirdPeerServer.updateFromClient(4);
//    thirdPeerServer.printData();
//    coordinator.printData();
//    secondPeerServer.printData();
//    return 0;
//}
