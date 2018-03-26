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

string toString(char *s) {
    string s1(s, strlen(s));
    return s;
}

Article::Article(int id, string article) : content(article), index(id) {}

ArticlePool::ArticlePool() : count(0) {
}

//initialize an articlePool with pool struct 
ArticlePool::ArticlePool(ArticlePoolStruct pool) {
    count = 0;
    for (int i = 0; i < pool.count; i++) {
        ArticleStruct *art = pool.artciles.array_article_val + i;
        if (art->depth == 0) {
            post(toString(art->content));
        } else {
            reply(toString(art->content), art->depth);
        }
    }
}

ArticlePool::~ArticlePool() {
    //releaseAll();
}

void ArticlePool::releaseArticle(Article *article) {
    for (int i = 0; i < article->nextArticles.size(); i++) {
        releaseArticle(article->nextArticles[i]);
        delete article->nextArticles[i];
    }
    article->nextArticles.clear();
    article->content.clear();
}


void ArticlePool::releaseAll() {
    for (int i = 1; i <= count; i++) {
        if (isHeadArticle[i]) {
            releaseArticle(articleMap[i]);
            delete articleMap[i];
        }
    }
    articleMap.clear();
    isHeadArticle.clear();
    count = 0;
}

int ArticlePool::storeArticle(string article, int father) {
    if (father > 0 && father <= count) {
        count++;
        Article *head = articleMap[father];
        Article *now = new Article(count, article);
        head->nextArticles.push_back(now);
        articleMap[count] = now;
        isHeadArticle.push_back(false);
        return count;
    } else if (father == 0) {
        count++;
        Article *now = new Article(count, article);
        articleMap[count] = now;
        isHeadArticle.push_back(true);
        return count;
    } else {
        cout << "Article index " << father << " doesn't exist" << endl;
        return -1;
    }
}



int ArticlePool::writeArticle(string article, int father, int index) {
    //cout << "in article " << index << " " << article << " " << father << " " << count << endl;

    map<int, Article*>::iterator iter;

    iter = articleMap.find(father);

    if (father == 0) {
        count++;
        Article *now = new Article(index, article);
        articleMap[count] = now;
        isHeadArticle.push_back(true);
        return count;
    } else if (iter->first > 0 && iter->first <= count) {
        count++;

        Article *head = articleMap[iter->first];
        Article *now = new Article(index, article);
        head->nextArticles.push_back(now);
        articleMap[count] = now;
        isHeadArticle.push_back(false);
        return count;
    } else {
        cout << "Article index " << father << " doesn't exist" << endl;
        return -1;
    }
}



