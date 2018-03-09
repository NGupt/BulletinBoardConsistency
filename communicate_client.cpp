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
    int post(char * content);

    char * read();

    ArticleContent choose(int index);

    int reply(char * content, int index);

    Client(char *ip, int port, char *serv_ip) {
        clnt = clnt_create(serv_ip, COMMUNICATE_PROG, COMMUNICATE_VERSION, "udp");
        if (clnt == NULL) {
            clnt_pcreateerror(serv_ip);
            exit(1);
        }
        std::cout << ".....Completed client creation.....\n";
    }

    ~Client() {
      if (clnt){
	clnt_destroy(clnt); }
    }

    /*Function implementation to receive udp communication*/
};

int Client::post(char * content) {
    auto output = post_1(content, clnt);
    if (output == NULL) {
        clnt_perror(clnt, "Cannot post");
    } else {
        std::cout << "Post with index " << *output << std::endl;
    }
}

char * Client::read() {
    auto output = read_1(clnt);
    if (output == NULL) {
        clnt_perror(clnt, "Cannot read");
    } else {
        std::cout << *output << std::endl;
    }
}

ArticleContent Client::choose(int index) {
    auto output = choose_1(index, clnt);
    if (output == NULL) {
        clnt_perror(clnt, "Cannot choose");
    } else {
        std::cout << output->index << " " << output->content << std::endl;
    }
}

int Client::reply(char * content, int index) {
    auto output = reply_1(content, index, clnt);
    if (output == NULL) {
        clnt_perror(clnt, "Cannot choose reply");
    } else {
        std::cout << "Choose index " << *output << std::endl;
    }
}



int main(int argc, char *argv[]) {

    if (argc < 4) {
        std::cout << "Usage: ./clientside client_ip port server_ip\n";
        exit(1);
    }
    char *client_ip = (char *) argv[1];
    int client_port = stoi(argv[2]);
    char *serv_ip = (char *) argv[3];
    char func[1];
    int func_number;
    //char article_string[MAX_ARTICLE_LENGTH];

    Client conn(client_ip, client_port, serv_ip);

    while (1) {
        std::cout << "Please enter what function you want to perform [1-4]:\n"
                  << "Function description\n1 Post\n2 Read\n3 Choose\n4 Reply\n";
        std::cin >> func;
        try {
            func_number = stoi(func);
        }
        catch(std::exception& e) {
            cout << "ERROR:  Please limit operation values from 1-4 " << endl;
            continue;
        }
        char *article = new char[10];
        char *article2 = new char[10];
        switch (func_number) {
            case 1:
                strcpy(article, "article 1");
                conn.post(article);
                break;
            case 2:
                conn.read();
                break;
            case 3:
                conn.choose(1);
                break;
            case 4:
                strcpy(article2, "article 2");
                conn.reply(article2, 1);
                break;
            default:
                std::cout << "Wrong format specified. Please retry \n";
                break;
        }
    }
}
