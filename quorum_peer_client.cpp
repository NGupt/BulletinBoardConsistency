#include "peer.h"

using namespace std;
using std::vector;
using std::mutex;
using std::thread;
using std::make_shared;

static bool PeerClient::choose_first(const std::pair<int,pair<string,int> > &lhs,
                  const std::pair<int,pair<string,int> > &rhs) {
    bool result = lhs.first < rhs.first;
    cout << lhs.first << rhs.first << result << endl;
    return result;
}

//coordinator to server , asking for version
int PeerClient::udp_ask_vote(PeerClient *q, const char *ip, int port, const char *buf, int buf_size) {
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
        q->serverList.erase(std::remove(q->serverList.begin(), q->serverList.end(), pair<string,int>(ip,port)), q->serverList.end());
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
char * PeerClient::udp_get_updated_pool(PeerClient *q, const char *ip, int port, const char *buf, const int buf_size) {
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
    memset(q->pool_content,'\0', MAXPOOLLENGTH);
    if ((recvfrom(fd, q->pool_content, MAXPOOLLENGTH, 0, res->ai_addr, &res->ai_addrlen)) < 0) {
        perror("timed out, removing server from my serverList");
        q->serverList.erase(std::remove(q->serverList.begin(), q->serverList.end(), pair<string,int>(ip,port)), q->serverList.end());
        freeaddrinfo(res);
        close(fd);
        return q->pool_content;
    }

    freeaddrinfo(res);
    close(fd);
    return q->pool_content;
}

//coordinator to write quorum servers to update their pool
int PeerClient::udp_synchronize(PeerClient *q, const char *ip, int port, const char *buf, int buf_size) {
    int fd;
    struct addrinfo remoteAddr;
    struct addrinfo* res;

    memset(&remoteAddr, 0, sizeof(remoteAddr));
    remoteAddr.ai_family = AF_UNSPEC;
    remoteAddr.ai_socktype = SOCK_DGRAM;
    remoteAddr.ai_protocol = 0;
    remoteAddr.ai_flags = AI_ADDRCONFIG;

    cout << "buf: " << buf <<endl;
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
        q->serverList.erase(std::remove(q->serverList.begin(), q->serverList.end(), pair<string,int>(ip,port)), q->serverList.end());
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
int PeerClient::readVote(PeerClient *q, string req_type) {
    cout << "entering readVote" << endl;
    int serv_version = 0;
    //randomly select some servers
    subscriber_lock.lock();
    random_shuffle(q->serverList.begin(), q->serverList.end());
    subscriber_lock.unlock();

    cout << "after subscribers lock" << endl;

    for(int i=0; i<= (q->serverList.size())/2; ){
        readQuorumList.push_back(make_pair(0, make_pair(q->serverList[i].first,q->serverList[i].second )));
        cout << "after readQuorumList " << endl;
        cout << readQuorumList[i].second.first << ":" << readQuorumList[i].second.second << endl;
        i++;
    }
    int num_votes = 0;

    while (num_votes < readQuorumList.size()) {
        //We need ask our own vote as well !?
        string vote_for("");
        vote_for.append("READ");
        vote_for.append(";");
        if ((serv_version = udp_ask_vote(this, readQuorumList[num_votes].second.first.c_str(), readQuorumList[num_votes].second.second, vote_for.c_str(), sizeof(vote_for))) < 0) {
            cout << "Could not ask for vote" << endl;
        }
            //readQuorumList.push_back(make_pair(serv_version, make_pair(readQuorumList[num_votes].second.first.c_str(), readQuorumList[num_votes].second.second)));
            readQuorumList[num_votes].first = serv_version;
            num_votes++;
            cout << "num_votes " << num_votes << endl;

    }
    num_votes = 0;

    //TODO: get these values
//    std::vector<pair<int,pair<string,int>> > ::iterator max1;
//    max1 = std::max_element(readQuorumList.begin(), readQuorumList.end(), q->choose_first);
//    std::cout << "max1: " << max1->second.first << ":" << max1->second.second;
//    //At this point voting is done
//    //search the read quorum for version given by serv_index, get the server target ip corresponding to it
//    string target_serv_ip = max1->second.first;
//    int serv_port = max1->second.second;


    string target_serv_ip = readQuorumList[1].second.first;;
    int serv_port = readQuorumList[1].second.second;
    string content_request = "POOL;";
    //TODO: read updated pool, add a check for null
    char *pool_content = udp_get_updated_pool(this, target_serv_ip.c_str(), serv_port, content_request.c_str(), content_request.length());
    req_type.append(pool_content);
    udp_fwd_req(target_serv_ip.c_str(), serv_port, client_ip, client_port, req_type.c_str(), strlen(pool_content));
    return 0;
}

int PeerClient::writeVote(PeerClient *q, string write_content) {
    cout << "entering writeVote " << write_content << endl;

    int serv_version = 0;
    //randomly select some servers
    subscriber_lock.lock();
    random_shuffle(q->serverList.begin(), q->serverList.end());
    subscriber_lock.unlock();

    cout << "subscribers , print write quorum" << endl;
    for(int i=0; i<= (q->serverList.size())/2; i++){
        cout << "entering loop " << i << endl;
        writeQuorumList.push_back(make_pair(0, make_pair(q->serverList[q->serverList.size()- i - 1].first,q->serverList[q->serverList.size()- i - 1].second )));
        cout << writeQuorumList[i].second.first << ":" << writeQuorumList[i].second.second << endl;
    }

    int num_votes = 0;

    while (num_votes < writeQuorumList.size()) {
        string vote_for("");
        vote_for.append("WRITE");
        vote_for.append(";");
        cout << "entering while " << write_content << endl;
        if ((serv_version = udp_ask_vote(this, writeQuorumList[num_votes].second.first.c_str(), writeQuorumList[num_votes].second.second, vote_for.c_str(), sizeof(vote_for))) < 0) {
            cout << "Could not ask for vote" << endl;
        }
            //writeQuorumList.push_back(make_pair(serv_version, make_pair(writeQuorumList[num_votes].second.first.c_str(), writeQuorumList[num_votes].second.second)));
        writeQuorumList[num_votes].first = serv_version;

        num_votes++;
            cout << "num_votes " << num_votes << endl;

    }
    num_votes = 0;
//    std::vector<pair<int,pair<string,int>> > ::iterator max1;
//    max1 = std::max_element(writeQuorumList.begin(), writeQuorumList.end(), q->choose_first);
//    std::cout << "max1: " << max1->second.first << ":" << max1->second.second;
//    //At this point voting is done
//    //search the write quorum for version given by serv_index, get the server target ip corresponding to it
//    string target_serv_ip = max1->second.first;
//    int serv_port = max1->second.second;
//    cout << "INFO: Read vote with latest version" << max1->first << " concluded" << endl;


    string target_serv_ip = writeQuorumList[0].second.first;
    int serv_port = writeQuorumList[0].second.second;
    cout << "INFO: Read vote with latest version" << target_serv_ip << " concluded" << endl;

//udp to write
    string update_req = "UPDATE;";
    update_req.append(write_content);
    udp_synchronize(this, target_serv_ip.c_str(), serv_port, update_req.c_str(), update_req.length());

    string request = "POOL;";
    char *pool_content = udp_get_updated_pool(this, target_serv_ip.c_str(), serv_port, request.c_str(), request.length());
    string sync_req = "SYNCHRONIZE;";
    sync_req.append(pool_content);
    //write the same content to rest of the servers
    for(int i=0; i< writeQuorumList.size(); i++) {
        udp_synchronize(this, writeQuorumList[i].second.first.c_str(), writeQuorumList[i].second.second, sync_req.c_str(), sync_req.length());
    }
}

int PeerClient::post(char *content) {
    int output = -1;
    std::string myString(content, strlen(content));
    cout << "before static " << myString << endl;
    //PeerClient *static_post_peer = new PeerClient(server_ip,server_port,coordinator_ip,coordinator_port, isQuorum);
    if (isCoordinator(server_ip)) {
        cout << "before thread" <<endl;
        std::thread post_thread(&PeerClient::writeVote, this, this, myString);
        cout << "before detach" <<endl;
        if(post_thread.joinable()){
            post_thread.join();
        }
        //post_thread.detach();
        output = 1;
        return output;
    }
    return output;
}

int PeerClient::reply(char *content, int index) {
    int output = -1;
    std::string myString(content, strlen(content));
    myString.append(to_string(index));
    cout << "before static " << myString << endl;
    //PeerClient *static_post_peer = new PeerClient(server_ip,server_port,coordinator_ip,coordinator_port, isQuorum);
    if (isCoordinator(server_ip)) {
        cout << "before thread" <<endl;
        std::thread post_thread(&PeerClient::writeVote, this, this, myString);
        cout << "before detach" <<endl;
        if(post_thread.joinable()){
            post_thread.join();
        }
        //post_thread.detach();
        output = 1;
        return output;
    }
    return output;
}

string PeerClient::read() {
    if (isCoordinator(server_ip)) {
        //PeerClient *static_read_peer = new PeerClient(server_ip,server_port,coordinator_ip,coordinator_port, isQuorum);
        string req_type = "READ;";
        std::thread update_thread(&PeerClient::readVote, this, this, req_type);
        if(update_thread.joinable()){
            update_thread.join();
        }
        //update_thread.detach();
        return "You will get updated data from another server\n";
    }
    return "Please connect to coordinator\n";
}

ArticleContent PeerClient::choose(int index) {
    static ArticleContent result;
    result.content = new char[MAXSTRING];
    Article *resultArticle = articlePool.choose(index);
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
}


//get the current articlePool
ArticlePoolStruct PeerClient::getLocalArticle() {
    return articlePool.getArticle();
}

server_list PeerClient::get_server_list() {
    server_list output;
    if (isCoordinator(server_ip)) {
        now->outputServerList(now);
        return now->buildServerList();
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
        return now->joinServer(ip, port);
    } else {
        if (join_server_1(ip, port, pclnt) == (int *) NULL) {
            clnt_perror(pclnt, "call failed");
            return -1;
        }
        return 0;
    }
}
