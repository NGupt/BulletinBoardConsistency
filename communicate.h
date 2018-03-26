/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#ifndef _COMMUNICATE_H_RPCGEN
#define _COMMUNICATE_H_RPCGEN

#include <rpc/rpc.h>


#ifdef __cplusplus
extern "C" {
#endif

#define MAXIP 16
#define MAXSERVERS 10
#define MAXARTICLES 100
#define MAXSTRING 105
#define MAXPOOLLENGTH 10505

typedef char *str;

typedef char *IP;

struct ArticleContent {
	int index;
	str content;
};
typedef struct ArticleContent ArticleContent;

struct node {
	IP ip;
	int port;
};
typedef struct node node;

typedef struct {
	u_int server_list_len;
	node *server_list_val;
} server_list;

struct ArticleStruct {
	int index;
	str content;
	int depth;
};
typedef struct ArticleStruct ArticleStruct;

typedef struct {
	u_int array_article_len;
	ArticleStruct *array_article_val;
} array_article;

struct ArticlePoolStruct {
	array_article artciles;
	int count;
	int update_count;
};
typedef struct ArticlePoolStruct ArticlePoolStruct;

struct post_1_argument {
	char *arg1;
	char *arg2;
	int arg3;
};
typedef struct post_1_argument post_1_argument;

struct read_1_argument {
	char *arg1;
	int arg2;
};
typedef struct read_1_argument read_1_argument;

struct choose_1_argument {
	int arg1;
	char *arg2;
	int arg3;
};
typedef struct choose_1_argument choose_1_argument;

struct reply_1_argument {
	char *arg1;
	int arg2;
	char *arg3;
	int arg4;
};
typedef struct reply_1_argument reply_1_argument;

struct join_server_1_argument {
	IP arg1;
	int arg2;
};
typedef struct join_server_1_argument join_server_1_argument;

#define COMMUNICATE_PROG 0x20000001
#define COMMUNICATE_VERSION 1

#if defined(__STDC__) || defined(__cplusplus)
#define POST 1
extern  int * post_1(char *, char *, int , CLIENT *);
extern  int * post_1_svc(char *, char *, int , struct svc_req *);
#define READ 2
extern  char ** read_1(char *, int , CLIENT *);
extern  char ** read_1_svc(char *, int , struct svc_req *);
#define CHOOSE 3
extern  ArticleContent * choose_1(int , char *, int , CLIENT *);
extern  ArticleContent * choose_1_svc(int , char *, int , struct svc_req *);
#define REPLY 4
extern  int * reply_1(char *, int , char *, int , CLIENT *);
extern  int * reply_1_svc(char *, int , char *, int , struct svc_req *);
#define GET_SERVER_LIST 5
extern  server_list * get_server_list_1(CLIENT *);
extern  server_list * get_server_list_1_svc(struct svc_req *);
#define JOIN_SERVER 6
extern  int * join_server_1(IP , int , CLIENT *);
extern  int * join_server_1_svc(IP , int , struct svc_req *);
extern int communicate_prog_1_freeresult (SVCXPRT *, xdrproc_t, caddr_t);

#else /* K&R C */
#define POST 1
extern  int * post_1();
extern  int * post_1_svc();
#define READ 2
extern  char ** read_1();
extern  char ** read_1_svc();
#define CHOOSE 3
extern  ArticleContent * choose_1();
extern  ArticleContent * choose_1_svc();
#define REPLY 4
extern  int * reply_1();
extern  int * reply_1_svc();
#define GET_SERVER_LIST 5
extern  server_list * get_server_list_1();
extern  server_list * get_server_list_1_svc();
#define JOIN_SERVER 6
extern  int * join_server_1();
extern  int * join_server_1_svc();
extern int communicate_prog_1_freeresult ();
#endif /* K&R C */

/* the xdr functions */

#if defined(__STDC__) || defined(__cplusplus)
extern  bool_t xdr_str (XDR *, str*);
extern  bool_t xdr_IP (XDR *, IP*);
extern  bool_t xdr_ArticleContent (XDR *, ArticleContent*);
extern  bool_t xdr_node (XDR *, node*);
extern  bool_t xdr_server_list (XDR *, server_list*);
extern  bool_t xdr_ArticleStruct (XDR *, ArticleStruct*);
extern  bool_t xdr_array_article (XDR *, array_article*);
extern  bool_t xdr_ArticlePoolStruct (XDR *, ArticlePoolStruct*);
extern  bool_t xdr_post_1_argument (XDR *, post_1_argument*);
extern  bool_t xdr_read_1_argument (XDR *, read_1_argument*);
extern  bool_t xdr_choose_1_argument (XDR *, choose_1_argument*);
extern  bool_t xdr_reply_1_argument (XDR *, reply_1_argument*);
extern  bool_t xdr_join_server_1_argument (XDR *, join_server_1_argument*);

#else /* K&R C */
extern bool_t xdr_str ();
extern bool_t xdr_IP ();
extern bool_t xdr_ArticleContent ();
extern bool_t xdr_node ();
extern bool_t xdr_server_list ();
extern bool_t xdr_ArticleStruct ();
extern bool_t xdr_array_article ();
extern bool_t xdr_ArticlePoolStruct ();
extern bool_t xdr_post_1_argument ();
extern bool_t xdr_read_1_argument ();
extern bool_t xdr_choose_1_argument ();
extern bool_t xdr_reply_1_argument ();
extern bool_t xdr_join_server_1_argument ();

#endif /* K&R C */

#ifdef __cplusplus
}
#endif

#endif /* !_COMMUNICATE_H_RPCGEN */
