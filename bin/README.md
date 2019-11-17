### forexprostools-downloader.exe

Программа скачивает исторчиеские данные новостей, включая полный текущий день по UTC времени. 
Последняя неделя, содержащееся в хранилище новостей, будет перезаписана, так как она может содержать неполные данные новостей.

Команды и флаги:

* *path_json <строка>* Путь к файлу настроек в формате JSON.
* *path_database <строка>* Путь к базе данных новостей.
* *-nodayoff* Флаг отключения выходных дней. По умолчанию выходные дни тоже записываются в хранилище данных новостей.

Пример bat-файла:

```
forexprostools-downloader.exe path_json "forexprostools-downloader-settings.json"
pause
```

Пример файла JSON:

```json
{
	"path_database":"..\\storage\\forexprostools.dat",
	"is_use_day_off":true
}
```