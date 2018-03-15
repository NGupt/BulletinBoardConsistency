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
    void post(char * content);

    void read();

    void choose(int index);

    void reply(char * content, int index);

    void get_server_list();

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

void Client::post(char * content) {
    auto output = post_1(content, clnt);
    if (*output == 0) {
        std::cout << "Post the article " << content << " fails" << std::endl;
        //clnt_perror(clnt, "Cannot post");
    } else {
        std::cout << "Post the article " << *output << " " << content << std::endl;
    }
}

void Client::read() {
    auto output = read_1(clnt);
    //if (output == NULL) {
    //    clnt_perror(clnt, "Cannot read");
    //} else {
    std::cout << "Read from server\n" << *output << std::endl;
    //}
}

void Client::choose(int index) {
    auto output = choose_1(index, clnt);
    if (output->index == 0) {
        std::cout << "Cannot choose article with id " << index << std::endl;
    } else {
        std::cout << "Choose the article:\n" << output->index << " " << output->content << std::endl;
    }
}

void Client::reply(char * content, int index) {
    auto output = reply_1(content, index, clnt);
    if (*output == 0) {
        std::cout << "Can't reply to article " << index << " with " << content << std::endl;
        //clnt_perror(clnt, "Cannot reply to ");
    } else {
        std::cout << "Reply the article " << index << " with the new article " << *output << " " << content << std::endl;
    }
}

void Client::get_server_list() {
    auto output = get_server_list_1(clnt);
    //TODO: fix it
    if (output == (server_list *) NULL) {
      clnt_perror (clnt, "call failed");
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
        char *article = new char[MAXSTRING];
        string articleStr;
        int articleId = 0;
        string articleIdStr;
        bool articleIdCorrect =false;
        switch (func_number) {
            case 1:
                std::cout << "Please enter the article content:\n";
                strcpy(article, "");
                while (1) {
                    std::getline(cin, articleStr);
                    if (articleStr.length() > 1) {
                        break;
                    }
                }
                strcpy(article, articleStr.substr(0, articleStr.length()).c_str());
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
                while (1) {
                    std::getline(cin, articleStr);
                    if (articleStr.length() > 1) {
                        break;
                    }
                }
                strcpy(article, articleStr.substr(0, articleStr.length()).c_str());
                conn.reply(article, articleId);
                break;
            default:
                std::cout << "Wrong format specified. Please retry \n";
                break;
        }
    }
}
