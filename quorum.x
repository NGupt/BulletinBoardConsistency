program QUORUM_PROG {
    version QUORUM_VERSION {
        int POST(string) = 1;
        string READ() = 2;
        ArticleContent CHOOSE(int) = 3;
        int REPLY(string, int) = 4;
        int SEND_FLAG(int) = 5;
        server_list GET_SERVER_LIST() = 9;
        int JOIN_SERVER(IP, int) = 10;


        int QUEUE_UP(int art_id, string content);
        int WRITE_UP(ArticlePool art);
        int fetchWriteVote(int id, int mod_time);
        int ReadVote(int id, int mod_time);
        int fetchReadVote(int id, int mod_time);
    } = 1;
} = 0x20000001;
