/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include "communicate.h"
#include "article.h"
#include "peer.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <rpc/pmap_clnt.h>
#include <string.h>
#include <memory.h>
#include <sys/socket.h>
#include <netinet/in.h>

#ifndef SIG_PF
#define SIG_PF void(*)(int)
#endif
using namespace std;

/* Default timeout can be changed using clnt_control() */
static struct timeval TIMEOUT = { 25, 0 };

////////////////////////////// peer server////////////////////////////
static int *
_post_1 (char * *argp, struct svc_req *rqstp)
{
	return (post_1_svc(*argp, rqstp));
}

static char **
_read_1 (void  *argp, struct svc_req *rqstp)
{
	return (read_1_svc(rqstp));
}

static ArticleContent *
_choose_1 (int  *argp, struct svc_req *rqstp)
{
	return (choose_1_svc(*argp, rqstp));
}

static int *
_reply_1 (reply_1_argument *argp, struct svc_req *rqstp)
{
	return (reply_1_svc(argp->arg1, argp->arg2, rqstp));
}

static int *
_send_flag_1 (int  *argp, struct svc_req *rqstp)
{
	return (send_flag_1_svc(*argp, rqstp));
}

static ArticlePoolStruct *
_get_article_1 (void  *argp, struct svc_req *rqstp)
{
	return (get_article_1_svc(rqstp));
}

static int *
_send_article_1 (ArticlePoolStruct  *argp, struct svc_req *rqstp)
{
	return (send_article_1_svc(*argp, rqstp));
}

static int *
_send_server_list_1 (server_list  *argp, struct svc_req *rqstp)
{
	return (send_server_list_1_svc(*argp, rqstp));
}

static void
communicate_prog_1(struct svc_req *rqstp, register SVCXPRT *transp)
{
	union {
		char *post_1_arg;
		int choose_1_arg;
		reply_1_argument reply_1_arg;
		int send_flag_1_arg;
		ArticlePoolStruct send_article_1_arg;
		server_list send_server_list_1_arg;
	} argument;
	char *result;
	xdrproc_t _xdr_argument, _xdr_result;
	char *(*local)(char *, struct svc_req *);

	switch (rqstp->rq_proc) {
	case NULLPROC:
		(void) svc_sendreply (transp, (xdrproc_t) xdr_void, (char *)NULL);
		return;

	case POST:
		_xdr_argument = (xdrproc_t) xdr_wrapstring;
		_xdr_result = (xdrproc_t) xdr_int;
		local = (char *(*)(char *, struct svc_req *)) _post_1;
		break;

	case READ:
		_xdr_argument = (xdrproc_t) xdr_void;
		_xdr_result = (xdrproc_t) xdr_wrapstring;
		local = (char *(*)(char *, struct svc_req *)) _read_1;
		break;

	case CHOOSE:
		_xdr_argument = (xdrproc_t) xdr_int;
		_xdr_result = (xdrproc_t) xdr_ArticleContent;
		local = (char *(*)(char *, struct svc_req *)) _choose_1;
		break;

	case REPLY:
		_xdr_argument = (xdrproc_t) xdr_reply_1_argument;
		_xdr_result = (xdrproc_t) xdr_int;
		local = (char *(*)(char *, struct svc_req *)) _reply_1;
		break;

	case SEND_FLAG:
		_xdr_argument = (xdrproc_t) xdr_int;
		_xdr_result = (xdrproc_t) xdr_int;
		local = (char *(*)(char *, struct svc_req *)) _send_flag_1;
		break;

	case GET_ARTICLE:
		_xdr_argument = (xdrproc_t) xdr_void;
		_xdr_result = (xdrproc_t) xdr_ArticlePoolStruct;
		local = (char *(*)(char *, struct svc_req *)) _get_article_1;
		break;

	case SEND_ARTICLE:
		_xdr_argument = (xdrproc_t) xdr_ArticlePoolStruct;
		_xdr_result = (xdrproc_t) xdr_int;
		local = (char *(*)(char *, struct svc_req *)) _send_article_1;
		break;

	case SEND_SERVER_LIST:
		_xdr_argument = (xdrproc_t) xdr_server_list;
		_xdr_result = (xdrproc_t) xdr_int;
		local = (char *(*)(char *, struct svc_req *)) _send_server_list_1;
		break;

	default:
		svcerr_noproc (transp);
		return;
	}
	memset ((char *)&argument, 0, sizeof (argument));
	if (!svc_getargs (transp, (xdrproc_t) _xdr_argument, (caddr_t) &argument)) {
		svcerr_decode (transp);
		return;
	}
	result = (*local)((char *)&argument, rqstp);
	if (result != NULL && !svc_sendreply(transp, (xdrproc_t) _xdr_result, result)) {
		svcerr_systemerr (transp);
	}
	if (!svc_freeargs (transp, (xdrproc_t) _xdr_argument, (caddr_t) &argument)) {
		fprintf (stderr, "%s", "unable to free arguments");
		exit (1);
	}
	return;
}

////////////////////////////// peer server////////////////////////////


