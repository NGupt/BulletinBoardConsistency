#include <iostream>
#include "communicate.h"
#include "string.h"
#include "stdlib.h"
#include <string>
using namespace std;


class Client;

class Client {

public:
    CLIENT * clnt;
    char *self_ip;
    int self_port;
    void post(char * content);

    void read();

    void choose(int index);

    void reply(char * content, int index);

    void get_server_list();

    Client(char *ip, char *serv_ip) {
        clnt = clnt_create(serv_ip, COMMUNICATE_PROG, COMMUNICATE_VERSION, "udp");
        if (clnt == NULL) {
            clnt_pcreateerror(serv_ip);
            exit(1);
        }
        std::cout << ".....Completed client creation.....\n";
    }

    ~Client() {
        if (clnt)
            clnt_destroy(clnt);
    }
};

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

    Client conn(client_ip, serv_ip);
    conn.self_ip = client_ip;
    conn.self_port = self_port;

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
