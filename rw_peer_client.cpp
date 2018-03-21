#include "communicate.h"
#include "peer.h"

//PeerClient *now;

int PeerClient::post(char *content) {
    int output = -1;
    std::string myString(content, strlen(content));
    output = articlePool.post(myString);
    if (isCoordinator(server_ip)) {
        update_thread = thread(updateAllServers, this, articlePool, 0);
        update_thread.detach();
        return output;
    } else {
        update_thread = thread(post_1, content, pclnt);
        update_thread.detach();
        if (output == 0) {
            clnt_perror(pclnt, "Cannot post");
        } else {
            std::cout << "Post the article " << output << " " << content << std::endl;
        }
    }
    return output;
}


int PeerClient::reply(char *content, int index) {
    int output = -1;
    std::string myString(content, strlen(content));
    output = articlePool.reply(myString, index);
    if (isCoordinator(server_ip)) {
        update_thread = thread(updateAllServers, this, articlePool, index);
        update_thread.detach();
        return output;
    } else {
        update_thread = thread(reply_1, content, index, pclnt);
        update_thread.detach();
        if (output == 0) {
            clnt_perror(pclnt, "Cannot reply");
        }
    }
    return output;
}

string PeerClient::read() {
    return articlePool.read();
}

//TODO: FIX
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
////////////////////////////peer client////////////////////////////////////////
