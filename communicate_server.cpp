#include "communicate.h"
#include "peer.h"

//PeerClient *now;

////////////////////////////coordinator/////////////////////////////////////
int *
post_1_svc(char *content, struct svc_req *rqstp) {
    static int result = 0;
    auto res = now->post(content);
    result = res;
    if (result == 0) {
        cout << "Post fails. " << endl;
    } else {
        cout << "Post an article:" << endl;
        cout << result << " " << content << endl;
    }
    return &result;
}

char **
read_1_svc(struct svc_req *rqstp) {
    static char *result = new char[MAXPOOLLENGTH];
    string resultStr = now->read();
    strcpy(result, resultStr.c_str());
    cout << "Read from server:" << endl;
    cout << result << endl;
    return &result;
}

ArticleContent *
choose_1_svc(int index, struct svc_req *rqstp) {
    static ArticleContent result;
    result = now->choose(index);
    return &result;
}

int *
reply_1_svc(char *content, int index, struct svc_req *rqstp) {
    static int result = -1;
    result = now->reply(content, index);
    if (result == -1) {
        cout << "Can't reply to article with id " << index << "." << endl;
    } else {
        cout << "Reply article " << index << " with:";
        cout << result << " " << content << endl;
    }
    return &result;
}


int *
send_flag_1_svc(int arg1,  struct svc_req *rqstp)
{
    static int  result;

    /*
     * insert server code here
     */

    return &result;
}

server_list *
get_server_list_1_svc(struct svc_req *rqstp) {
    static server_list result;
    result = now->get_server_list();
    return &result;
}

int *
join_server_1_svc(IP arg1, int arg2, struct svc_req *rqstp) {
    static int result = -1;

    string ips(arg1, strlen(arg1));
    cout << "join a server " << ips << " " << arg2 << " into coordinator" << endl;

    result = now->joinServer(ips, arg2);
    if (result == -1) {
        cout << "Can't join "<< endl;
    }
    return &result;
}
////////////////////////////coordinator//////////////////////////////////
