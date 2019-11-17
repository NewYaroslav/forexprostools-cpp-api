#include <iostream>
#include <iomanip>
#include <cctype>
#include <ForexprostoolsApi.hpp>
#include <ForexprostoolsDataStore.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

#define PROGRAM_VERSION "1.0"
#define PROGRAM_DATE "17.11.2019"

int main(int argc, char* argv[]) {
    std::string path_database = "..//..//storage//forexprostools.dat"; // путь к базе данных новостей
    ForexprostoolsDataStor::DataStore iDataStore(path_database);

    /* получим минимальную и максимальную даты новостей
     */
    xtime::timestamp_t min_timestamp = 0, max_timestamp = 0;
    iDataStore.get_min_max_timestamp(min_timestamp, max_timestamp);
    std::cout << "date: " << xtime::get_str_date(min_timestamp) << " - " << xtime::get_str_date(max_timestamp) << std::endl;

    /* получим новости за один день */
    std::vector<ForexprostoolsApiEasy::News> day_list_news;
    iDataStore.get(xtime::get_timestamp(15,11,2019), 0, xtime::SECONDS_IN_DAY, day_list_news);

    if(true) {
        for(size_t i = 0; i < day_list_news.size(); ++i) {
            std::cout << "news: " << i << std::endl;
            std::cout << xtime::get_str_date_time(day_list_news[i].timestamp) << std::endl;
            std::cout << "name: " << day_list_news[i].name << std::endl;
            std::cout << "currency: " << day_list_news[i].currency << std::endl;
            std::cout << "country: " << day_list_news[i].country << std::endl;
            std::cout << "level_volatility: " << day_list_news[i].level_volatility << std::endl;
            std::cout << "previous: " << day_list_news[i].previous << std::endl;
            std::cout << "actual: " << day_list_news[i].actual << std::endl;
            std::cout << "forecast: " << day_list_news[i].forecast << std::endl;
        }
        if(false) {
            //iDataStore.write_news(day_list_news,  xtime::get_timestamp(13,11,2019));
            //iDataStore.get_min_max_timestamp(min_timestamp, max_timestamp);
            //std::cout << "date: " << xtime::get_str_date(min_timestamp) << " - " << xtime::get_str_date(max_timestamp) << std::endl;
        }
    }
    std::cout << "date: " << xtime::get_str_date(min_timestamp) << " - " << xtime::get_str_date(max_timestamp) << std::endl;
    return 0;
}

