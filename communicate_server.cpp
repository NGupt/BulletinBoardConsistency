/*
 * This is sample code generated by rpcgen.
 * These are only templates and you can use them
 * as a guideline for developing your own functions.
 */

#include "communicate.h"
#include "peer.h"
#include <cstring>
#include <algorithm> //for std::find


using namespace std;
using std::to_string;
using std::thread;

#ifndef SIG_PF
#define SIG_PF void(*)(int)
#endif

PeerClient *now;

int sock;
thread insert_listen_thread;
int insert_listen_fd;

bool PeerClient::isCoordinator(string ip) {
    return ip == coordinator_ip;
}

server_list PeerClient::buildServerList() {
    server_list res;
    res.server_list_val = new node[serverList.size()];
    res.server_list_len = serverList.size();
    int pos = 0;
    for (int i = 0; i < serverList.size(); i++) {
        node *p = res.server_list_val + pos;
        p->ip = new char[serverList[i].first.length() + 1];
        strcpy(p->ip, serverList[i].first.c_str());
        p->port = serverList[i].second;
        pos++;
    }
    return res;
}

void outputServerList(PeerClient *p) {
    //output server list;
    cout << "outputing server list:" << endl;
    server_list servers = p->buildServerList();
    for (int i = 0; i < servers.server_list_len; i++) {
        cout << (servers.server_list_val + i)->ip << " " << (servers.server_list_val + i)->port << endl;
    }
    cout << endl;
}

//join a serve to coordinator
int PeerClient::joinServer(string ip, int port) {
    if(std::find(this->serverList.begin(), this->serverList.end(), make_pair(ip, port)) != this->serverList.end()) {
    } else {
        this->serverList.push_back(make_pair(ip, port));
    }
    outputServerList(this);
    return 0;
}

int PeerClient::updateServer(int art_id, string content, char *backup_IP, int backup_port) {

    int result = -1;
//    char buf[MAXPOOLLENGTH];
//    snprintf(buf, MAXPOOLLENGTH, "Insert;%d;%s", art_id, content.c_str());

    result = udp_send_confirm(backup_IP, backup_port, content.c_str(), MAXPOOLLENGTH);
    if (result < 0) {
        printf("ERROR: Insert: Sending update\n");
        return -1;
    }

    return result;
}


int PeerClient::udp_send_confirm(const char *ip, int port, const char *buf, const int buf_size) {
    int fd;
    struct addrinfo hints;
    struct addrinfo* res;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = 0;
    hints.ai_flags = AI_ADDRCONFIG;

    //cout << "buf: " << buf <<endl;
    if (getaddrinfo(ip, std::to_string(static_cast<long long>(port)).c_str(), &hints, &res) != 0)
    {
        perror("cant get addressinfo");
        return -1;
    }

    fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd == -1)
    {
        perror("cant create socket");
        freeaddrinfo(res);
        return -1;
    }

    int s = sendto(fd, buf, buf_size, 0, res->ai_addr, res->ai_addrlen);
    if (s == -1)
    {
        perror("cant send ");
        freeaddrinfo(res);
        close(fd);
        return -1;
    }

//    /* Begin listening for confirmation of insert */
//    printf("INFO: Insert: listening for confirmation\n");
//    char recbuf[MAXPOOLLENGTH];
//    if ((recvfrom(fd, recbuf, MAXPOOLLENGTH, 0, res->ai_addr, &res->ai_addrlen)) < 0) {
//        freeaddrinfo(res);
//        close(fd);
//        return -1;
//    }
//
//    //print details of the client/peer and the data received
//    printf("Received Data: %s\n" , buf);

    freeaddrinfo(res);
    close(fd);

    return 0;
}

int PeerClient::insert(PeerClient *p, int art_id, string content)
{
    int result;
    for (int i=0; i<(p->serverList.size()); i++) {
        if(p->isCoordinator(p->serverList[i].first.c_str()))
            continue;
        printf("Updating %s:%d\n",p->serverList[i].first.c_str(), p->serverList[i].second);
        result = p->updateServer(art_id, content, (char*)(p->serverList[i].first.c_str()), p->serverList[i].second);
        if (result == -1) {
            printf("Update to backup failed");
            return -1;
        }

        //printf("Insert Complete.\n");
        // send confirmation to client
        return result;
    }
}

