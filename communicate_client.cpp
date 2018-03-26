#include <iostream>
#include "communicate.h"
#include "string.h"
#include "stdlib.h"
#include <unistd.h>
#include <string>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <thread>

using namespace std;


class Client;

class Client {

public:
    CLIENT * clnt;
    std::thread udp_thread;
    int sock = -1;
    char *self_ip;
    int self_port;
    void post(char * content);

    void read();

    void choose(int index);

    void reply(char * content, int index);

    void get_server_list();

    static void listen_for_article(char *own_ip, int port, int sock);

    Client(char *ip, char *serv_ip, int port) {
        strcpy(self_ip, ip);
        self_port = port;
        clnt = clnt_create(serv_ip, COMMUNICATE_PROG, COMMUNICATE_VERSION, "udp");
        if (clnt == NULL) {
            clnt_pcreateerror(serv_ip);
            exit(1);
        }
        std::cout << ".....Completed client creation.....\n";
        //TODO: get server_ip - change self_ip to serv_ip
        struct sockaddr_in client_addr;
        if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
            perror("socket()");
            exit(1);
        }
        int optval = 1;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const void *) &optval, sizeof(int));
        memset(&client_addr, 0, sizeof(client_addr));
        client_addr.sin_family = AF_INET;
        client_addr.sin_addr.s_addr= htonl(INADDR_ANY);
        client_addr.sin_port = htons(self_port);

        if (bind(sock, (struct sockaddr*)&client_addr, sizeof(client_addr)) == -1)     {
            close(sock);
            perror("binding socket");
        }
        udp_thread = std::thread(listen_for_article, serv_ip, port, sock);
        std::cout << ".....before detach.....\n";
        udp_thread.detach();
//        if(udp_thread.joinable()){
//            udp_thread.join();
//        }
    }

    ~Client() {
        if(udp_thread.joinable()){
            udp_thread.join();
        }
        if (clnt)
            clnt_destroy(clnt);

    }
};

void Client::listen_for_article(char *serv_ip, int port, int sock) {
    struct sockaddr_in remote_addr;
    socklen_t slen = sizeof(remote_addr);

    memset((char *) &remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(port);
    if (inet_aton(serv_ip, &remote_addr.sin_addr) == 0) {
        perror("inet_aton failed");
        exit(1);
    }
    free(serv_ip);

    char article[MAXPOOLLENGTH];
    // Clear the buffer by filling null, it might have previously received data
    memset(article, '\0', MAXPOOLLENGTH);

    while(1) {
        // Try to receive some data; This is a blocking call
        if (recvfrom(sock, article, MAXPOOLLENGTH, 0, (struct sockaddr *) &remote_addr, &slen) == -1) {
            perror("recvfrom()");
            exit(1);
        }
        std::cout << "..... received....... \n" << article << "\n.....\n";
    }
}

void Client::post(char * content) {
    auto output = post_1(content, self_ip, self_port, clnt);
    if (*output == 0) {
        clnt_perror(clnt, "Cannot post");
    } else if (*output == -1){
        std::cout << "\nCould not post at index" << std::endl;
    } else {
        std::cout << "\nPosted article successfully" << std::endl;
    }
}

void Client::read() {
    auto output = read_1(self_ip, self_port, clnt);
    if (output == NULL) {
        clnt_perror(clnt, "Cannot read");
    } else {
        std::cout << "\nRead from server\n" << *output << std::endl;
    }
}

void Client::choose(int index) {
    auto output = choose_1(index, self_ip, self_port, clnt);
    if (output->index == 0) {
        std::cout << "\nCannot choose article with id " << index << std::endl;
    } else {
        std::cout << "\nChosen article is:\n" << output->index << " " << output->content << std::endl;
    }
}

void Client::reply(char * content, int index) {
    auto output = reply_1(content, index, self_ip, self_port, clnt);
    if (*output == 0) {
        clnt_perror(clnt, "Cannot reply");
    } else if (*output == -1){
        cout << "\nArticle with index " << index << " does not exist. Please retry.\n" << endl;
    } else {
        std::cout << "\nReplied to article " << index << "successfully with " << content << std::endl;
    }
}

void Client::get_server_list() {
    auto output = get_server_list_1(clnt);
    if (output == (server_list *) NULL) {
        clnt_perror (clnt, "call failed");
    } else {
        std::cout << "\nserver_list is :" << endl;
        for (int i = 0; i < output->server_list_len; i++) {
            std::cout << (output->server_list_val + i)->ip << ":" << (output->server_list_val + i)->port << endl;
        }
        cout << endl;
    }
}

int main(int argc, char *argv[]) {

    if (argc < 4) {
        std::cout << "Usage: ./clientside client_ip server_ip client_port \n";
        exit(1);
    }
    char *client_ip = (char *) argv[1];
    char *serv_ip = (char *) argv[2];
    int self_port = stoi(argv[3]);
    char func[1];
    int func_number;
    //char article_string[MAX_ARTICLE_LENGTH];

    Client conn(client_ip, serv_ip, self_port);

    while (1) {
        std::cout << "Please enter what function you want to perform [1-5]:\n"
                  << "Function description\n1 Post\n2 Read\n3 Choose\n4 Reply\n5 Get Server List\n";
        std::cin >> func;
        try {
            func_number = stoi(func);
        }
        catch(std::exception& e) {
            cout << "ERROR:  Please limit operation values from 1-5 " << endl;
            continue;
        }
        char *article = new char[MAXSTRING];
        int articleId = 0;
        string articleIdStr;
        bool articleIdCorrect =false;
        switch (func_number) {
            case 1:
                std::cout << "Please enter the article content:\n";
                std::cin.get();
                std::cin.getline(article, MAXSTRING);
                conn.post(article);
                break;
            case 2:
                conn.read();
                break;
            case 3:
                std::cout << "Please enter what articles you want to choose:\n";
                articleIdCorrect = false;
                while (!articleIdCorrect) {
                    std::cin >> articleIdStr;
                    try {
                        articleId = stoi(articleIdStr);
                        articleIdCorrect = true;
                    } catch (std::exception& e) {
                        std::cout << "ERROR:  Please input integer for article id" << std::endl;
                    }
                }
                conn.choose(articleId);
                break;
            case 4:
                std::cout << "Please enter the article id you want to reply:\n";
                articleIdCorrect = false;
                while (!articleIdCorrect) {
                    std::cin >> articleIdStr;
                    try {
                        articleId = stoi(articleIdStr);
                        articleIdCorrect = true;
                    } catch (std::exception& e) {
                        std::cout << "ERROR:  Please input integer for article id" << std::endl;
                    }
                }
                std::cout << "Please enter the reply article:\n";
                std::cin.get();
                std::cin.getline(article, MAXSTRING);
                conn.reply(article, articleId);
                break;
            case 5:
                conn.get_server_list();
                break;
            default:
                std::cout << "Wrong format specified. Please retry \n";
                break;
        }
    }
}
