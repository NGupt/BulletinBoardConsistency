#include "peer.h"

using namespace std;
using std::vector;
using std::mutex;
using std::thread;
using std::make_shared;

bool choose_first(const std::pair<int,pair<string,int> > &lhs,
                  const std::pair<int,pair<string,int> > &rhs) {
    return lhs.first < rhs.first ;
}

//PeerClient::PeerClient(string ip, int port, string coordinator_ip, int coordinator_port) {
//    this->server_ip = ip;
//    this->server_port = port;
//    this->coordinator_ip = coordinator_ip;
//    this->coordinator_port = coordinator_port;
//  //  now = this;
//    char *c_ip = new char[coordinator_ip.length() + 1];
//    strcpy(c_ip, coordinator_ip.c_str());
//    char *o_ip = new char[ip.length() + 1];
//    strcpy(o_ip, ip.c_str());
//
//    pclnt = clnt_create(c_ip, COMMUNICATE_PROG, COMMUNICATE_VERSION, "udp");
//    if (!pclnt) {printf("ERROR: Creating clnt to coordinator %s\n", c_ip);
//        exit(1);
//    }
//
//    if (isCoordinator(o_ip)) {
//        cout << "INFO: Coord starting" << endl;
//        subscriber_lock.lock();
//        subscribers.push_back(NULL);
//        subscriber_lock.unlock();
//    } else {
//        join_server(o_ip, port);  // Our coordinator is not part of client accessible server list in case of quorum consistency
//        get_server_list();
//    }
//
//    sockaddr_in si_me;
//    insert_listen_fd = socket(AF_INET, SOCK_DGRAM, 0);
//    if (insert_listen_fd == -1) {
//        perror("Error: creating socket");
//        throw;
//    }
//    int optval = 1;
//    setsockopt(insert_listen_fd, SOL_SOCKET, SO_REUSEADDR, (const void *) &optval, sizeof(int));
//    memset(&si_me, 0, sizeof(si_me));
//    si_me.sin_family = AF_INET;
//    si_me.sin_addr.s_addr= htonl(INADDR_ANY);
//
//    if (isCoordinator(o_ip)) {
//        si_me.sin_port = htons(coordinator_port);
//        cout << "Coordinator started" << endl;
//    } else {
//        si_me.sin_port = htons(server_port);
//        cout << "Backup peer server started" << endl;
//    }
//
//    if (bind(insert_listen_fd, (struct sockaddr*)&si_me, sizeof(si_me)) == -1)     {
//        close(insert_listen_fd);
//        insert_listen_fd = -1;
//        perror("binding socket");
//        throw;
//    }
//
//    if(!isCoordinator(o_ip)) {
//        get_server_list();
//        //if server joined later, then get the latest article copy
//        if(articlePool.read() == ""){
//            char *temp_articles = *read_1(pclnt);
//            // cout << "empty article pool, output after update:\n" << temp_articles << endl;
//            decode_articles(temp_articles);
//            cout << "existing articles were\n" << articlePool.read() << endl;
//        }
//        //create listening udp thread
//        insert_listen_thread = thread(udp_recv_vote_req, this, c_ip, server_port);
//        insert_listen_thread.detach();
//    }
//    cout << "Initialization complete" << endl;
//
//    std::cout << ".....Completed Server creation.....\n";
//}