/////////////////////////////peer client///////////////////////////
int *
send_flag_1(int arg1,  CLIENT *clnt)
{
	static int clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	if (clnt_call (clnt, SEND_FLAG,
		(xdrproc_t) xdr_int, (caddr_t) &arg1,
		(xdrproc_t) xdr_int, (caddr_t) &clnt_res,
		TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&clnt_res);
}

ArticlePoolStruct *
get_article_1(CLIENT *clnt)
{
	static ArticlePoolStruct clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	 if (clnt_call (clnt, GET_ARTICLE, (xdrproc_t) xdr_void, (caddr_t) NULL,
		(xdrproc_t) xdr_ArticlePoolStruct, (caddr_t) &clnt_res,
		TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&clnt_res);
}

int *
send_article_1(ArticlePoolStruct arg1,  CLIENT *clnt)
{
	static int clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	if (clnt_call (clnt, SEND_ARTICLE,
		(xdrproc_t) xdr_ArticlePoolStruct, (caddr_t) &arg1,
		(xdrproc_t) xdr_int, (caddr_t) &clnt_res,
		TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&clnt_res);
}

int *
send_server_list_1(server_list arg1,  CLIENT *clnt)
{
	static int clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	if (clnt_call (clnt, SEND_SERVER_LIST,
		(xdrproc_t) xdr_server_list, (caddr_t) &arg1,
		(xdrproc_t) xdr_int, (caddr_t) &clnt_res,
		TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&clnt_res);
}

int *
post_1(char *arg1,  CLIENT *clnt)
{
	static int clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	if (clnt_call (clnt, POST,
		(xdrproc_t) xdr_wrapstring, (caddr_t) &arg1,
		(xdrproc_t) xdr_int, (caddr_t) &clnt_res,
		TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&clnt_res);
}

char **
read_1(CLIENT *clnt)
{
	static char *clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	 if (clnt_call (clnt, READ, (xdrproc_t) xdr_void, (caddr_t) NULL,
		(xdrproc_t) xdr_wrapstring, (caddr_t) &clnt_res,
		TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&clnt_res);
}

ArticleContent *
choose_1(int arg1,  CLIENT *clnt)
{
	static ArticleContent clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	if (clnt_call (clnt, CHOOSE,
		(xdrproc_t) xdr_int, (caddr_t) &arg1,
		(xdrproc_t) xdr_ArticleContent, (caddr_t) &clnt_res,
		TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&clnt_res);
}

int *
reply_1(char *arg1, int arg2,  CLIENT *clnt)
{
	reply_1_argument arg;
	static int clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	arg.arg1 = arg1;
	arg.arg2 = arg2;
	if (clnt_call (clnt, REPLY, (xdrproc_t) xdr_reply_1_argument, (caddr_t) &arg,
		(xdrproc_t) xdr_int, (caddr_t) &clnt_res,
		TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&clnt_res);
}


/////////////////////////////peer client///////////////////////////

int
main (int argc, char **argv)
{

	//deal with input
		if (argc < 5) {
				std::cout << "Usage: ./serverside server_ip server_port coordinator_ip coordinator_port\n";
				exit(1);
		}

		string server_ip((char *) argv[1], strlen((char *)argv[1]));
		int server_port = stoi(argv[2]);
		//char *coordinator_ip = (char *) argv[3];
		string coordinator_ip((char *) argv[3], strlen((char *)argv[3]));
		int coordinator_port = stoi(argv[4]);

		cout << "Start server at: " << endl;
		cout << server_ip << " " << server_port << endl;

		//if(coordinator_ip!=server_ip || coordinator_port!=server_port){
		  PeerClient pclient(server_ip, server_port, coordinator_ip, coordinator_port);
    //}
		 // else
	   //  PeerClient pclient(server_ip, server_port);

	register SVCXPRT *transp;

	pmap_unset (COMMUNICATE_PROG, COMMUNICATE_VERSION);

	transp = svcudp_create(RPC_ANYSOCK);
	if (transp == NULL) {
		fprintf (stderr, "%s", "cannot create udp service.");
		exit(1);
	}
	if (!svc_register(transp, COMMUNICATE_PROG, COMMUNICATE_VERSION, communicate_prog_1, IPPROTO_UDP)) {
		fprintf (stderr, "%s", "unable to register (COMMUNICATE_PROG, COMMUNICATE_VERSION, udp).");
		exit(1);
	}

	transp = svctcp_create(RPC_ANYSOCK, 0, 0);
	if (transp == NULL) {
		fprintf (stderr, "%s", "cannot create tcp service.");
		exit(1);
	}
	if (!svc_register(transp, COMMUNICATE_PROG, COMMUNICATE_VERSION, communicate_prog_1, IPPROTO_TCP)) {
		fprintf (stderr, "%s", "unable to register (COMMUNICATE_PROG, COMMUNICATE_VERSION, tcp).");
		exit(1);
	}
	svc_run ();
	fprintf (stderr, "%s", "svc_run returned");
	exit (1);
	/* NOTREACHED */
}
