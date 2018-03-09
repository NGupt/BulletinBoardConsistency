#include <iostream>
#include <vector>
#include <sstream>
#include <map>
#include "article.h"

using namespace std;



string intToStr(int x) {
    stringstream ss;
    ss << x;
    return ss.str();
}

Article::Article(int id, string article): content(article), index(id) {}

ArticlePool::ArticlePool() {
    count = 0;
}

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
}

    //reply to article with index
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

void ArticlePool::getArticleContent(string & articles, Article *now, int level) {
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
        getArticleContent(articles, now->nextArticles[i], level + 1);
    }
}


    //read the content of article
string ArticlePool::read() {
    string articles = "";
    for (int i = 1; i <= count; i++) {
        if (isHeadArticle[i - 1]) {
            Article * now = articleMap[i];
            getArticleContent(articles, now, 0);
        }
    }
    return articles;
}

int ArticlePool::getCount() {
    return count;
}

//int main() {
//    ArticlePool articlePool;
//    //------------test article pool-----/
//    articlePool.storeArticle("Article 1");
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
//    return 0;
//}
