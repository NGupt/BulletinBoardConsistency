#include <iostream>
#include <map>
#include <vector>
#include "article.h"
#include <set>
#include "peer_server.h"
using namespace std;


map<pair<string, int>, PeerServer*> simulateUDP;
 
PeerServer::PeerServer(string ip, int port) {
    this->ip = ip;
    this->port = port;
    serverList.insert(make_pair(ip, port));
    coordinator_ip = ip;
    coordinator_port = port;
    //for simulating rpc
    simulateUDP[make_pair(ip, port)] = this;
}

PeerServer::PeerServer(string coordinator_ip, int coordinator_port, string ip, int port) {
    this->ip = ip;
    this->port = port;
    this->coordinator_ip = coordinator_ip;
    this->coordinator_port = coordinator_port;
    PeerServer *coordinator = simulateUDP.find(findCoordinator())->second;
    coordinator->joinNewServer(ip, port, this);
    serverList = coordinator->getServerList(); 
}



void PeerServer::printPeerServerList() {
    for (auto it = serverList.begin(); it != serverList.end(); it++) {
        cout << "(" << it->first << "," << it->second << ")";
    }
    cout << endl << endl;
}

void PeerServer::printData() {
    cout << "Print data from server (" << ip << ", "  << port << ")" << endl;
    cout << articlePool.read();
}

//notify all servers if it's coordinator
//if it's a normal server: notify coordinator
void PeerServer::notifyDataChange(string origin_ip, int origin_port) {
    if (coordinator_ip == ip && coordinator_port == port) { 
        notifyAllPeerServer(origin_ip, origin_port);
    } else {
        updateCoordinatorData(origin_ip, origin_port);
    }
}

//coordinator: get data from a normal server
//notify the data change
void PeerServer::updateFromServer(ArticlePool newData, string origin_ip, int origin_port) {
    articlePool = newData;
    notifyDataChange(origin_ip, origin_port);
}


//notify coordinator to change data
void PeerServer::updateCoordinatorData(string origin_ip, int origin_port) {
    //To Do:
    //Find the coordinator based on coordinator ip and port
    PeerServer * c = simulateUDP.find(findCoordinator())->second;
    c->updateFromServer(articlePool, origin_ip, origin_port);
}


//coorindator: notify all peer servers in the serverList to change data to current coordinator's data
//skips the server that is origin
void PeerServer::notifyAllPeerServer(string origin_ip, int origin_port) {
    //ToDo:
    //Find all server based on (ip, port) map
    vector<pair<string,int>> goesDown;
    for (auto it = serverList.begin(); it != serverList.end(); it++) { 
        if (isCoordinator(*it)) continue;
        if (it->first == origin_ip && it->second == origin_port) continue;
        auto now = simulateUDP.find(*it);
        //there's a server goes down
        if (now == simulateUDP.end()) {
            goesDown.push_back(*it);
        //the server doesn't down
        } else {
            auto p = now->second;
            p->updateSimple(articlePool);
        }
    }
    for (int i = 0; i < goesDown.size(); i++) {
        deleteServer(goesDown[i].first, goesDown[i].second);
    }
}

set<pair<string,int>> PeerServer::getServerList() {
    return serverList;
}

void PeerServer::updateSimple(ArticlePool newData) {
    articlePool = newData;
}


bool PeerServer::isCoordinator() {
    pair<string, int> c = findCoordinator();
    return ip == c.first && port == c.second;
}
bool PeerServer::isCoordinator(const pair<string,int> &now) {
    pair<string, int> c = findCoordinator();
    return now.first == c.first && now.second == c.second;
}

//--------------join a server from server list-----------------//
//register the server to simulateUDP
void PeerServer::joinNewServer(string server_ip, int server_port, PeerServer *p) {
    //TODO
    simulateUDP[make_pair(server_ip, server_port)] = p;
    joinServer(server_ip, server_port);
}

//join a server based on whether it's a coordinator
void PeerServer::joinServer(string server_ip, int server_port) {
    joinServerSimple(server_ip, server_port);
    if (isCoordinator()) {
        joinServerToAll(server_ip, server_port);
    } else {
        notifyCoordinatorJoinServer(server_ip, server_port);
    }
}

//notify the coordinator to join a new server
void PeerServer::notifyCoordinatorJoinServer(string server_ip, int server_port) {
    auto now = simulateUDP.find(findCoordinator()); 
    now->second->joinServerToAll(server_ip, server_port);
}


