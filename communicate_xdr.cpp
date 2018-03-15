/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include "communicate.h"

bool_t
xdr_str (XDR *xdrs, str *objp)
{
	register int32_t *buf;

	 if (!xdr_string (xdrs, objp, MAXSTRING))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_IP (XDR *xdrs, IP *objp)
{
	register int32_t *buf;

	 if (!xdr_string (xdrs, objp, MAXIP))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_ArticleContent (XDR *xdrs, ArticleContent *objp)
{
	register int32_t *buf;

	 if (!xdr_int (xdrs, &objp->index))
		 return FALSE;
	 if (!xdr_str (xdrs, &objp->content))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_node (XDR *xdrs, node *objp)
{
	register int32_t *buf;

	 if (!xdr_IP (xdrs, &objp->ip))
		 return FALSE;
	 if (!xdr_int (xdrs, &objp->port))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_server_list (XDR *xdrs, server_list *objp)
{
	register int32_t *buf;

	 if (!xdr_array (xdrs, (char **)&objp->server_list_val, (u_int *) &objp->server_list_len, MAXSERVERS,
		sizeof (node), (xdrproc_t) xdr_node))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_ArticleStruct (XDR *xdrs, ArticleStruct *objp)
{
	register int32_t *buf;

	 if (!xdr_int (xdrs, &objp->index))
		 return FALSE;
	 if (!xdr_str (xdrs, &objp->content))
		 return FALSE;
	 if (!xdr_int (xdrs, &objp->depth))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_array_article (XDR *xdrs, array_article *objp)
{
	register int32_t *buf;

	 if (!xdr_array (xdrs, (char **)&objp->array_article_val, (u_int *) &objp->array_article_len, MAXARTICLES,
		sizeof (ArticleStruct), (xdrproc_t) xdr_ArticleStruct))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_ArticlePoolStruct (XDR *xdrs, ArticlePoolStruct *objp)
{
	register int32_t *buf;

	 if (!xdr_array_article (xdrs, &objp->artciles))
		 return FALSE;
	 if (!xdr_int (xdrs, &objp->count))
		 return FALSE;
	 if (!xdr_int (xdrs, &objp->update_count))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_reply_1_argument (XDR *xdrs, reply_1_argument *objp)
{
	 if (!xdr_string (xdrs, &objp->arg1, ~0))
		 return FALSE;
	 if (!xdr_int (xdrs, &objp->arg2))
		 return FALSE;
	return TRUE;
}