#include <iostream>
#include <iomanip>
#include <cctype>
#include <ForexprostoolsApi.hpp>
#include <ForexprostoolsDataStore.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

#define PROGRAM_VERSION "1.3"
#define PROGRAM_DATE "19.11.2019"

int main(int argc, char* argv[]) {
    std::cout << "forexprostools downloader " << PROGRAM_VERSION << " " << PROGRAM_DATE << std::endl;
    std::string path_json;	// путь к файлу json с настройками
    std::string path_database; // путь к базе данных новостей
    bool is_use_day_off = true;

    if(argc == 1) {
        std::cout << "Error! No parameters!" << std::endl;
        return -1;
    }

    /* парсим команды */
    for(int i = 1; i < argc; ++i) {
        std::string value = std::string(argv[i]);
        if((value == "path_json") && (i + 1) < argc) {
            path_json = std::string(argv[i + 1]);
        } else
        if((value == "path_database") && (i + 1) < argc) {
            path_database = std::string(argv[i + 1]);
        }
        if((value == "-nodayoff")) {
            is_use_day_off = false;
        }
    }
    if(path_json.size() == 0 && path_database.size() == 0) {
        std::cout << "Error! The path to the database or settings file is incorrect!" << std::endl;
        return -1;
    }

    /* Если надо, загружаем парметры из файла json */
    if(path_json.size() != 0) {
        json settings_json;
        try {
            std::ifstream file_json(path_json);
            file_json >> settings_json;
            file_json.close();
        }
        catch(...) {
            std::cout << "Error, file json cannot be opened!" << std::endl;
            return -1;
        }
        try {
            path_database = settings_json["path_database"];
            if(settings_json["is_use_day_off"] != nullptr) is_use_day_off = settings_json["is_use_day_off"];
            if(settings_json["is_skip_day_off"] != nullptr) {
                if(settings_json["is_skip_day_off"] == true) {
                    is_use_day_off = false;
                } else {
                    is_use_day_off = true;
                }
            }
        }
        catch(...) {
            std::cout << "Error, json file does not contain necessary objects!" << std::endl;
            return -1;
        }
    }

    /* создаем папку */
    if(path_database.size() != 0) {
        std::string only_path_database = bf::set_file_extension(path_database, "");
        if(only_path_database == path_database) path_database += "\\forexprostools-news.dat";
        else {
            std::vector<std::string> list_element;
            bf::parse_path(only_path_database, list_element);
            only_path_database.clear();
            for(size_t i = 0; i < list_element.size() - 1; ++i) {
                only_path_database += list_element[i];
                only_path_database += "\\";
            }
        }
        bf::create_directory(only_path_database);
    } else {
        path_database = "forexprostools-news.dat";
    }

    std::cout << "path: " << path_database << std::endl;
    std::cout << "use day off: " << is_use_day_off << std::endl;
    std::cout << "start of download..." << std::endl;

    ForexprostoolsDataStore::DataStore iDataStore(path_database);

    /* Определим начальное время загрузки данных
     * Если данные уже есть, обновим данные за последнюю неделю,
     * так как последняя неделя могла быть загружена ранее с недостающими данными
     */
    xtime::timestamp_t min_timestamp = 0, max_timestamp = 0;
    iDataStore.get_min_max_timestamp(min_timestamp, max_timestamp);
    const xtime::timestamp_t SECONDS_IN_WEEK = xtime::SECONDS_IN_DAY*7;
    if(max_timestamp > SECONDS_IN_WEEK) max_timestamp -= SECONDS_IN_WEEK;
    else max_timestamp = 0;
    /* отобразим дату данных, если есть корректные метки времени */
    if(min_timestamp != 0 && max_timestamp != 0) {
        std::cout
            << "already downloaded, date: "
            << xtime::get_str_date(min_timestamp)
            << " - "
            << xtime::get_str_date(max_timestamp)
            << std::endl;
    }

    /* начинаем згрузку данных  через API */
    int err = xquotes_common::OK;
    ForexprostoolsApi api;
    api.download_and_save_all_data(max_timestamp, xtime::get_timestamp(), is_use_day_off, [&](
            const std::vector<ForexprostoolsApiEasy::News> &list_news,
            const xtime::timestamp_t timestamp) {
        /* запишем полученне данные в хранилище  */
        err = iDataStore.write_news(list_news, timestamp);
        if(err != xquotes_common::OK) {
            std::cout << "write error, code: " << err << "\r";
            return;
        }
        std::cout
            << "downloaded data from https://sslecal2.forexprostools.com, date: "
            << xtime::get_str_date(timestamp)
            << "\r";
        iDataStore.save();
    });
    std::cout << std::endl;
    if(err == xquotes_common::OK) std::cout << "data download completed" << std::endl;
    else std::cout << "error downloading data, code: " << err << std::endl;
    return err;
}

