#include "quorum_peer_client.h"

using namespace std;
using std::vector;
using std::mutex;
using std::thread;
using std::make_shared;

bool choose_first(const std::pair<int, string> &lhs,
                  const std::pair<int, string> &rhs) {
    return lhs.first < rhs.first ;
}

QuoServer::QuoServer(string ip, int port, string coordinator_ip, int coordinator_port) {
    this->server_ip = ip;
    this->server_port = port;
    this->coordinator_ip = coordinator_ip;
    this->coordinator_port = coordinator_port;
  //  now = this;
    char *c_ip = new char[coordinator_ip.length() + 1];
    strcpy(c_ip, coordinator_ip.c_str());
    char *o_ip = new char[ip.length() + 1];
    strcpy(o_ip, ip.c_str());

    pclnt = clnt_create(c_ip, COMMUNICATE_PROG, COMMUNICATE_VERSION, "udp");
    if (!pclnt) {printf("ERROR: Creating clnt to coordinator %s\n", c_ip);
        exit(1);
    }

    if (isCoordinator(o_ip)) {
        cout << "INFO: Coord starting" << endl;
        subscriber_lock.lock();
        subscribers.push_back(NULL);
        subscriber_lock.unlock();
    } else {
        join_server(o_ip, port);  // Our coordinator is not part of client accessible server list in case of quorum consistency
        get_server_list();
    }

    sockaddr_in si_me;
    insert_listen_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (insert_listen_fd == -1) {
        perror("Error: creating socket");
        throw;
    }
    int optval = 1;
    setsockopt(insert_listen_fd, SOL_SOCKET, SO_REUSEADDR, (const void *) &optval, sizeof(int));
    memset(&si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_addr.s_addr= htonl(INADDR_ANY);

    if (isCoordinator(o_ip)) {
        si_me.sin_port = htons(coordinator_port);
        cout << "Coordinator started" << endl;
    } else {
        si_me.sin_port = htons(server_port);
        cout << "Backup peer server started" << endl;
    }

    if (bind(insert_listen_fd, (struct sockaddr*)&si_me, sizeof(si_me)) == -1)     {
        close(insert_listen_fd);
        insert_listen_fd = -1;
        perror("binding socket");
        throw;
    }

    if(!isCoordinator(o_ip)) {
        get_server_list();
        //if server joined later, then get the latest article copy
        if(articlePool.read() == ""){
            char *temp_articles = *read_1(pclnt);
            // cout << "empty article pool, output after update:\n" << temp_articles << endl;
            decode_articles(temp_articles);
            cout << "existing articles were\n" << articlePool.read() << endl;
        }
        //create listening udp thread
        insert_listen_thread = thread(udp_recv_vote_req, this, c_ip, server_port);
        insert_listen_thread.detach();
    }
    cout << "Initialization complete" << endl;

    std::cout << ".....Completed Server creation.....\n";
}

QuoServer::~QuoServer() {
    articlePool.releaseAll();
    if (pclnt) {
        clnt_destroy(pclnt);
    }
    if(insert_listen_thread.joinable()){
        insert_listen_thread.join();
    }
    if(update_thread.joinable()){
        update_thread.join();
    }
    close(this->insert_listen_fd);
}

//coordinator to server , asking for version
int QuoServer::udp_ask_vote(const char *ip, int port, const char *buf, int buf_size) {
    int fd;
    struct addrinfo remoteAddr;
    struct addrinfo* res;

    memset(&remoteAddr, 0, sizeof(remoteAddr));
    remoteAddr.ai_family = AF_UNSPEC;
    remoteAddr.ai_socktype = SOCK_DGRAM;
    remoteAddr.ai_protocol = 0;
    remoteAddr.ai_flags = AI_ADDRCONFIG;

    //cout << "buf: " << buf <<endl;
    if (getaddrinfo(ip, std::to_string(static_cast<long long>(port)).c_str(), &remoteAddr, &res) != 0) {
        perror("cant get addressinfo");
        freeaddrinfo(res);
        return -1;
    }

    if ((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
        perror("cant create socket");
        freeaddrinfo(res);
        close(fd);
        return -1;
    }

    if ((sendto(fd, buf, buf_size, 0, res->ai_addr, res->ai_addrlen)) == -1) {
        perror("cant send ");
        freeaddrinfo(res);
        close(fd);
        return -1;
    }

    /* Begin listening for confirmation of, else remove server from serverList*/
    char recbuf[MAXPOOLLENGTH];
    if ((recvfrom(fd, recbuf, MAXPOOLLENGTH, 0, res->ai_addr, &res->ai_addrlen)) < 0) {
        perror("timed out, removing server from my serverList");
        serverList.erase(std::remove(serverList.begin(), serverList.end(), pair<string,int>(ip,port)), serverList.end());
        freeaddrinfo(res);
        close(fd);
        return 0;
    }

    string recvbuf(recbuf, strlen(recbuf));
    int version_pos = recvbuf.find(buf);
    int version = atoi(recvbuf.substr(version_pos + 1).c_str());
    freeaddrinfo(res);
    close(fd);
    return version;
}


//coordinator to selected read quorum server so that server can send things to client
int QuoServer::udp_fwd_req(const char *serv_ip, int serv_port, const char *client_ip, int client_port, const char *buf, int buf_size) {
    int fd;
    struct addrinfo remoteAddr;
    struct addrinfo* res;

    memset(&remoteAddr, 0, sizeof(remoteAddr));
    remoteAddr.ai_family = AF_UNSPEC;
    remoteAddr.ai_socktype = SOCK_DGRAM;
    remoteAddr.ai_protocol = 0;
    remoteAddr.ai_flags = AI_ADDRCONFIG;

    //cout << "buf: " << buf <<endl;
    if (getaddrinfo(serv_ip, std::to_string(static_cast<long long>(serv_port)).c_str(), &remoteAddr, &res) != 0) {
        perror("cant get addressinfo");
        freeaddrinfo(res);
        return -1;
    }

    if ((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
        perror("cant create socket");
        freeaddrinfo(res);
        close(fd);
        return -1;
    }

    const int new_buf_length = buf_size + MAXIP + 4;

    string new_buf("");
    new_buf.append("FWD_REQ");
    new_buf.append(";");
    new_buf.append(client_ip);
    new_buf.append(";");
    new_buf = std::to_string(client_port);
    new_buf.append(";");
    new_buf.append(buf);
    if ((sendto(fd, new_buf.c_str(), new_buf_length, 0, res->ai_addr, res->ai_addrlen)) == -1) {
        perror("cant send ");
        freeaddrinfo(res);
        close(fd);
        return -1;
    }
    freeaddrinfo(res);
    close(fd);
    return 0;
}

//coordinator to latest pool
string QuoServer::udp_get_updated_pool(const char *ip, int port, const char *buf, const int buf_size) {
    int fd;
    struct addrinfo remoteAddr;
    struct addrinfo* res;

    memset(&remoteAddr, 0, sizeof(remoteAddr));
    remoteAddr.ai_family = AF_UNSPEC;
    remoteAddr.ai_socktype = SOCK_DGRAM;
    remoteAddr.ai_protocol = 0;
    remoteAddr.ai_flags = AI_ADDRCONFIG;

    //cout << "buf: " << buf <<endl;
    if (getaddrinfo(ip, std::to_string(static_cast<long long>(port)).c_str(), &remoteAddr, &res) != 0) {
        perror("cant get addressinfo");
        freeaddrinfo(res);
        return NULL;
    }

    if ((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
        perror("cant create socket");
        freeaddrinfo(res);
        close(fd);
        return NULL;
    }

    if ((sendto(fd, buf, buf_size, 0, res->ai_addr, res->ai_addrlen)) == -1) {
        perror("cant send ");
        freeaddrinfo(res);
        close(fd);
        return NULL;
    }

    /* Begin listening for confirmation of, else remove server from serverList*/
    char pool_content[MAXPOOLLENGTH];
    memset(pool_content,'\0', MAXPOOLLENGTH);
    if ((recvfrom(fd, pool_content, MAXPOOLLENGTH, 0, res->ai_addr, &res->ai_addrlen)) < 0) {
        perror("timed out, removing server from my serverList");
        serverList.erase(std::remove(serverList.begin(), serverList.end(), pair<string,int>(ip,port)), serverList.end());
        freeaddrinfo(res);
        close(fd);
        return pool_content;
    }

    freeaddrinfo(res);
    close(fd);
    return pool_content;
}

//coordinator to write quorum servers to update their pool
int QuoServer::udp_synchronize(const char *ip, int port, const char *buf, int buf_size) {
    int fd;
    struct addrinfo remoteAddr;
    struct addrinfo* res;

    memset(&remoteAddr, 0, sizeof(remoteAddr));
    remoteAddr.ai_family = AF_UNSPEC;
    remoteAddr.ai_socktype = SOCK_DGRAM;
    remoteAddr.ai_protocol = 0;
    remoteAddr.ai_flags = AI_ADDRCONFIG;

    //cout << "buf: " << buf <<endl;
    if (getaddrinfo(ip, std::to_string(static_cast<long long>(port)).c_str(), &remoteAddr, &res) != 0) {
        perror("cant get addressinfo");
        freeaddrinfo(res);
        return -1;
    }

    if ((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
        perror("cant create socket");
        freeaddrinfo(res);
        close(fd);
        return -1;
    }

    if ((sendto(fd, buf, buf_size, 0, res->ai_addr, res->ai_addrlen)) == -1) {
        perror("cant send ");
        freeaddrinfo(res);
        close(fd);
        return -1;
    }

    /* Begin listening for confirmation of, else remove server from serverList*/
    char recbuf[MAXPOOLLENGTH];
    if ((recvfrom(fd, recbuf, MAXPOOLLENGTH, 0, res->ai_addr, &res->ai_addrlen)) < 0) {
        perror("timed out, removing server from my serverList");
        serverList.erase(std::remove(serverList.begin(), serverList.end(), pair<string,int>(ip,port)), serverList.end());
        freeaddrinfo(res);
        close(fd);
        return 0;
    }

    freeaddrinfo(res);
    close(fd);
    return 0;
}


//only for server...........
void QuoServer::udp_recv_vote_req(QuoServer *s, string r_ip, int port){
    struct sockaddr_in remote_addr;
    const char *remote_ip = r_ip.c_str();
    socklen_t slen = sizeof(remote_addr);

    memset((char *) &remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(port);
    if (inet_aton(remote_ip, &remote_addr.sin_addr) == 0) {
        perror("inet_aton failed");
    }

    char vote_req[MAXPOOLLENGTH];
    // Clear the buffer by filling null, it might have previously received data
    memset(vote_req, '\0', MAXPOOLLENGTH);

    // Try to receive some data; This is a blocking call
    while (1) {
        if ((recvfrom(s->insert_listen_fd, vote_req, MAXPOOLLENGTH, 0, (struct sockaddr *) &remote_addr, &slen)) < 0) {
            perror("recvfrom()");
            exit(1);
        }

        int pos = 0;
        std::string delimiter = ";";
        std::string first_command = "";

        string req(vote_req, strlen(vote_req));
        pos = req.find(delimiter);
        first_command = req.substr(0, pos);   //returning line
        req = req.substr(pos+1);
        if((strcmp(first_command.c_str(),"READ") == 0)||(strcmp(first_command.c_str(),"WRITE") == 0)) {
            if (s->readlock->try_lock()) {
                printf("INFO: Read-locking %d\n", s->articlePool.count);
                strcat(vote_req, to_string(s->articlePool.count).c_str());
                //resend the received data to check if server is up
                cout << "sending received req along with version number " << vote_req << endl;

                if ((sendto(s->insert_listen_fd, vote_req, MAXPOOLLENGTH, 0, (struct sockaddr *) &remote_addr, slen)) ==
                    -1) {
                    close(s->insert_listen_fd);
                    s->insert_listen_fd = -1;
                    perror("Error: acknowledging the received string");
                    throw;
                }
            }
            s->readlock->unlock(); //clear the lock
        } else if (strcmp(first_command.c_str(),"FWD_REQ") == 0) {
            pos = req.find(delimiter);
            string client_ip = req.substr(0, pos);   //returning line
            req = req.substr(pos+1);
            pos = req.find(delimiter);
            int client_port = atoi(req.substr(0, pos).c_str());   //returning line
            string pool_content = req.substr(pos+1);
            //TODO: create client socket and send to that
            int client_sock = -1;
            if (s->readlock->try_lock()) {
                //resend the received data to check if server is up
                cout << "sending received pool content to client " << client_ip << ":" << client_port << endl;
                if ((sendto(client_sock, pool_content.c_str(), MAXPOOLLENGTH, 0, (struct sockaddr *) &remote_addr, slen)) ==
                    -1) {
                    close(client_sock);
                    perror("Error: acknowledging the received string");
                    throw;
                }
            }
            s->readlock->unlock(); //clear the lock
            close(client_sock);
        } else if (strcmp(first_command.c_str(),"POOL") == 0){
            if (s->readlock->try_lock()) {
                string pool_content = s->articlePool.read();
                cout << "returning latest pool content to coordinator " << pool_content << endl;
                if ((sendto(s->insert_listen_fd, pool_content.c_str(), MAXPOOLLENGTH, 0,
                            (struct sockaddr *) &remote_addr, slen)) ==
                    -1) {
                    close(s->insert_listen_fd);
                    s->insert_listen_fd = -1;
                    perror("Error: acknowledging the received string");
                    throw;
                }
            }
            s->readlock->unlock(); //clear the lock
        } else if (strcmp(first_command.c_str(),"SYNCHRONIZE") == 0){
            if (s->writelock->try_lock()) {
                string content = req.substr(pos+1);
                s->articlePool.releaseAll();
                char *pool_content = new char[content.length() + 1];
                std::strcpy(pool_content, content.c_str());
                s->decode_articles(pool_content); //will decode and save the pool content to pool of server
            }
            s->writelock->unlock(); //clear the lock
            //confirming back to coordinator that synchronization on this server happened
            if ((sendto(s->insert_listen_fd, vote_req, MAXPOOLLENGTH, 0, (struct sockaddr *) &remote_addr, slen)) ==
                -1) {
                close(s->insert_listen_fd);
                s->insert_listen_fd = -1;
                perror("Error: acknowledging the received string");
                throw;
            }
        }
    }
}

/* Starts a read vote for the identified article */
/* Returns when vote is completed. */
//TODO: ReadVote only to be called by coordinator
int QuoServer::ReadVote(ArticlePool pool) {
// initializing the vote to read article
    vector<int> versions;
    versions.push_back(0);
    int serv_version = 0;
    auto it = ReadQuorumList.begin();
    int start_size = serverList.size();
    int num_votes = 0;

    //TODO: randomly select some servers
    while (num_votes <= start_size / 2) {
        if (it == ReadQuorumList.end()) {
            it = ReadQuorumList.begin();
        }
        //We need ask our own vote as well !?
        string vote_for("");
        vote_for.append("READ");
        vote_for.append(";");
        if ((serv_version = udp_ask_vote(serverList[num_votes].first.c_str(), serverList[num_votes].second, vote_for.c_str(), sizeof(vote_for))) < 0) {
            cout << "Could not ask for vote" << endl;
        } else {
            ReadQuorumList.push_back(make_pair(serv_version, make_pair(serverList[num_votes].first, serverList[num_votes].second)));
            versions.push_back(serv_version);
            num_votes++;
        }
    }

    //TODO: get these values
    int client_port, buf_size;
    const char *client_ip;
    auto max1 = std::max_element(ReadQuorumList.begin(), ReadQuorumList.end(), choose_first);
    std::cout << "max1: " << max1->second.first << ":" << max1->second.second;
    //At this point voting is done
    //search the read quorum for version given by serv_index, get the server target ip corresponding to it
    string target_serv_ip = max1->second.first;
    int serv_port = max1->second.second;
    string request = "POOL;";
    crit.lock();
    //TODO: read updated pool
    string pool_content = udp_get_updated_pool(max1->second.first.c_str(), max1->second.second, request.c_str(), MAXPOOLLENGTH);
    crit.unlock();
    udp_fwd_req(target_serv_ip.c_str(), serv_port, client_ip, client_port, pool_content.c_str(), buf_size);
    return 0;
}

int QuoServer::WriteVote(ArticlePool pool) {
    vector<int> versions;
    versions.push_back(0);
    int serv_version = 0;

    auto it = WriteQuorumList.begin();
    int start_size = serverList.size();
    int num_votes = 0;

    //TODO: randomly select some servers
    while (num_votes <= start_size / 2) {
        if (it == WriteQuorumList.end()) {
            it = WriteQuorumList.begin();
        }
        string vote_for("");
        vote_for.append("WRITE");
        vote_for.append(";");
        if ((serv_version = udp_ask_vote(serverList[num_votes].first.c_str(), serverList[num_votes].second, vote_for.c_str(), sizeof(vote_for))) < 0) {
            cout << "Could not ask for vote" << endl;
        } else {
            WriteQuorumList.push_back(make_pair(serv_version, make_pair(serverList[num_votes].first, serverList[num_votes].second)));
            versions.push_back(serv_version);
            num_votes++;
        }
    }
    auto max1 = std::max_element(WriteQuorumList.begin(), WriteQuorumList.end(), choose_first);
    //At this point voting is done
    //search the write quorum for version given by serv_index, get the server target ip corresponding to it
    string target_serv_ip = max1->second.first;
    int serv_port = max1->second.second;

    cout << "INFO: Read vote with latest version" << max1->first << " concluded" << endl;
    string request = "POOL;";
    crit.lock();
    string pool_content = udp_get_updated_pool(target_serv_ip.c_str(), serv_port, request.c_str(), MAXPOOLLENGTH);
    crit.unlock();
    string sync_req = "SYNCHRONIZE;";
    sync_req.append(pool_content);
    //write the same content to rest of the servers
    for(int i=0; i< WriteQuorumList.size(); i++) {
        udp_synchronize(WriteQuorumList[i].second.first.c_str(), WriteQuorumList[i].second.second, sync_req.c_str(), MAXPOOLLENGTH);
    }
}

int QuoServer::getReadVote(QuoServer *q) {
    printf("INFO: Read vote %d requested\n", q->articlePool.count);
    int vote = !isWriter(q);
    if (vote) {
        cout << "INFO:" << server_ip << "voted for READ " << q->articlePool.count << endl;
    } else {
        cout << "INFO:" << server_ip << "voted against READ " << q->articlePool.count << endl;
        return 0;
    }

    if (readlock->try_lock())
        printf("INFO: Read-locking %d\n", q->articlePool.count);
    printf("INFO: Read vote sent\n");
    return vote;
}

int QuoServer::clearReadVote(QuoServer *q)
{
    printf("INFO: Clearing READ %d\n", q->articlePool.count);
    if (isReader(q)) {
        readlock->unlock();
    }

    return 0;
}
