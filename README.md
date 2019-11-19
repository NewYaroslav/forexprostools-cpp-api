# forexprostools-cpp-api
Библиотека на С++ для загрузки новостей с Forexprostools.com

## Описание
Данная *header-only* библиотека содержит класс для взаимодействия с экономическим календарем [https://www.investing.com/webmaster-tools/economic-calendar](https://www.investing.com/webmaster-tools/economic-calendar)

Класс *ForexprostoolsApi* позволяет загружать исторические данные за любой период. 

На данный момент библиотека находится в разработке, поэтому в репозитории в любой момент может что-то поменяться.

## Программа для загрузки новостей

Репозиторий содержит готовую программу *bin/forexprostools-downloader.exe* для загрузки новостей с сайта *www.investing.com* за весь период. 
При повторном вызове программа перезагрузит лишь последнюю неделю в уже ранее загруженных данных. Для работы программы могут понадобиться *dll* библиотеки, они расположены здесь *bin/dll.7z*.
Уже загруженная база данных находится здесь: *storage/forexprostools.dat*.

## Как пользоваться?
После подключения всех зависимостей в проект надо просто добавить заголовочный файл *ForexprostoolsApi.hpp*.
Если интересен только доступ к историческим данным новостей, можно опдключить только *ForexprostoolsDataStore.hpp*, чтобы не подключать *curl*.
Подробнее о зависиостях смотрите в заголовке "Зависимости*.

Файл *ForexprostoolsApi.hpp* содержит код для работы с API.
Файл *ForexprostoolsApiEasy.hpp* содержит вспомогательные функции, в частности класс *News* для хранения данных новостей.
Файл *ForexprostoolsDataStore.hpp* содержит класс *DataStore* для хранения новостей в формате файла хранилища библиотеки [xquotes_history](https://github.com/NewYaroslav/xquotes_history)

### Класс для хранения новости

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
	uint64_t timestamp = 0;       			/**< Метка времени новости */

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
	
	std::string path_database = "..//..//storage//forexprostools.dat"; // путь к базе данных новостей
    ForexprostoolsDataStor::DataStore iDataStore(path_database);

    /* получим минимальную и максимальную даты новостей
     */
    xtime::timestamp_t min_timestamp = 0, max_timestamp = 0;
    iDataStore.get_min_max_timestamp(min_timestamp, max_timestamp);
    std::cout 
		<< "date: " 
		<< xtime::get_str_date(min_timestamp) 
		<< " - " 
		<< xtime::get_str_date(max_timestamp) 
		<< std::endl;

    /* получим новости за один день */
    std::vector<ForexprostoolsApiEasy::News> day_list_news;
    iDataStore.get(
		xtime::get_timestamp(15,11,2019), 
		0, 
		xtime::SECONDS_IN_DAY, day_list_news);
	
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
	
	/* найдем новость с заданным уровнем волатильности в пределах 20 минут от метки времени */
	for(xtime::timestamp_t t = min_timestamp; t <= max_timestamp; t += xtime::SECONDS_IN_MINUTE) {
        int news_state = ForexprostoolsApiEasy::NO_NEWS;
        iDataStore.filter(
			"EURUSD", 
			t, 
			xtime::SECONDS_IN_HOUR/3, 
			xtime::SECONDS_IN_HOUR/3, 
			true, 
			true, 
			false, 
			false, 
			news_state);
        if(news_state == ForexprostoolsApiEasy::NEWS_FOUND) {
            std::cout 
				<< "news_state: " 
				<< news_state 
				<< " date " 
				<< xtime::get_str_date_time(t) 
				<< std::endl;
            break;
        }
    }
	
	/* грузим ... историю */
	ForexprostoolsApi api;

	std::vector<ForexprostoolsApiEasy::News> list_news; // список новостей
	// загружаем новости
	int err = api.download_all_news(
		xtime::get_unix_timestamp(1, 11, 2018, 0, 0, 0), 
		xtime::get_unix_timestamp(30, 11, 2018, 0, 0, 0), 
		list_news);
	return 0;
}
```

## Зависимости

*forexprostools-cpp-api* зависит от следующих внешних библиотек / пакетов

* Библиотека *CURL* для работы с *https*. Помимо *curl* вам может понадобится еще *OpenSSL* - [https://curl.haxx.se/windows/](https://curl.haxx.se/windows/) [https://github.com/NewYaroslav/curl-7.60.0-win64-mingw](https://github.com/NewYaroslav/curl-7.60.0-win64-mingw)
* *Header-only* библиотека для работы с *JSON* - [https://github.com/nlohmann/json](https://github.com/nlohmann/json)
* Библиотека *zlib* для работы с сжатыми данными. Библиотеку нужно собрать или подключит ее файлы в проект. Репозиторий: [https://github.com/madler/zlib](https://github.com/madler/zlib)
* Библиотека *gzip*, это оболочка над *zlib*, рекомендую этот [fork](https://github.com/NewYaroslav/gzip-hpp), а не оригинал, так как в оригинале нет загрушек в *header* файлах - [https://github.com/mapbox/gzip-hpp](https://github.com/mapbox/gzip-hpp) [https://github.com/NewYaroslav/gzip-hpp](https://github.com/NewYaroslav/gzip-hpp)
* Библиотека *xquotes_history* для работы с хранилищем новостей. Репозиторий: [https://github.com/NewYaroslav/xquotes_history](https://github.com/NewYaroslav/xquotes_history)
* Библиотека *xtime*, эта библиотека нужна для работы с меткой времени. Можно просто добавить два файла *.cpp* и *.hpp* в проект. Репозиторий: [https://github.com/NewYaroslav/xtime_cpp.git](https://github.com/NewYaroslav/xtime_cpp.git)
* *gcc* или *mingw* с поддержкой *C++11*, например: [https://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win64/Personal%20Builds/mingw-builds/7.3.0/threads-posix/seh/x86_64-7.3.0-release-posix-seh-rt_v5-rev0.7z/download](https://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win64/Personal%20Builds/mingw-builds/7.3.0/threads-posix/seh/x86_64-7.3.0-release-posix-seh-rt_v5-rev0.7z/download)

Все необходимые библиотеки добавлены, как субмодули, в папку lib. 