void PeerClient::listen_from(PeerClient *s,string r_ip, int port){
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

    char article_update[MAXPOOLLENGTH];
    // Clear the buffer by filling null, it might have previously received data
    memset(article_update, '\0', MAXPOOLLENGTH);

    int pos = 0;
    int index = 0;
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

        string remaining_content = article.substr(pos+1);
        pos = remaining_content.find(delimiter);
        index = 0;
        if((remaining_content.substr(0, pos)) != "") {
            index = atoi((remaining_content.substr(0, pos)).c_str());
        }
        string content = remaining_content.substr(pos+1);
        //cout << "content " << content << " index " << index << endl;
        //if listen_for happened bcz of post call
        //s->articlePool.count += s->articlePool.getCount() + 1;
        s->articlePool.storeArticle(content,index); //with index = 0 , it is same as post

//        if ((sendto(s->insert_listen_fd, article_update, MAXPOOLLENGTH, 0, (struct sockaddr *) &remote_addr, slen)) == -1)
//        {
//            close(s->insert_listen_fd);
//            s->insert_listen_fd = -1;
//            cout << "Error: acknowledging the received string" << endl;
//            throw;
//        }
    }
}


PeerClient::PeerClient(string ip, int port, string coordinator_ip, int coordinator_port) {
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
    if (pclnt == NULL) {
        clnt_pcreateerror(c_ip);
        exit(1);
    }
    if (isCoordinator(o_ip)) {
        cout << "is coordinator" << endl;
    } else {
        cout << "is not coordinator, joining server to " << c_ip << endl;
    }
    join_server(o_ip, port);
    get_server_list();


    sockaddr_in si_me;
    printf("Begin listening to requests from the other servers...\n");
    insert_listen_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (insert_listen_fd == -1) {
        printf("Error: creating socket\n");
        throw;
    }
    int optval = 1;
    setsockopt(insert_listen_fd, SOL_SOCKET, SO_REUSEADDR, (const void *) &optval, sizeof(int));
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

    std::cout << ".....Completed Server creation.....\n";
}

PeerClient::~PeerClient() {
    if (pclnt) {
        clnt_destroy(pclnt);
    }
    if(insert_listen_thread.joinable()){
        insert_listen_thread.join();
    }
    //close(sock);
}

int PeerClient::post(char *content) {
    int output;
    if (isCoordinator(server_ip)) {
        std::string myString(content, strlen(content));
        //post to articlePool
        output = articlePool.post(myString);
        //send article to other servers;
        std::map<int,Article*>::iterator rit;
        Article *temp;
        for (rit=articlePool.articleMap.begin(); rit!=articlePool.articleMap.end(); ++rit)
            temp = rit->second;
        //cout << (temp->index) << temp->content << "before sending"<< endl;
       // articlePool.PrintArticlePoolStruct(articlePool.getArticle());

        string send_content;
        stringstream ss;
        ss << temp->index;  //increment if coming from server
        send_content = ss.str();
        send_content.append(";");
        send_content.append(";");
        send_content.append(temp->content);
        //cout << "content: " << send_content << endl;

        insert(this, temp->index, send_content);
    } else {
        output = *post_1(content, pclnt);
        if (output == 0) {
            clnt_perror(pclnt, "Cannot post");
        } else {
            std::cout << "Post the article " << output << " " << content << std::endl;
        }
    }
    return output;
}

string PeerClient::read() {
    return articlePool.read();
//
//    if (isCoordinator(server_ip)) {
//        return articlePool.read();
//    } else {
//        auto output = read_1(pclnt);
//        if (output == NULL) {
//            clnt_perror(pclnt, "Cannot read");
//        } else {
//            std::cout << "Read from server\n" << *output << std::endl;
//        }
//        return *output;
//    }
}

