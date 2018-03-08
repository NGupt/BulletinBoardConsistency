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
xdr_ArticleContent (XDR *xdrs, ArticleContent *objp)
{
	register int32_t *buf;

	 if (!xdr_int (xdrs, &objp->index))
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
