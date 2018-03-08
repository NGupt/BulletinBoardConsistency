/* Needed so that include header is included only once during compilation*/
#pragma once

#include <iostream>
#include <vector>
#include <sstream>
#include <map>
#include <string>
#include <cstdio>
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
public:
    ArticlePool();

    //store the article into articleMap
    //return the index of the new article
    //if doesn't store successfully, return index 0
    int storeArticle(string article, int father);

    Article* choose(int index);

    //reply to article with index
    //return the index of the new article
    //the index == 0 if doesn't post successfully
    int reply(string article, int index);

    //post an article
    //return the index of the new article
    //the index == 0 if doesn't post successfully
    int post(string article);

    void getArticleContent(string & articles, Article *now, int level);


    //read the content of article
    string read();

    int getCount();
};