//coordinator: join this server to all server's serverlist
void PeerServer::joinServerToAll(string server_ip, int server_port) {
    vector<pair<string, int>> goesDown;
    joinServerSimple(server_ip, server_port);
    for (auto it = serverList.begin(); it != serverList.end(); it++) {
        //skip coordinator
        if (isCoordinator(*it)) continue;
        //find avaliable server
        auto now = simulateUDP.find(*it);
        if (now != simulateUDP.end()) {
            //join this server from server's server list
            now->second->joinServerSimple(server_ip, server_port);
        } else {
            goesDown.push_back(make_pair(server_ip, server_port));
        }
        
    }
    for (int i = 0; i < goesDown.size(); i++) {
        deleteServer(goesDown[i].first, goesDown[i].second);
    }
}

//simply join a server to this server's server list
void PeerServer::joinServerSimple(string server_ip, int server_port) {
    if (serverList.find(make_pair(server_ip, server_port)) == serverList.end()) {
        serverList.insert(make_pair(server_ip, server_port)); 
    }
}


//---------------delete a server from server list-------------//
void PeerServer::deleteServer(string server_ip, int server_port) {
    if (isCoordinator()) {
        deleteServerToAll(server_ip, server_port);
    } else {
        deleteServerSimple(server_ip, server_port);
        notifyCoordinatorDeleteServer(server_ip, server_port);
    }
}

void PeerServer::notifyCoordinatorDeleteServer(string server_ip, int server_port) {
    auto now = simulateUDP.find(make_pair(coordinator_ip, coordinator_port)); 
    now->second->deleteServerToAll(server_ip, server_port);
}

//delete (server_ip, server_port) in the serverlist for all avaliable servers
void PeerServer::deleteServerToAll(string server_ip, int server_port) {
    deleteServerSimple(server_ip, server_port); 
    vector<pair<string, int>> goesDown;
    //go through serverlist
    for (auto it = serverList.begin(); it != serverList.end(); it++) {
        //skip coordinator
        if (it->first != coordinator_ip || it->second != coordinator_port) {
            //find avaliable server
            auto now = simulateUDP.find(*it);
            if (now != simulateUDP.end()) {
                //delete this server from server's server list
                now->second->deleteServerSimple(server_ip, server_port);
            } else {
                goesDown.push_back(make_pair(server_ip, server_port));
            }
        }
    }
    for (int i = 0; i < goesDown.size(); i++) {
        if (goesDown[i].first == server_ip && goesDown[i].second == server_port) continue;
        deleteServer(goesDown[i].first, goesDown[i].second);
    }
}

void PeerServer::deleteServerSimple(string server_ip, int server_port) {
    auto it = serverList.find(make_pair(server_ip, server_port));
    if (it != serverList.end()) {
        serverList.erase(it);
    } else {
    }
    //delete this server in simulateUDP too
    //TODO
    auto now = simulateUDP.find(make_pair(server_ip, server_port));
    if (now != simulateUDP.end()) {
        simulateUDP.erase(now);
    }
    cout << endl;

}

//find the coordinator
//if the coordinator goes down -> run election
//return the new ip and port of coordinator 
pair<string, int> PeerServer::findCoordinator() {
    //try find coordinator
    auto c = simulateUDP.find(make_pair(coordinator_ip, coordinator_port));
    //coordinator goes down
    if (c == simulateUDP.end()) {
        return chooseNewCoordinator();
    }
    return make_pair(coordinator_ip, coordinator_port);
}

//choose a new coordinator when the old one goes down
pair<string,int> PeerServer::chooseNewCoordinator() {
    //TODO


    //mark servers that goes down
    vector<pair<string, int>> goesDown;
    string old_ip = coordinator_ip;
    int old_port = coordinator_port;
    changeCoordinator(ip, port);
    //delete coordinator_ip from every existing servers
    for (auto it = serverList.begin(); it != serverList.end(); it++) {
        auto now = simulateUDP.find(*it);
        if (now != simulateUDP.end()) {
            now->second->changeCoordinator(ip, port);
            now->second->deleteServerSimple(old_ip, old_port); 
        } else {
            goesDown.push_back(*it);
        }
    }
    //make this server the coordinator

    //erase all servers that goes down
    for (int i = 0; i < goesDown.size(); i++) {
        //cout << "deleting : " << goesDown[i].first << " " << goesDown[i].second << endl;
        if (goesDown[i].first == old_ip && goesDown[i].second == old_port) continue;
        deleteServer(goesDown[i].first, goesDown[i].second);
    }
    return make_pair(coordinator_ip, coordinator_port);
}

