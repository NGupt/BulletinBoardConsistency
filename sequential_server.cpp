#include "sequential_server.h"

using namespace std;
using std::to_string;
using std::thread;

int sock;

thread insert_listen_thread;
int insert_listen_fd;


std::vector <std::string> peerList_IP;
std::vector <int> peerList_port;

char *c_ip;
char *o_ip;

bool SequentialServer::isCoordinator(string server_ip) {
    return server_ip == coordinator_ip ;
}

int SequentialServer::insert(PeerClient *p, int art_id, string content)
{
    int result;
    //SequentialServer *s = new SequentialServer(p->server_ip, p->server_port, p->coordinator_ip, p->coordinator_port);
    // if this server is the coordinator
    //if (s->isCoordinator(o_ip)) {
        for (int i=0; i<(p->serverList.size()); i++) {
            printf("Updating %s:%d\n",p->serverList[i].first.c_str(), p->serverList[i].second);
            result = p->updateServer(art_id, content, (char*)(p->serverList[i].first.c_str()), p->serverList[i].second);
            if (result == -1) {
                printf("Update to backup failed");
                return -1;
            }
//            else {
//                // connect to coordinator
//                // send to coordinator
//                // wait for confirmation that update has completed
//
//                /* Send register msg */
//                printf("%s:%d is updating servers\n",c_ip, s->coordinator_port);
//                result = s->updateServer(art_id, s->articleMap[art_id]->content, (char*)c_ip, s->coordinator_port);
//                if (result == -1) {
//                    printf("Update to backup failed. %s:%d\n", c_ip, s->coordinator_port);
//                    return -1;
//                }
//            }
            printf("Insert Complete.\n");
            // send confirmation to client
            return result;
        }
    //}
}

void SequentialServer::listen_from(SequentialServer *s,string r_ip, int port){
    struct sockaddr_in remote_addr, self_addr;
    const char *remote_ip = r_ip.c_str();
    int bytes = 0;
    socklen_t slen = sizeof(remote_addr);

    memset((char *) &remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(port);
    if (inet_aton(remote_ip, &remote_addr.sin_addr) == 0) {
        fprintf(stderr, "inet_aton failed\n");
    }

    bzero((char *) &self_addr, sizeof(self_addr));
    self_addr.sin_family = AF_INET;
    self_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    self_addr.sin_port = htons((unsigned short) port);

    char article_update[MAXSTRING];
    // Clear the buffer by filling null, it might have previously received data
    memset(article_update, '\0', MAXSTRING);

    // Try to receive some data; This is a blocking call
    while (1) {
        if (recvfrom(s->insert_listen_fd, article_update, MAXSTRING, 0, (struct sockaddr *) &remote_addr, &slen) < 0) {
            perror("recvfrom()");
        }
        cout << "listened "  << " " << article_update <<endl;

//
//        if ((sendto(s->insert_listen_fd, article_update, MAXSTRING, 0, (struct sockaddr *) &remote_addr, slen)) == -1)
//        {
//            close(s->insert_listen_fd);
//            s->insert_listen_fd = -1;
//            cout << "Error: acknowledging the received string" << endl;
//            throw;
//        }
    }
}

SequentialServer::SequentialServer(string server_ip, int server_port, string coordinator_ip, int coordinator_port){
    this->server_ip = server_ip;
    this->server_port = server_port;
    this->coordinator_ip = coordinator_ip;
    this->coordinator_port = coordinator_port;
    c_ip = new char [coordinator_ip.length()+1];
    strcpy (c_ip, coordinator_ip.c_str());
    o_ip = new char [server_ip.length()+1];
    strcpy (o_ip, server_ip.c_str());

    sockaddr_in si_me;
    printf("Begin listening to requests from the other servers...\n");
    insert_listen_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (insert_listen_fd == -1) {
        printf("Error: creating socket\n");
        throw;
    }

    memset(&si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_addr.s_addr= htonl(INADDR_ANY);

    if (isCoordinator(o_ip)) {
        // This is the Coordinator
        si_me.sin_port = htons(coordinator_port); // listens to peer_servers
        cout << "Coordinator started" << endl;
    } else {
        // This is not the Coordinator
        si_me.sin_port = htons(server_port); // listens to the coordinator
        cout << "Backup peer server started" << endl;
    }

    if (bind(insert_listen_fd, (struct sockaddr*)&si_me, sizeof(si_me)) == -1)     {
        close(insert_listen_fd);
        insert_listen_fd = -1;
        cout << "Error: binding socket" << endl;
        throw;
    }
    insert_listen_thread = thread(listen_from, this, c_ip, server_port);
    insert_listen_thread.detach();
    cout << "Initialization complete" << endl;

}

SequentialServer::~SequentialServer() {
    if(insert_listen_thread.joinable()){
        insert_listen_thread.join();
    }
}

//int SequentialServer::updateServer(int art_id, string content, char *backup_IP, int backup_port) {
//
//    int result;
//    char buf[MAXSTRING];
//    snprintf(buf, MAXSTRING, "Insert;%d;%s", art_id, content.c_str());
//
//    result = udp_send_confirm(backup_IP, backup_port, buf, MAXSTRING);
//    if (result < 0) {
//        printf("ERROR: Insert: Sending update\n");
//        return -1;
//    }
//
//    return result;
//}
//
//
//int SequentialServer::udp_send_confirm(const char *ip, int port, const char *buf, const int buf_size) {
//    int fd;
//    struct addrinfo hints;
//    struct addrinfo* res;
//
//    memset(&hints, 0, sizeof(hints));
//    hints.ai_family = AF_UNSPEC;
//    hints.ai_socktype = SOCK_DGRAM;
//    hints.ai_protocol = 0;
//    hints.ai_flags = AI_ADDRCONFIG;
//
//    if (getaddrinfo(ip, std::to_string(static_cast<long long>(port)).c_str(), &hints, &res) != 0)
//    {
//        return -1;
//    }
//
//    fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
//    if (fd == -1)
//    {
//        freeaddrinfo(res);
//        return -1;
//    }
//
//    int s = sendto(fd, buf, buf_size, 0, res->ai_addr, res->ai_addrlen);
//    if (s == -1)
//    {
//        freeaddrinfo(res);
//        close(fd);
//        return -1;
//    }
//
////    /* Begin listening for confirmation of insert */
////    printf("INFO: Insert: listening for confirmation\n");
////    char recbuf[MAXSTRING];
////    if ((recvfrom(fd, recbuf, MAXSTRING, 0, res->ai_addr, &res->ai_addrlen)) < 0) {
////        freeaddrinfo(res);
////        close(fd);
////        return -1;
////    }
////
////    //print details of the client/peer and the data received
////    printf("Received Data: %s\n" , buf);
//
//    freeaddrinfo(res);
//    close(fd);
//
//    return s;
//}
