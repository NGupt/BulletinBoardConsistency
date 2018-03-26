const MAXIP = 16;
const MAXSERVERS = 10;
const MAXARTICLES = 100;
const MAXSTRING = 105;
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
        int POST(string, string, int) = 1;
        string READ(string, int) = 2;
        ArticleContent CHOOSE(int, string, int) = 3;
        int REPLY(string, int, string, int) = 4;
        server_list GET_SERVER_LIST() = 5;
        int JOIN_SERVER(IP, int) = 6;
    } = 1;
} = 0x20000001;