//coordinator to server , asking for version
int PeerClient::udp_ask_vote(const char *ip, int port, const char *buf, int buf_size) {
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
int PeerClient::udp_fwd_req(const char *serv_ip, int serv_port, const char *client_ip, int client_port, const char *buf, int buf_size) {
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
char * PeerClient::udp_get_updated_pool(const char *ip, int port, const char *buf, const int buf_size) {
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
int PeerClient::udp_synchronize(const char *ip, int port, const char *buf, int buf_size) {
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


/* Starts a read vote for the identified article */
/* Returns when vote is completed. */
//TODO: readVote only to be called by coordinator
int PeerClient::readVote(ArticlePool pool) {
// initializing the vote to read article
    vector<int> versions;
    versions.push_back(0);
    int serv_version = 0;
    auto it = readQuorumList.begin();
    int start_size = serverList.size();
    int num_votes = 0;

    //TODO: randomly select some servers
    while (num_votes <= start_size / 2) {
        if (it == readQuorumList.end()) {
            it = readQuorumList.begin();
        }
        //We need ask our own vote as well !?
        string vote_for("");
        vote_for.append("READ");
        vote_for.append(";");
        if ((serv_version = udp_ask_vote(serverList[num_votes].first.c_str(), serverList[num_votes].second, vote_for.c_str(), sizeof(vote_for))) < 0) {
            cout << "Could not ask for vote" << endl;
        } else {
            readQuorumList.push_back(make_pair(serv_version, make_pair(serverList[num_votes].first, serverList[num_votes].second)));
            versions.push_back(serv_version);
            num_votes++;
        }
    }

    //TODO: get these values
    int client_port, buf_size;
    const char *client_ip;
    std::vector<pair<int,pair<string,int>> > ::iterator max1;
    max1 = std::max_element(readQuorumList.begin(), readQuorumList.end(), choose_first);
    std::cout << "max1: " << max1->second.first << ":" << max1->second.second;
    //At this point voting is done
    //search the read quorum for version given by serv_index, get the server target ip corresponding to it
    string target_serv_ip = max1->second.first;
    int serv_port = max1->second.second;
    string request = "POOL;";
    crit.lock();
    //TODO: read updated pool
    char *pool_content = udp_get_updated_pool(max1->second.first.c_str(), max1->second.second, request.c_str(), MAXPOOLLENGTH);
    crit.unlock();
    udp_fwd_req(target_serv_ip.c_str(), serv_port, client_ip, client_port, pool_content, buf_size);
    return 0;
}

int PeerClient::writeVote(ArticlePool pool) {
    vector<int> versions;
    versions.push_back(0);
    int serv_version = 0;

    auto it = writeQuorumList.begin();
    int start_size = serverList.size();
    int num_votes = 0;

    //TODO: randomly select some servers
    while (num_votes <= start_size / 2) {
        if (it == writeQuorumList.end()) {
            it = writeQuorumList.begin();
        }
        string vote_for("");
        vote_for.append("WRITE");
        vote_for.append(";");
        if ((serv_version = udp_ask_vote(serverList[num_votes].first.c_str(), serverList[num_votes].second, vote_for.c_str(), sizeof(vote_for))) < 0) {
            cout << "Could not ask for vote" << endl;
        } else {
            writeQuorumList.push_back(make_pair(serv_version, make_pair(serverList[num_votes].first, serverList[num_votes].second)));
            versions.push_back(serv_version);
            num_votes++;
        }
    }
    std::vector<pair<int,pair<string,int>> > ::iterator max1;
    max1 = std::max_element(writeQuorumList.begin(), writeQuorumList.end(), choose_first);
    //At this point voting is done
    //search the write quorum for version given by serv_index, get the server target ip corresponding to it
    string target_serv_ip = max1->second.first;
    int serv_port = max1->second.second;

    cout << "INFO: Read vote with latest version" << max1->first << " concluded" << endl;
    string request = "POOL;";
    crit.lock();
    char *pool_content = udp_get_updated_pool(target_serv_ip.c_str(), serv_port, request.c_str(), MAXPOOLLENGTH);
    crit.unlock();
    string sync_req = "SYNCHRONIZE;";
    sync_req.append(pool_content);
    //write the same content to rest of the servers
    for(int i=0; i< writeQuorumList.size(); i++) {
        udp_synchronize(writeQuorumList[i].second.first.c_str(), writeQuorumList[i].second.second, sync_req.c_str(), MAXPOOLLENGTH);
    }
}







int PeerClient::post(char *content) {
    int output = -1;
//    std::string myString(content, strlen(content));
//    output = articlePool.post(myString);
//    if (isCoordinator(server_ip)) {
//        update_thread = thread(updateAllServers, this, articlePool, 0);
//        update_thread.detach();
//        return output;
//    } else {
//        update_thread = thread(post_1, content, pclnt);
//        update_thread.detach();
//        if (output == 0) {
//            clnt_perror(pclnt, "Cannot post");
//        } else {
//            std::cout << "Post the article " << output << " " << content << std::endl;
//        }
//    }
    return output;
}


int PeerClient::reply(char *content, int index) {
    int output = -1;
//    std::string myString(content, strlen(content));
//    output = articlePool.reply(myString, index);
//    if (isCoordinator(server_ip)) {
//        update_thread = thread(updateAllServers, this, articlePool, index);
//        update_thread.detach();
//        return output;
//    } else {
//        update_thread = thread(reply_1, content, index, pclnt);
//        update_thread.detach();
//        if (output == 0) {
//            clnt_perror(pclnt, "Cannot reply");
//        }
//    }
    return output;
}

string PeerClient::read() {
    return articlePool.read();
}

ArticleContent PeerClient::choose(int index) {
    static ArticleContent result;
//    result.content = new char[MAXSTRING];
//    Article *resultArticle = articlePool.choose(index);
//    if (resultArticle == NULL) {
//        strcpy(result.content, "");
//        result.index = 0;
//        cout << "The article with id " << index << " doesn't exist in the server." << endl;
//    } else {
//        strcpy(result.content, resultArticle->content.c_str());
//        result.index = resultArticle->index;
//        cout << "The client choose the article: " << endl;
//        cout << result.index << " " << result.content << endl;
//    }
    return result;
}


//get the current articlePool
ArticlePoolStruct PeerClient::getLocalArticle() {
    return articlePool.getArticle();
}


int PeerClient::send_flag(int flag) {
    auto output = send_flag_1(flag, pclnt);
    if (output == (int *) NULL) {
        clnt_perror (pclnt, "call failed");
    }
    return *output;
}

server_list PeerClient::get_server_list() {
    server_list output;
    if (isCoordinator(server_ip)) {
//        now->outputServerList(now);
//        return now->buildServerList();
    } else {
        auto output = get_server_list_1(pclnt);
        if (output == (server_list *) NULL) {
            clnt_perror(pclnt, "call failed");
        }
        std::cout << "server_list is :" << endl;
        for (int i = 0; i < output->server_list_len; i++) {
            std::cout << (output->server_list_val + i)->ip << ":" << (output->server_list_val + i)->port << endl;
        }
        return *output;
    }
}

int PeerClient::join_server(IP ip, int port) {
    if (isCoordinator(ip)) {
 //       return now->joinServer(ip, port);
    } else {
        if (join_server_1(ip, port, pclnt) == (int *) NULL) {
            clnt_perror(pclnt, "call failed");
            return -1;
        }
        return 0;
    }
}
