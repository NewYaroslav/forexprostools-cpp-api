#include <iostream>
#include <iomanip>
#include <cctype>
#include <fstream>
#include <ForexprostoolsDataStore.hpp>

using json = nlohmann::json;

int main(int argc, char* argv[]) {
    std::string path_database = "..//..//storage//forexprostools.dat"; // путь к базе данных новостей
    ForexprostoolsDataStore::DataStore iNewsDataBase(path_database);

    std::cout << "find the number of unique news..." << std::endl;

    std::map<std::string,uint64_t> unique_news;
    std::vector<std::pair<std::string,uint64_t>> array_unique_news;
    xtime::timestamp_t news_min_time = 0, news_max_time = 0;
    iNewsDataBase.get_min_max_timestamp(news_min_time, news_max_time);
    xtime::for_days(news_min_time, news_max_time,[&](const xtime::timestamp_t &t){
        std::vector<ForexprostoolsApiEasy::News> list_news;
        iNewsDataBase.read_news(list_news, t);
        for(size_t n = 0; n < list_news.size(); ++n) {
            unique_news[list_news[n].name]++;
        }
    });

    std::cout << "end" << std::endl;
    std::cout << "unique_news.size: " << unique_news.size() << std::endl;
    std::transform(
            unique_news.begin(),
            unique_news.end(),
            std::back_inserter(array_unique_news),
                   [](std::pair<std::string,uint64_t> e) -> std::pair<std::string,uint64_t> {
                   return e;
    });

    std::sort(
            array_unique_news.begin(),
            array_unique_news.end(),
            [](const std::pair<std::string,uint64_t> &a, const std::pair<std::string,uint64_t> &b) {
        return a.second < b.second;
    });

    std::ofstream fout("unique_news.txt");
    for(size_t s = 0; s < array_unique_news.size(); ++s) {
        std::cout << array_unique_news[s].first << " " << array_unique_news[s].second << std::endl;
        fout << array_unique_news[s].first << " " << array_unique_news[s].second << std::endl;
    }
    fout << "total news: " << array_unique_news.size() << std::endl;
    fout.close();
    return 0;
}

