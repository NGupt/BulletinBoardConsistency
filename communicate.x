const MAXSTRING = 50;
typedef string str<MAXSTRING>;

struct ArticleContent {
    int index;
    str content;
    /*
    int* nextArticles;
    Article** nextArticles;
    */
    /*
    Article(int it, string article): index(id), content(article) {}
    */
};

program COMMUNICATE_PROG {
    version COMMUNICATE_VERSION {
        int POST(string) = 1;
        string READ() = 2; 
        ArticleContent CHOOSE(int) = 3;
        int REPLY(string, int) = 4;
    } = 1;
} = 0x20000001;
