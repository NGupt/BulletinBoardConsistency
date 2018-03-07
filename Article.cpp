#include <iostream>
#include <vector>
#include <sstream>
#include <map>
using namespace std;

class FormatHelper {
public:
    static string intToStr(int x) { 
        stringstream ss;
        ss << x;
        return ss.str();
    }
};

struct Article {
    string content;
    int index;
    vector<Article*> nextArticles;
    Article(int id, string article): index(id), content(article) {}
};

class ArticlePool {
private:
    map<int, Article*> articleMap;
    vector<bool> isHeadArticle;
    int count;
public:
    ArticlePool() {
        count = 0;
    }

    //store the article into articleMap
    //return the index of the new article
    //if doesn't store successfully, return index 0
    int storeArticle(string article, int father = 0) {
        count++;
        if (father > 0 && father < count) { 
            Article *head = articleMap[father];
            Article * now = new Article(count, article);
            head->nextArticles.push_back(now);
            articleMap[count] = now;
            isHeadArticle.push_back(false);
            return count;
        } else if (father == 0) {
            Article *now = new Article(count, article);
            articleMap[count] = now;
            isHeadArticle.push_back(true);
            return count;
        } else {
            cout << "Article " << father << " doesn't exist." << endl;
        }
        return 0;
    }

    Article* choose(int index) {
        if (articleMap.find(index) != articleMap.end()) {
            return articleMap[index];
        } else {
            cout << "Article " << index << " doesn't exist." << endl;
            return NULL;
        }
    }

    //reply to article with index
    //return the index of the new article
    //the index == 0 if doesn't post successfully
    int reply(string article, int index) { 
        return storeArticle(article, index);
    }

    //post an article
    //return the index of the new article
    //the index == 0 if doesn't post successfully
    int post(string article) {
        return storeArticle(article);
    }

    void getArticleContent(string & articles, Article *now, int level) {
        string currentLine = "";
        for (int i = 0; i < 4 * level; i++) {
            currentLine.push_back(' ');
        }
        currentLine += FormatHelper::intToStr(now->index);
        currentLine.push_back(' ');
        currentLine += now->content;
        articles += currentLine;
        articles.push_back('\n');
        for (int i = 0; i < now->nextArticles.size(); i++) {
            getArticleContent(articles, now->nextArticles[i], level + 1);
        }
    }


    //read the content of article
    string read() {
        string articles = "";
        for (int i = 1; i <= count; i++) {
            if (isHeadArticle[i - 1]) {
                Article * now = articleMap[i];
                getArticleContent(articles, now, 0);
            }
        }
        return articles;
    }

    int getCount() {
        return count;
    }
};

int main() {
    ArticlePool articlePool;
    //------------test article pool-----/
    articlePool.storeArticle("Article 1");
    cout << articlePool.read() << endl;
    int id1 = articlePool.post("Article 2");
    cout << articlePool.read() << endl;
    articlePool.reply("A reply for Article 2", id1);
    cout << articlePool.read() << endl;
    int id2 = articlePool.reply("A reply for Article 2", id1);
    cout << articlePool.read() << endl;
    articlePool.reply("A reply for Article 4", id2);
    cout << articlePool.read() << endl;
    cout << "--------tests for single choose-----" << endl;
    Article * now = articlePool.choose(id1);
    if (now != NULL) 
    cout << now->index << " " << now->content << endl;
    now = articlePool.choose(id2);
    if (now != NULL) 
    cout << now->index << " " << now->content << endl;
    now = articlePool.choose(6);
    return 0;
}
