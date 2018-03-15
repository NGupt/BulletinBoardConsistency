const MAXIP = 16;
const MAXSERVERS = 10;
const MAXARTICLES = 100;
const MAXSTRING = 101;
typedef string str<MAXSTRING>;
typedef string IP<MAXIP>;

struct ArticleContent {
    int index;
    str content;
};

struct node {
    IP ip;
    int port;
};
typedef node server_list<MAXSERVERS>;

struct ArticleStruct {
    int index;
    str content;
    int depth;
};
typedef ArticleStruct array_article<MAXARTICLES>;

struct ArticlePoolStruct {
    array_article artciles;
    int count;
    int update_count;
};

program COMMUNICATE_PROG {
    version COMMUNICATE_VERSION {
        int POST(string) = 1;
        string READ() = 2;
        ArticleContent CHOOSE(int) = 3;
        int REPLY(string, int) = 4;
        int SEND_FLAG(int) = 5;
        ArticlePoolStruct GET_ARTICLE() = 6;
        int SEND_ARTICLE(ArticlePoolStruct) = 7;
        int SEND_SERVER_LIST(server_list) = 8;
        server_list GET_SERVER_LIST() = 9;
        int JOIN_SERVER(IP, int) = 10;
    } = 1;
} = 0x20000001;
