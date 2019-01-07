#include <iostream>
#include <iomanip>
#include <ForexprostoolsApi.hpp>

int main()
{
        std::cout << "Hello world!" << std::endl;
        ForexprostoolsApi api;

        std::vector<ForexprostoolsApiEasy::News> list_news; // список новостей
        // загружаем новости
        int err = api.download_all_news(xtime::get_unix_timestamp(1, 11, 2018, 0, 0, 0), xtime::get_unix_timestamp(30, 11, 2018, 0, 0, 0), list_news);
        // выводим на экран
        for(size_t i = 0; i < list_news.size(); ++i) {
                std::cout << list_news[i].name << std::endl;
        }

        // инициализируем хранилище новостей
        ForexprostoolsApiEasy::NewsList news_data(list_news);
        std::vector<ForexprostoolsApiEasy::News> day_list_news; // список новостей вблизи временной метки
        // получим новый список новостей
        int err_news_data = news_data.get_news(xtime::get_unix_timestamp(5, 11, 2018, 6, 0, 0), xtime::SEC_HOUR * 5, xtime::SEC_HOUR * 5, day_list_news);
        std::cout << "err_news_data " << err_news_data << std::endl;

        // выводим новый список
        for(size_t i = 0; i < day_list_news.size(); ++i) {
                std::cout << xtime::get_str_unix_date_time(day_list_news[i].timestamp) << " " << day_list_news[i].name << std::endl;
        }

        return 0;
}