Article *ArticlePool::choose(int index) {
    if (articleMap.find(index) != articleMap.end()) {
        return articleMap[index];
    } else {
        //cout << "Article index " << index << " doesn't exist." << endl;
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

void ArticlePool::readArticleContent(string &articles, Article *now, int level) {
    string currentLine = "";
//    for (int i = 0; i < 4 * level; i++) {
//        currentLine.push_back(' ');
//    }
    for (int i = 0; i < level; i++) {
        currentLine.push_back('\t');
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


//return an article pointer which is a deep copy of the given one;
//ArticlePool ArticlePool::deepCopy(Article *article) {
//    Article *res = new Article();
//    res->index = article->index;
//    res->content = article->content;
//    for (int i = 0; i < article->nextArticles.size(); i++) {
//        res->nextArticles.push_back(deepCopy(article->nextArticles[i]));
//    }
//    return res;
//}


//ArticlePool ArticlePool::deepCopy() {
//    ArticlePool newPool = ArticlePool();
//    cout << newPool.count << endl;
//    cout << count << endl;
//    newPool.count = count;
//    for (int i = 0; i < isHeadArticle.size(); i++) {
//        newPool.isHeadArticle.push_back(isHeadArticle[i]);
//    }
//    for (int i = 1; i <= count; i++) {
//        if (isHeadArticles[i]) {
//            newPool.articleMap[it->first] = deepCopy(it->second);
//        }
//    }
//    return newPool;
//}


//read the content of article
//string ArticlePool::read() {
//    string articles = "";
//    for (int i = 1; i <= count; i++) {
//        if (isHeadArticle[i - 1]) {
//            Article *now = articleMap[i];
//            readArticleContent(articles, now, 0);
//        }
//    }
//    return articles;
//}


string ArticlePool::read() {
    string articles = "";
    //std::map<int, Article*> mymap;
    std::map<int, Article*>::iterator it;
    for (it=articleMap.begin(); it!=articleMap.end(); ++it){
        if (isHeadArticle[it->first-1]) {
            Article *now = it->second;
            readArticleContent(articles, now, 0);
        }
    }
    return articles;
}


int ArticlePool::getCount() {
    return count;
}

void ArticlePool::getArticleContent(ArticleStruct *&article, Article *now, int level) {
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
            Article *now = articleMap[i];
            int pos = 0;
            getArticleContent(articleP, now, 0);
        }
    }
    return res;
}

void ArticlePool::PrintArticlePoolStruct(ArticlePoolStruct pool) {
    cout << "Count: " << pool.count << "\nupdate_count: " << pool.update_count << endl;
    if (pool.count == 0) return;
    ArticleStruct *article = pool.artciles.array_article_val;
    for (int i = 0; i < pool.count; i++) {
        for (int j = 0; j < article->depth; j++) {
            for (int k = 0; k < 4; k++) {
                cout << " ";
            }
        }
        cout << "printing " << article->index << " " << article->content << endl;
        article++;
    }
}

void encodeInt(char *&buffer, int x) {
    *buffer = x & 0xff;
    *(++buffer) = (x >> 8) & 0xff;
    *(++buffer) = (x >> 16) & 0xff;
    *(++buffer) = (x >> 24) & 0xff;
    buffer++;
}

void encodeString(char *&buffer, string s, int len) {
    strcpy(buffer, s.c_str());
    buffer += len;
}

void ArticlePool::encodeArticle(char *&buffer, Article *article, int father) {
    encodeInt(buffer, father);
    encodeString(buffer, article->content, MAXSTRING);
    for (int i = 0; i < article->nextArticles.size(); i++) {
        encodeArticle(buffer, article->nextArticles[i], article->index);
    }
}

int decodeInt(char *&buffer) {
    int x = *buffer & 0xff;
    x = x | ((*(++buffer) & 0xff) << 8);
    x = x | ((*(++buffer) & 0xff) << 16);
    x = x | ((*(++buffer) & 0xff) << 24);
    buffer++;
    return x;
}

string decodeString(char *&buffer, int len) {
    string s(buffer, strlen(buffer));
    buffer += len;
    return s;
}

char *ArticlePool::encodeArticlePool() {
    //store count(4)
    //content(101) father(4)
    char *res = new char[4 + (MAXSTRING + 4) * count];
    char *r = res;
    encodeInt(r, count);
    for (int i = 1; i <= count; i++) {
        if (isHeadArticle[i - 1]) {
            encodeArticle(res, articleMap[i], 0);
        }
    }
    return r;


}


void ArticlePool::decodeArticlePool(char *article) {
    releaseAll();
    int size = decodeInt(article);
    for (int i = 0; i < size; i++) {
        int father = decodeInt(article);
        string s = decodeString(article, MAXSTRING);
        storeArticle(s, father);
    }
}




//int main() {
////    ArticlePool articlePool;
////    //------------test article pool-----/
////    ArticlePool newPool2(articlePool.getArticle());
////    articlePool.post("Article 1");
////    cout << "AAAA" << articlePool.read() << endl;
////    int id1 = articlePool.post("Article 2");
////    cout << articlePool.read() << endl;
////    articlePool.reply("A reply for Article 2", id1);
////    cout << articlePool.read() << endl;
////    int id2 = articlePool.reply("A reply for Article 2", id1);
////    cout << articlePool.read() << endl;
////    articlePool.reply("A reply for Article 4", id2);
////    cout << articlePool.read() << endl;
////    cout << "--------tests for single choose-----" << endl;
////    Article * now = articlePool.choose(id1);
////    if (now != NULL)
////    cout << now->index << " " << now->content << endl;
////    now = articlePool.choose(id2);
////    if (now != NULL)
////    cout << now->index << " " << now->content << endl;
////    now = articlePool.choose(6);
////    ArticlePool newPool(articlePool.getArticle());
////    newPool.post("newnewnew");
////    cout << articlePool.read() << endl;
////    cout << newPool.read() << endl;
////
////    cout << "testing article pool struct" << endl;
////    cout << articlePool.read() << endl;
////    ArticlePoolStruct pool = articlePool.getArticle();
////    articlePool.PrintArticlePoolStruct(pool);
//    ArticlePool newPool3;
//    newPool3.post("a new article");
//    cout << newPool3.read() << endl;
//    cout << "testing udp article message" << endl;
//    char * poolMessage = newPool3.encodeArticlePool();
//    cout << poolMessage << endl;
//    //newPool3.PrintArticlePoolStruct(poolMessage);
//    newPool3.decodeArticlePool(poolMessage);
//    //cout << newPool3.read();
//    return 0;
//}
