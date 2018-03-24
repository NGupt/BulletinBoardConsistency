#include "quorum_peer_client.h"

using namespace std;
using std::vector;
using std::mutex;
using std::make_shared;

#include "quorum.h"

//
///////////////////////serverside//////////////////////////////
//int *
//queue_up_1_svc(int art_id, char *content,  struct svc_req *rqstp)
//{
//    static int  result;
//    return &result;
//}
//
//int *
//write_up_1_svc(ArticlePool art,  struct svc_req *rqstp)
//{
//    static int  result;
//    return &result;
//}
//
//int *
//fetch_write_vote_1_svc(int id, int mod_time,  struct svc_req *rqstp)
//{
//    static int  result;
//    return &result;
//}
//
//int *
//read_vote_1_svc(int id, int mod_time,  struct svc_req *rqstp)
//{
//    static int  result;
//    return &result;
//}
//////////////////////////serverside////////////////////////
//
//
//
/////////////////////////clientside////////////////////////
//if (queue_up_1(queue_up_1_art_id, queue_up_1_content, clnt) == (int *) NULL) {
//clnt_perror (pclnt, "call failed");
//}
//if (write_up_1(write_up_1_art, clnt) == (int *) NULL) {
//clnt_perror (pclnt, "call failed");
//}
//if (fetch_write_vote_1(fetch_write_vote_1_id, fetch_write_vote_1_mod_time, clnt) == (int *) NULL) {
//clnt_perror (pclnt, "call failed");
//}
//if (read_vote_1(read_vote_1_id, read_vote_1_mod_time, clnt) == (int *) NULL) {
//clnt_perror (pclnt, "call failed");
//}
////////////////////////clientside//////////////////////////






int QuoServer::synchronizer(ArticlePool art) {
    char *temp_articles = *read_1(pclnt);
    // cout << "empty article pool, output after update:\n" << temp_articles << endl;
    articlePool.releaseAll();  //for synchronization, clearing the bulletin board first
    decode_articles(temp_articles);
    cout << "existing articles were\n" << articlePool.read() << endl;
    return 0;
}

