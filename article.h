/* Needed so that include header is included only once during compilation*/
#pragma once

#include <iostream>
#include <vector>
#include <sstream>
#include <map>
#include <string>
#include <cstdio>
#include "communicate.h"
using namespace std;

string intToStr(int x);

struct Article {
public:
    string content;
    int index;
    vector<Article*> nextArticles;
    Article(int id, string article);
};

class ArticlePool {
private:
    map<int, Article*> articleMap;
    vector<bool> isHeadArticle;
    int count;

    int storeArticle(string article, int father);
    void readArticleContent(string & articles, Article *now, int level);
    void getArticleContent(ArticleStruct* &articleP, Article *now, int level);
public:
    ArticlePool();
    ArticlePool(ArticlePoolStruct pool);
    ~ArticlePool();

    //store the article into articleMap
    //return the index of the new article
    //if doesn't store successfully, return index 0
    Article* choose(int index);

    //reply to article with index
    //return the index of the new article
    //the index == 0 if doesn't post successfully
    int reply(string article, int index);

    //post an article
    //return the index of the new article
    //the index == 0 if doesn't post successfully
    int post(string article);

    //read the content of article
    string read();
    ArticlePoolStruct getArticle();

    //ArticlePool deepCopy();
    int getCount();

    void releaseAll();
    void releaseArticle(Article *article);
    
    void encodeArticle(char *&buffer, Article *article, int father);
    void encodeString(char *& buffer, string content);
    void encodeInt(char *& buffer, int x);
    string decodeString(char *& buffer);
    int decodeInt(char *& buffer);
    char * encodeArticlePool();
    void decodeArticlePool(char * pool);
};
