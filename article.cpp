#include <iostream>
#include <vector>
#include <sstream>
#include <map>
#include "article.h"

using namespace std;



string intToStr(int x) { stringstream ss;
    ss << x;
    return ss.str();
}

string toString(char *s) {
    string s1(s, strlen(s));
    return s;
}

Article::Article(int id, string article): content(article), index(id) {}

ArticlePool::ArticlePool(): count(0) {
}

//initialize an articlePool with pool struct 
ArticlePool::ArticlePool(ArticlePoolStruct pool) {
    count = 0;
    for (int i = 0; i < pool.count; i++) {
        ArticleStruct * art = pool.artciles.array_article_val + i;
        if (art->depth == 0) {
            post(toString(art->content));
        } else {
            reply(toString(art->content), art->depth);
        }
    }
}
//
////TODO
//~ArticlePool() {
//}

int ArticlePool::storeArticle(string article, int father) {
    if (father > 0 && father <= count) { 
        count++;
        Article *head = articleMap[father];
        Article * now = new Article(count, article);
        head->nextArticles.push_back(now);
        articleMap[count] = now;
        isHeadArticle.push_back(false);
        return count;
    }
    else if (father == 0) {
        count++;
        Article *now = new Article(count, article);
        articleMap[count] = now;
        isHeadArticle.push_back(true);
        return count;
    } else {
        //cout << "Article " << father << " doesn't exist." << endl;
    }
    return 0;
}

Article* ArticlePool::choose(int index) {
    if (articleMap.find(index) != articleMap.end()) {
        return articleMap[index];
    } else {
        //cout << "Article " << index << " doesn't exist." << endl;
        return NULL;
    }
} //reply to article with index
    //return the index of the new article
    //the index == 0 if doesn't post successfully
int ArticlePool::reply(string article, int index) { 
    return storeArticle(article, index);
}

    //post an article
    //return the index of the new article
    //the index == 0 if doesn't post successfully
int ArticlePool::post(string article) {
    //cout << " into articlePool's post "<< endl;
    return storeArticle(article, 0);
}

void ArticlePool::readArticleContent(string & articles, Article *now, int level) {
    string currentLine = "";
    for (int i = 0; i < 4 * level; i++) {
        currentLine.push_back(' ');
    }
    currentLine += intToStr(now->index);
    currentLine.push_back(' ');
    currentLine += now->content;
    articles += currentLine;
    articles.push_back('\n');
    for (int i = 0; i < now->nextArticles.size(); i++) {
        readArticleContent(articles, now->nextArticles[i], level + 1);
    }
}



ArticlePool ArticlePool::deepCopy() {
    ArticlePool newPool = ArticlePool();
    cout << newPool.count << endl;
    cout << count << endl;
    newPool.count = count;
    for (int i = 0; i < isHeadArticle.size(); i++) {
        newPool.isHeadArticle.push_back(isHeadArticle[i]);
    }
    for (map<int, Article *>::iterator it = articleMap.begin(); it != articleMap.end(); it++) {
        newPool.articleMap[it->first] = it->second;
    }
    return newPool;
}


    //read the content of article
string ArticlePool::read() {
    string articles = "";
    for (int i = 1; i <= count; i++) {
        if (isHeadArticle[i - 1]) {
            Article * now = articleMap[i];
            readArticleContent(articles, now, 0);
        }
    }
    return articles;
}

int ArticlePool::getCount() {
    return count;
}

void ArticlePool::getArticleContent(ArticleStruct* &article, Article *now, int level) {
    article->index = now->index;
    article->content = new char[now->content.length() + 1];
    strcpy(article->content, now->content.c_str());
    article->depth = level;
    ++article;
    for (int i = 0; i < now->nextArticles.size(); i++) {
        getArticleContent(article, now->nextArticles[i], now->index);
    }
}

ArticlePoolStruct ArticlePool::getArticle() {
    ArticlePoolStruct res;
    res.count = count;
    res.update_count = count; 
    if (count == 0) return res;
    res.artciles.array_article_val = new ArticleStruct[count];
    ArticleStruct *articleP = res.artciles.array_article_val; 
    for (int i = 1; i <= count; i++) {
        if (isHeadArticle[i - 1]) {
            Article * now = articleMap[i];
            int pos = 0;
            getArticleContent(articleP, now, 0);
        }
    }
    return res;
}

void PrintArticlePoolStruct(ArticlePoolStruct pool) {
    cout << "Count: " << pool.count << "\nupdate_count: " << pool.update_count << endl;
    if (pool.count == 0) return;
    ArticleStruct *article = pool.artciles.array_article_val;
    for (int i = 0; i < pool.count; i++) {
        for (int j = 0; j < article->depth; j++) {
            for (int k = 0; k < 4; k++) {
                cout << " ";
            }
        }
        cout << article->index << " " << article->content << endl;
        article++;
    }
}



//int main() {
//    ArticlePool articlePool;
//    //------------test article pool-----/
//    ArticlePool newPool2 = articlePool.deepCopy();
//    articlePool.post("Article 1");
//    cout << articlePool.read() << endl;
//    int id1 = articlePool.post("Article 2");
//    cout << articlePool.read() << endl;
//    articlePool.reply("A reply for Article 2", id1);
//    cout << articlePool.read() << endl;
//    int id2 = articlePool.reply("A reply for Article 2", id1);
//    cout << articlePool.read() << endl;
//    articlePool.reply("A reply for Article 4", id2);
//    cout << articlePool.read() << endl;
//    cout << "--------tests for single choose-----" << endl;
//    Article * now = articlePool.choose(id1);
//    if (now != NULL) 
//    cout << now->index << " " << now->content << endl;
//    now = articlePool.choose(id2);
//    if (now != NULL) 
//    cout << now->index << " " << now->content << endl;
//    now = articlePool.choose(6);
//    ArticlePool newPool = articlePool.deepCopy();
//    newPool.post("newnewnew");
//    cout << articlePool.read() << endl;
//    cout << newPool.read() << endl;
//    cout << "testing article pool struct" << endl;
//    cout << articlePool.read() << endl;
//    ArticlePoolStruct pool = articlePool.getArticle();
//    PrintArticlePoolStruct(pool);
//    ArticlePool newPool3(pool);
//    cout << newPool3.read() << endl;
//    return 0;
//}