QuoServer::QuoServer(string ip, int port, string coordinator_ip, int coordinator_port) {
    this->server_ip = ip;
    this->server_port = port;
    this->coordinator_ip = coordinator_ip;
    this->coordinator_port = coordinator_port;
    now = this;
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
//        num_confirmations_read = 0;
//        num_confirmations_write = 0;
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
        insert_listen_thread = thread(listen_from, this, c_ip, server_port);
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

int QuoServer::udp_ask_vote(const char *ip, int port, const char *buf, const int buf_size) {
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

    int version_pos = recbuf.find(buf);
    int version = atoi(recbuf.substr(content_pos + 1).c_str());
    freeaddrinfo(res);
    close(fd);
    return version;
}



int QuoServer::udp_fwd_req(const char *serv_ip, int serv_port, const char *client_ip, int client_port, const char *buf, const int buf_size) {
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
    new_buf.append(client_ip);
    new_buf.append(";");
    new_buf = std::to_string(client_port);
    new_buf.append(";");
    new_buf.append(buf);
    if ((sendto(fd, new_buf, new_buf_length, 0, res->ai_addr, res->ai_addrlen)) == -1) {
        perror("cant send ");
        freeaddrinfo(res);
        close(fd);
        return -1;
    }
    freeaddrinfo(res);
    close(fd);
    return 0;
}




void QuoServer::udp_receive_vote(QuoServer *s,string r_ip, int port){
    struct sockaddr_in remote_addr, self_addr;
    const char *remote_ip = r_ip.c_str();
    int bytes = 0;
    socklen_t slen = sizeof(remote_addr);

    memset((char *) &remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(port);
    if (inet_aton(remote_ip, &remote_addr.sin_addr) == 0) {
        perror("inet_aton failed");
    }

    char article_update[MAXPOOLLENGTH];
    // Clear the buffer by filling null, it might have previously received data
    memset(article_update, '\0', MAXPOOLLENGTH);

    int pos = 0;
    int index = 0;
    int reply_index = 0;

    // Try to receive some data; This is a blocking call
    while (1) {
        if ((recvfrom(s->insert_listen_fd, article_update, MAXPOOLLENGTH, 0, (struct sockaddr *) &remote_addr, &slen)) < 0) {
            perror("recvfrom()");
            exit(1);
        }
        // cout << "listened "  << " " << article_update << endl;
        std::string delimiter = ";";
        string article(article_update, strlen(article_update));
        pos = article.find(delimiter);
        index = atoi((article.substr(0, pos)).c_str());

        string remaining_content = article.substr(pos+1);
        pos = remaining_content.find(delimiter);
        reply_index = 0;
        if((remaining_content.substr(0, pos)) != "") {
            reply_index = atoi((remaining_content.substr(0, pos)).c_str());
        }
        string content = remaining_content.substr(pos+1);
        //cout << "content " << content << " index " << index << endl;
        if(s->articlePool.choose(index) == NULL)
            s->articlePool.storeArticle(content,reply_index); //with index = 0 , it is same as post

        //resend the received data to check if server is up
        if ((sendto(s->insert_listen_fd, article_update, MAXPOOLLENGTH, 0, (struct sockaddr *) &remote_addr, slen)) == -1) {
            close(s->insert_listen_fd);
            s->insert_listen_fd = -1;
            perror("Error: acknowledging the received string");
            throw;
        }
    }
}


bool choose_first(const std::pair<int, string> &lhs,
                   const std::pair<int, string> &rhs) {
    return lhs.first < rhs.first ;
}

/* Starts a read vote for the identified article */
/* Returns when vote is completed. */
//TODO:Has to be integrated with ArticlePool structure
int QuoServer::ReadVote(ArticlePool pool) {
// initializing the vote to read article
    vector<int> versions;
    versions.push_back(0);


    vector<pair<int,string>> ReadQuorumList;
    //So here we need to get number of connections before starting the vote.
    // Hence we need to iterate over the server list and push them to our vector
//    for (int i=0; i< serverList.size();i++) {
//        voters.push_back(i);
//    }
    int serv_version = 0;

    auto it = ReadQuorumList.begin();
    //Should be number of active clients
    int start_size = serverList.size() ;
    int num_votes = 0;

    //int id = art_tree[i]->first;
    //int id = articlePool.count; //number of articles in pool
    //randomly select some servers
    while(num_votes <= start_size/2) {
        if (it == ReadQuorumList.end()) {
            it = ReadQuorumList.begin();
        }
        //We need ask our own vote as well !?
        char *vote_for = "Read";
        if ((*it) != NULL) {
            //If not an active connection
//            if () //TODO: How to check in udp whether a server is active or not
//            {
//                it = voters.erase(it);
//            }
            if ((serv_version = udp_ask_vote(serverList[num_votes].first.c_str(), serverList[num_votes].second,
                                             vote_for, vote_for.length())) < 0)
                // TODO:We send the request to server
            {
                cout << "Could not ask for vote" << endl;
            } else {
                ReadQuorumList.push_back(serv_version, make_pair(serverList[num_votes].first.c_str()));
                versions.push_back(serv_version);
                num_votes++;
            }
        }
    }
    auto max1 = std::max_element(ReadQuorumList.begin(), ReadQuorumList.end(), choose_first);
    cout << "max1: " << max1->second;
    if(self_version <= max1->first) {
        //TODO search the read quorum for version given by serv_index, get the server target ip corresponding to it
        string target_serv_ip = max1->second;
        udp_fwd_req(target_serv_ip, serv_port, client_ip, client_port, *buf, buf_size);
    }
    else
        self_version = max1->first;

//
//    char *response;
//    //To check for vote response
//    if (udp_receive_vote(this, serverList[i].first.c_str(), serverList[i].second) < 0) {
//        cout << "Could not receive vote " << endl;
//    } else if (response[0] == 0) {
//        //Response YES received
//        num_votes++;
//    } else {
//        it++;
//    }
//
//    {
//        int vote = getReadVote(id);
//        if (vote) {
//            it = voters.erase(it);
//            num_votes++;
//        }
//        else {
//            ++
//                    it;
//        }
//    }
}


    //At this point voting is done
    /* Vote done. Push current version to all subscribers */
    cout << "INFO: Read vote " << id << " concluded" << endl;
    crit.lock();
    ArticlePool pool_current_version = art_tree[id]; //TODO: Sending updated article
    crit.unlock();
    synchronizer(pool_current_version);
    clearReadVote(id); //Clearing all votes

    return 0;
}

int QuoServer::getReadVote(int id)
{
    printf("INFO: Read vote %d requested\n", id);
    int vote = !isWriter(id);
    if (vote) {
        cout << "INFO:" << server_ip << "voted for READ " << id << endl;
    }
    else {
        cout << "INFO:" << server_ip << "voted against READ " << id << endl;
        return 0;
    }

    if (readlock[id]->try_lock()) {
        printf("INFO: Read-locking %d\n", id);
    }

    printf("INFO: Read vote sent\n");
    return vote;
}

int QuoServer::clearReadVote(int id)
{
    printf("INFO: Clearing READ %d\n", id);
    if (isReader(id))
    {
        readlock[id]->unlock();
    }

    return 0;
}

