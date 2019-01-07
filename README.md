# forexprostools-cpp-api
Библиотека на С++ для загрузки новостей с Forexprostools.com

### Описание
Данная *header-only* библиотека содержит класс для взаимодействия с экономическим календарем [https://sslecal2.forexprostools.com](https://sslecal2.forexprostools.com/?columns=exc_flags,exc_currency,exc_importance,exc_actual,exc_forecast,exc_previous&category=_employment,_economicActivity,_inflation,_credit,_centralBanks,_confidenceIndex,_balance,_Bonds&importance=3&features=datepicker,timezone,timeselector,filters&countries=25,32,6,37,72,22,17,39,14,10,35,43,56,36,110,11,26,12,4,5&calType=week&timeZone=55&lang=1)

Класс *ForexprostoolsApi* позволяет загружать исторические данные за любой период. 

На данный момент библиотека находится в разработке

### Как пользоваться?
После подключения всех зависимостей в проект просто добавить заголовочный файл *ForexprostoolsApi.hpp*

Файл *ForexprostoolsApiEasy.hpp* содержит вспомогательные функции, в частности класс *News* для хранения данных новостей

```C++
/** \brief Класс Новостей
 */
class News
{
public:
		std::string name;                       /**< Имя новости */
		std::string currency;                   /**< Валюта новости */
		std::string country;                    /**< Страна новости */
		int level_volatility = NOT_INIT;        /**< Уровень волатильности (-1 не инициализировано, 0,1,2) */
		double previous;                        /**< Предыдущее значение */
		double actual;                          /**< Актуальное значение */
		double forecast;                        /**< Предсказанное значение */
		bool is_previous = false;               /**< Наличие предыдущего значения */
		bool is_actual = false;                 /**< Наличие актуального значения */
		bool is_forecast = false;               /**< Наличие предсказанного значения */
		unsigned long long timestamp = 0;       /**< Временная метка новости */

		News() {};
};
```

### Пример программы

```C++
#include <iostream>
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
```

### Зависимости

*forexprostools-cpp-api* зависит от следующих внешних библиотек / пакетов

* *Библиотека CURL* - [https://curl.haxx.se/windows/](https://curl.haxx.se/windows/) [https://github.com/NewYaroslav/curl-7.60.0-win64-mingw](https://github.com/NewYaroslav/curl-7.60.0-win64-mingw)
* *Библиотека JSON* - [https://github.com/nlohmann/json](https://github.com/nlohmann/json)
* *Библиотека zlib* - [https://github.com/madler/zlib](https://github.com/madler/zlib)
* *Библиотека gzip* - [https://github.com/mapbox/gzip-hpp](https://github.com/mapbox/gzip-hpp) [https://github.com/NewYaroslav/gzip-hpp](https://github.com/NewYaroslav/gzip-hpp)
* *Библиотека xtime* - [https://github.com/NewYaroslav/xtime_cpp.git](https://github.com/NewYaroslav/xtime_cpp.git)
* *gcc* или *mingw* с поддержкой C++11, например - [https://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win64/Personal%20Builds/mingw-builds/7.3.0/threads-posix/seh/x86_64-7.3.0-release-posix-seh-rt_v5-rev0.7z/download](https://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win64/Personal%20Builds/mingw-builds/7.3.0/threads-posix/seh/x86_64-7.3.0-release-posix-seh-rt_v5-rev0.7z/download)

Все необходимые библиотеки добавлены, как субмодули, в папку lib. 