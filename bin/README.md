# forexprostools-downloader-1.5

Программа скачивает исторические данные новостей, включая 14 дней (*начиная с версии* **1.4, текущая версия 1.5**) из будущего, начиная от текущего дня по *UTC* времени. 
**Последние 14 дней, содержащиеся в хранилище новостей, будут перезаписаны**, так как эти данные могут содержать неполные данные новостей.

## Команды и флаги

Все команды и флаги начинаются с символов: <-->,<-> или </>. 
Все команды представлены ниже:

* *path_json <строка>* | *pj <строка>* | *json_file <строка>* | *jf <строка>* - Путь к файлу настроек в формате JSON.
* *path_database <строка>* | *pd <строка>* - Путь к базе данных новостей.
* *use_day_off* | *udo* - Флаг включает загрузку выходных дней. По умолчанию выходные дни тоже записываются в хранилище данных новостей.
* *not_use_day_off* | *nudo* - Флаг отключает загрузку выходных дней. 

Пример *bat-файла*:

```
forexprostools-downloader.exe /jf "forexprostools-downloader-settings.json"
pause
```

Также параметры можно задавать через *JSON* файл.

* ключ *path_database* - указывает на путь путь к файлу, начиная от текущей директории программы или от переемнной окружения (если указана)
* ключ *environmental_variable* - указывает на имя переменной окружения, которая должна содержать директорию. 
Если переменная окружения задана, то путь к файлу с новостями формируется следующим образом:

```C++
const char* env_ptr = std::getenv(environmental_variable.c_str());
path_database = std::string(env_ptr) + "\\" + path_database;
```

аналогично формируется путь к файлу сертификата для работы с https:

```C++
const char* env_ptr = std::getenv(environmental_variable.c_str());
sert_file = std::string(env_ptr) + "\\" + sert_file;
```

* ключ *sert_file* - указывает на путь к файлу сертификата
* ключ *use_day_off* - использовать выходные дни

Примеры файла *JSON*:

```json
{
	"path_database":"..\\storage\\forexprostools.dat",
	"is_use_day_off":true
}
```

```json
{
	"path_database":"storage/news/forexprostools.dat",
	"environmental_variable":"MY_DIR",
	"sert_file":"bin/curl-ca-bundle.crt",
	"use_day_off":true
}
```