ArticleContent PeerClient::choose(int index) {
    //if (isCoordinator(server_ip)) {
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
//    } else {
//        auto output = choose_1(index, pclnt);
//        if (output->index == 0) {
//            std::cout << "Cannot choose article with id " << index << std::endl;
//        } else {
//            std::cout << "Choose the article:\n" << output->index << " " << output->content << std::endl;
//        }
//        return *output;
//    }
}

int PeerClient::reply(char *content, int index) {
    if (isCoordinator(server_ip)) {
        std::string myString(content, strlen(content));
        int result = articlePool.reply(myString, index);
        //send article to other servers;
        std::map<int,Article*>::iterator rit;
        Article *temp;
        for (rit=articlePool.articleMap.begin(); rit!=articlePool.articleMap.end(); ++rit)
            temp = rit->second;
        //cout << (temp->index) << temp->content << "before sending"<< endl;
       // articlePool.PrintArticlePoolStruct(articlePool.getArticle());

        string send_content;
        stringstream ss;
        ss << (temp->index);  //increment if coming from server
        send_content = ss.str();
        send_content.append(";");
        stringstream replyindex;
        replyindex << index;
        send_content.append(replyindex.str());
        send_content.append(";");
        send_content.append(temp->content);
        //cout << "content: " << send_content << endl;

        insert(this, (temp->index), send_content);
        return result;
    } else {
        auto output = reply_1(content, index, pclnt);
        if (*output == 0) {
            clnt_perror(pclnt, "Cannot reply to ");
        } else {
            std::cout << "Reply the article " << index << " with the new article " << *output << " " << content
                      << std::endl;
        }
        return *output;
    }
}

//get the current articlePool
ArticlePoolStruct PeerClient::getLocalArticle() {
    return articlePool.getArticle();
}

server_list PeerClient::get_server_list() {
    server_list output;
    if (isCoordinator(server_ip)) {
        outputServerList(now);
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
////////////////////////////peer client////////////////////////////////////////





////////////////////////////peer server/////////////////////////////////////
int *
post_1_svc(char *content, struct svc_req *rqstp) {
    static int result = 0;
    auto res = now->post(content);
    result = res;
    if (result == 0) {
        cout << "Post fails. " << endl;
    } else {
        cout << "Post an article:" << endl;
        cout << result << " " << content << endl;
    }
    return &result;
}

char **
read_1_svc(struct svc_req *rqstp) {
    static char *result = new char[MAXPOOLLENGTH];
    string resultStr = now->read();
    strcpy(result, resultStr.c_str());
    cout << "Read from server:" << endl;
    cout << result << endl;
    return &result;
}

ArticleContent *
choose_1_svc(int index, struct svc_req *rqstp) {
    static ArticleContent result;
    result = now->choose(index);
//     if (result == (ArticleContent *)NULL) {
//         strcpy(result.content, "");
//         result.index = 0;
//         cout << "The article with id " << index << " doesn't exist in the server." << endl;
//     } else {
//         strcpy(result.content, resultArticle->content.c_str());
//         result.index = resultArticle->index;
//         cout << "The client choose the article: " << endl;
//         cout << result.index << " " << result.content << endl;
//     }
    return &result;
}

int *
reply_1_svc(char *content, int index, struct svc_req *rqstp) {
    static int result = -1;
    result = now->reply(content, index);
    if (result == -1) {
        cout << "Can't reply to article with id " << index << "." << endl;
    } else {
        cout << "Reply article " << index << " with:";
        cout << result << " " << content << endl;
    }
    return &result;
}

server_list *
get_server_list_1_svc(struct svc_req *rqstp) {
    static server_list result;
    result = now->buildServerList();
    return &result;
}

int *
join_server_1_svc(IP arg1, int arg2, struct svc_req *rqstp) {
    static int result = -1;

    string ips(arg1, strlen(arg1));
    cout << "join a server " << ips << " " << arg2 << " into coordinator" << endl;

    result = now->joinServer(ips, arg2);
    if (result == -1) {
        cout << "Can't join "<< endl;
    }
    return &result;
}
////////////////////////////peer server//////////////////////////////////
