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
void encodeString(char *& buffer, string content, int len);
void encodeInt(char *& buffer, int x);
string decodeString(char *& buffer, int len);
int decodeInt(char *& buffer);

class Article {
public:
    string content;
    int index;
    vector<Article*> nextArticles;
    Article(int id, string article);
};

class ArticlePool {
private:
    vector<bool> isHeadArticle;

public:
    int count;
    map<int, Article*> articleMap;
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
    int storeArticle(string article, int father);
    int writeArticle(string article, int father, int index);
    void readArticleContent(string & articles, Article *now, int level);
    void getArticleContent(ArticleStruct* &articleP, Article *now, int level);

    void releaseAll();
    void releaseArticle(Article *article);

    void encodeArticle(char *&buffer, Article *article, int father);
    char * encodeArticlePool();
    void decodeArticlePool(char * pool);
    void PrintArticlePoolStruct(ArticlePoolStruct pool);

};
