#include <iostream>
#include <map>
#include <vector>
using namespace std;

//struct address {
//    int ip;
//    int port;
//};

map<pair<int,int>, bool> serverIsCoordinator;
pair<int,int> coordinator;
class Server;

map<pair<int,int>, Server*> serverMap; 

class Server { 
    pair<int,int> addr;
    int data;
    int timeStamp;
public:
    //server pointer, whether is coordinator
    Server(int ip, int port) {
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

    void chooseNewCoordinator() {
        serverIsCoordinator.erase(serverIsCoordinator.find(coordinator));
        serverMap.erase(serverMap.find(coordinator));
        auto it = serverIsCoordinator.begin();
        it->second = true;
        coordinator = it->first;
    }

    void printServerList() {
        for (auto it = serverIsCoordinator.begin(); it != serverIsCoordinator.end(); it++) {
            cout << it->first.first << ", " << it->first.second << " " << (it->second ? "isCoordinator" : "is not") << endl;
        }
        cout << endl;
    }

    void printData() {
        cout << "Print data from server (" << addr.first << " "  << addr.second << ")" << endl;
        cout << data << endl;
        cout << endl;
    }

    void updateFromClient(int newData) {
        data = newData;
        if (coordinator == addr) { 
            notifyAllServer(newData);
        } else {
            updateCoordinatorData(newData);
        }
    }

    void updateCoordinatorData(int newData) {
        //To Do:
        //Find the coordinator based on coordinator ip and port
        Server * c = serverMap.find(coordinator)->second;
        c->updateFromClient(newData);
    }

    void notifyAllServer(int newData) {
        //To Do:
        //Find all server based on (ip, port) map
        for (auto it = serverMap.begin(); it != serverMap.end(); it++) { 
            if (it->second->addr != coordinator) {
                it->second->updateFromCoordinator(newData);
            }
        }
    }

    void updateFromCoordinator(int newData) {
        data = newData;
    }

};


int main() {
    cout << "Initialize coordinator" << endl;
    Server coordinator(1, 1);
    cout << "Print Server List from coordinator:" << endl;
    coordinator.printServerList();
    cout << "Initialize second server" << endl;
    Server secondServer(2, 2);
    Server thirdServer(3, 3);
    cout << "Print Server List from second server:" << endl;
    secondServer.printServerList();
    cout << "Print Server List from coordinator:" << endl;
    coordinator.printServerList();
    //cout << "Remove coordinator and choose new coordinator:" << endl;
    //secondServer.chooseNewCoordinator();
    coordinator.printServerList();
    cout << "Update server (3,3) with new data: 4" << endl;
    thirdServer.updateFromClient(4);
    thirdServer.printData();
    coordinator.printData();
    secondServer.printData();
    return 0;
}
