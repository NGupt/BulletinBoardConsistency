#include "communicate.h"
#include "peer.h"

PeerClient *now;

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

void PeerClient::outputServerList(PeerClient *p) {
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
    if(std::find(serverList.begin(), serverList.end(), make_pair(ip, port)) != serverList.end()) {
    } else {
        serverList.push_back(make_pair(ip, port));
    }
    outputServerList(now);
    return 0;
}

int PeerClient::updateAllServers(PeerClient *p, ArticlePool articlePool, int reply_index)
{
    int result = -1;
    //send article to other servers;
    std::map<int,Article*>::iterator rit;
    Article *temp;
    for (rit=articlePool.articleMap.begin(); rit!=articlePool.articleMap.end(); ++rit)
        temp = rit->second;
    //cout << (temp->index) << temp->content << "before sending"<< endl;
    // articlePool.PrintArticlePoolStruct(articlePool.getArticle());

    string send_content("");
    send_content = std::to_string(temp->index);
    send_content.append(";");
    if(reply_index != 0){
      send_content.append(std::to_string(reply_index));
    }
    send_content.append(";");
    send_content.append(temp->content);
    //cout << "content all: " << send_content << endl;
    for (int i=0; i<(p->serverList.size()); i++) {
        if(p->isCoordinator(p->serverList[i].first.c_str())) {
            result = 0;
            continue;
        }
        printf("Updating %s:%d\n",p->serverList[i].first.c_str(), p->serverList[i].second);
        result = p->udp_send_confirm(p->serverList[i].first.c_str(), p->serverList[i].second, send_content.c_str(), MAXPOOLLENGTH);
        if (result == -1) {
            cout << "Update to backup failed" << endl;
            return -1;
        }
        // send confirmation to client
    }
    //while(p->num_confirmations < (p->serverList.size() - 1));
    return result;
}

int PeerClient::udp_send_confirm(const char *ip, int port, const char *buf, const int buf_size) {
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
     struct timeval tv;
      tv.tv_sec = 3;
    tv.tv_usec = 0;
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
        perror("Error");
    }

    /* Begin listening for confirmation of insert */
    printf("INFO: listening for confirmation\n");
    char recbuf[MAXPOOLLENGTH];
    if ((recvfrom(fd, recbuf, MAXPOOLLENGTH, 0, res->ai_addr, &res->ai_addrlen)) < 0) {

      perror("timed out, removing server from my serverList");
             serverList.erase(std::remove(serverList.begin(), serverList.end(), pair<string,int>(ip,port)), serverList.end());
        freeaddrinfo(res);
        close(fd);
        return 0;
    }


    if (strcmp(recbuf,buf) == 0) {
        cout << "Received confirmation from: " << ip << endl;
        return 0;
    }
    freeaddrinfo(res);
    close(fd);
    return -1;
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

        //cout << "resending recvd thing" << article_update << endl;

        if ((sendto(s->insert_listen_fd, article_update, MAXPOOLLENGTH, 0, (struct sockaddr *) &remote_addr, slen)) == -1) {
            close(s->insert_listen_fd);
            s->insert_listen_fd = -1;
            perror("Error: acknowledging the received string");
            throw;
        }
    }
}

void PeerClient::decode_articles(char *temp_articles){
    int line_pos = 0;
    int content_pos = 0;
    int parent_id = 0;
    int num_lines = 0;
    int index = 1;
    bool reply_flag = false;
    std::string line_delimiter = "\n";
    std::string content_delimiter = " ";
    std::string reply_delimiter = "\t";
    std::string content = "";
    std::string line = "";

    string articles(temp_articles, strlen(temp_articles));

    while(articles != "") {
        line_pos = articles.find(line_delimiter);

        line = articles.substr(0, line_pos);   //returning line
        articles = articles.substr(line_pos + 1);  //returning rest of levels

        reply_flag = false;
        content_pos = line.find(reply_delimiter);
        while((content_pos = line.find(reply_delimiter)) == 0) {
            line = line.substr(content_pos + 1);  //returning reply line without tab
            reply_flag = true;
        }
        content_pos = line.find(content_delimiter);
        content = line.substr(content_pos + 1);
        index = atoi(line.substr(0, content_pos).c_str());
//        cout << index << " " << content << " " << parent_id << endl;
        if(reply_flag)
            articlePool.writeArticle(content, parent_id, index);
        else
            articlePool.writeArticle(content, 0, index );

        parent_id = atoi(line.substr(0,content_pos).c_str());
        num_lines++;
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
        num_confirmations = 0;
    } else {
        cout << "is not coordinator, joining server to " << c_ip << endl;
    }
    join_server(o_ip, port);
    get_server_list();


    sockaddr_in si_me;
    //printf("Begin listening to requests from the other servers...\n");
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

PeerClient::~PeerClient() {
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
