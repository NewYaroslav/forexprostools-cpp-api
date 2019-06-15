#include <iostream>
#include <iomanip>
#include <ForexprostoolsApi.hpp>

int main()
{
        std::cout << "Hello world!" << std::endl;
        ForexprostoolsApi api;

        std::vector<ForexprostoolsApiEasy::News> list_news; // список новостей
        // загружаем новости
        int err = api.download_all_news(xtime::get_unix_timestamp(14, 6, 2019, 0, 0, 0), xtime::get_unix_timestamp(14, 6, 2019, 23, 59, 59), list_news);
#if(1)
        // выводим на экран
        for(size_t i = 0; i < list_news.size(); ++i) {
                //std::cout << list_news[i].name << std::endl;
        }
#endif
        // инициализируем хранилище новостей
        ForexprostoolsApiEasy::NewsList news_data(list_news);
        std::vector<ForexprostoolsApiEasy::News> day_list_news; // список новостей вблизи временной метки
        // получим новый список новостей
        int err_news_data = news_data.get_news(xtime::get_unix_timestamp(14, 6, 2019, 0, 0, 0), xtime::SEC_HOUR * 24, xtime::SEC_HOUR * 24, day_list_news);
        std::cout << "err_news_data " << err_news_data << std::endl;


        ForexprostoolsApiEasy::write_news_file("test.json", day_list_news);
        ForexprostoolsApiEasy::read_news_file("test.json", day_list_news);
         // выводим новый список
#if(1)
        for(size_t i = 0; i < day_list_news.size(); ++i) {
                std::cout << "news: " << i << std::endl;
                std::cout << xtime::get_str_unix_date_time(day_list_news[i].timestamp) << std::endl;
                std::cout << "name: " << day_list_news[i].name << std::endl;
                std::cout << "currency: " << day_list_news[i].currency << std::endl;
                std::cout << "country: " << day_list_news[i].country << std::endl;
                std::cout << "level_volatility: " << day_list_news[i].level_volatility << std::endl;
                std::cout << "previous: " << day_list_news[i].previous << std::endl;
                std::cout << "actual: " << day_list_news[i].actual << std::endl;
                std::cout << "forecast: " << day_list_news[i].forecast << std::endl;
        }
#endif
        return 0;
}