//---------------coordinator changes--------------//
//a normal server, changes its coordinator direcly
void PeerServer::changeCoordinator(string coordinator_ip, int coordinator_port) {
    this->coordinator_ip = coordinator_ip;
    this->coordinator_port = coordinator_port;
}


//---------------article related functions--------//
int PeerServer::reply(string article, int index) {
    if (!isCoordinator()) { 
        PeerServer *coordinator = simulateUDP.find(findCoordinator())->second;
        articlePool = coordinator->deepCopy();
    }
    int res;
    if (res = articlePool.reply(article, index)) {
        notifyDataChange(ip, port);
    }
    return res;
}


int PeerServer::post(string article) {
    if (!isCoordinator()) {
        PeerServer *coordinator = simulateUDP.find(findCoordinator())->second;
        articlePool = coordinator->deepCopy();
    }
    int res;
    if (res = articlePool.post(article)) {
        notifyDataChange(ip, port);
    }
    return res;
}

ArticlePool PeerServer::deepCopy() {
    return articlePool.deepCopy();

}

Article* PeerServer::choose(int index) {
    if (!isCoordinator()) {
        PeerServer *coordinator = simulateUDP.find(findCoordinator())->second;
        articlePool = coordinator->deepCopy();
    }
    return articlePool.choose(index);
}

string PeerServer::read() {
    if (!isCoordinator()) {
        PeerServer *coordinator = simulateUDP.find(findCoordinator())->second;
        articlePool = coordinator->deepCopy();
    }
    return articlePool.read();
}

int PeerServer::getTimeStamp() {
    return timeStamp;
}

//test goes down
void PeerServer::close() {
    simulateUDP.erase(make_pair(ip, port));
}





//int main() {
//    //ArticlePool articlePool;
//    ////------------test article pool-----/
//    cout << "Initialize coordinator" << endl;
//    PeerServer coordinator("183.0.0.1", 1);
//    cout << "Print PeerServer List from (183.0.0.1, 1)" << endl;
//    coordinator.printPeerServerList();
//    coordinator.post("article 1");
//    cout << "Read from (183.0.0.1, 1)" << endl;
//    coordinator.printData();
//
//
//    cout << "Initialize second server" << endl;
//    PeerServer secondPeerServer("183.0.0.1", 1, "183.0.0.2", 2);
//    cout << "Initialize third server" << endl;
//    PeerServer thirdPeerServer("183.0.0.1", 1, "183.0.0.3", 3);
//    cout << "Print PeerServer List from (183.0.0.2, 2):" << endl;
//    secondPeerServer.printPeerServerList();
//    cout << "Print PeerServer List from (183.0.0.3, 3):" << endl;
//    coordinator.printPeerServerList();
//    cout << "Post \"article 2\" at (183.0.0.3, 3):" << endl;
//    thirdPeerServer.post("article 2");
//    thirdPeerServer.printData();
//    cout << "Reply \"article 3\" for \"article 2\" at (183.0.0.3, 3):" << endl;
//    secondPeerServer.reply("article 3", 2);
//    thirdPeerServer.printData();
//    PeerServer fourthPeerServer("183.0.0.1", 1, "183.0.0.4", 4);
//    cout << "Print PeerServer List from (183.0.0.2, 2):" << endl;
//    secondPeerServer.printPeerServerList();
//
//    //---test when the coordinator goes down----//
//    cout << "Coordinator goes down" << endl;
//    coordinator.close();
//    cout << "post article at second server" << endl;
//    secondPeerServer.post("article 4");
//    secondPeerServer.printData();
//    thirdPeerServer.printData();
//    fourthPeerServer.printData();
//
//    //---test when a normal goes down----//
//    cout << "A normal server goes down" << endl;
//    cout << "(183.0.0.4, 4) goes down" << endl;
//    fourthPeerServer.close();
//    cout << "post article at second server" << endl;
//    secondPeerServer.post("article 5");
//    secondPeerServer.printData();
//    cout << "Peer Server list at (183.0.0.2, 2)" << endl;
//    secondPeerServer.printPeerServerList();
//    thirdPeerServer.printData();
//    cout << "Peer Server list at (183.0.0.3, 3)" << endl;
//    thirdPeerServer.printPeerServerList();
//    return 0;
//}
