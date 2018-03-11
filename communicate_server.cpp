/*
 * This is sample code generated by rpcgen.
 * These are only templates and you can use them
 * as a guideline for developing your own functions.
 */

#include <iostream>
#include "communicate.h"
#include "article.h"
#include "string.h"
using namespace std;

static ArticlePool articlePool;

int *
post_1_svc(char *content,  struct svc_req *rqstp)
{
    std::string myString(content, strlen(content));
    static int result = 0;
    int now = articlePool.post(myString);
    result = now;
    if (result == 0) {
        cout << "Post fails. " << endl;
    } else {
        cout << "Post an article:" << endl;
        cout << result << " " << content << endl;
    }
    
	/*
	 * insert server code here
	 */

	return &result;
}

char **
read_1_svc(struct svc_req *rqstp)
{
	static char * result = new char[MAXSTRING];
    string resultStr = articlePool.read();
    strcpy(result, resultStr.c_str());
	/*
	 * insert server code here
	 */
    cout << "Read from server:" << endl;
    cout << result << endl;
	return &result;
}

ArticleContent *
choose_1_svc(int index,  struct svc_req *rqstp)
{
	static ArticleContent  result;
    result.content = new char[MAXSTRING];
    Article * resultArticle = articlePool.choose(index);
	/*
	 * insert server code here
	 */
    if (resultArticle == NULL) {
        strcpy(result.content, "");
        result.index = 0;
        cout << "The article with id " << index << " doesn't exist in the server." << endl;
    } else {
        strcpy(result.content, resultArticle->content.c_str());
        result.index = resultArticle->index;
        cout << "The client choose the article: " << endl;
        cout << result.index << " " << result.content << endl;
    }
	return &result;
}

int *
reply_1_svc(char *content, int index,  struct svc_req *rqstp)
{
    string resultStr(content, strlen(content));
	static int result = 0;
    int now = articlePool.reply(resultStr, index);
    result = now;
    if (result == 0) {
        cout << "Can't reply to article with id " << index << "." << endl;
    } else {
        cout << "Reply article " << index << " with:";
        cout << result << " " << resultStr << endl;
    }

	/*
	 * insert server code here
	 */

	return &result;
}

