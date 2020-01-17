/*
* forexprostools-cpp-api - Forexprostools C++ API client
*
* Copyright (c) 2018 Elektro Yar. Email: git.electroyar@gmail.com
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/
#include <iostream>
#include <iomanip>
#include <cctype>
#include <cstdlib>
#include <ForexprostoolsApi.hpp>
#include <ForexprostoolsDataStore.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

#define PROGRAM_VERSION "1.5"
#define PROGRAM_DATE "17.01.2020"

/* обработать все аргументы */
bool process_arguments(
    const int argc,
    char **argv,
    std::function<void(
        const std::string &key,
        const std::string &value)> f) noexcept {
    if(argc <= 1) return false;
    bool is_error = true;
    for(int i = 1; i < argc; ++i) {
        std::string key = std::string(argv[i]);
        if(key.size() > 0 && (key[0] == '-' || key[0] == '/')) {
            uint32_t delim_offset = 0;
            if(key.size() > 2 && (key.substr(2) == "--") == 0) delim_offset = 1;
            std::string value;
            if((i + 1) < argc) value = std::string(argv[i + 1]);
            is_error = false;
            f(key.substr(delim_offset), value);
        }
    }
    return !is_error;
}

int main(int argc, char* argv[]) {
    std::cout << "forexprostools downloader" << std::endl;
    std::cout
        << "version: " << PROGRAM_VERSION
        << " date: " << PROGRAM_DATE
        << std::endl << std::endl;

    std::string path_json;	// путь к файлу json с настройками
    std::string path_database; // путь к базе данных новостей
    std::string environmental_variable;
    std::string sert_file("curl-ca-bundle.crt");
    bool is_use_day_off = true;

    if(!process_arguments(argc, argv,[&](const std::string &key, const std::string &value){
        if (key == "path_json" ||
            key == "pj" ||
            key == "json_file" ||
            key == "jf") {
            path_json = value;
        } else
        if(key == "path_database" || key == "pd") {
            path_database = value;
        } else
        if(key == "use_day_off" || key == "udo") {
            is_use_day_off = true;
        } else
        if(key == "not_use_day_off" || key == "nudo") {
            is_use_day_off = false;
        }
    })) {
        std::cerr << "Error! No parameters!" << std::endl;
        return EXIT_FAILURE;
    }

    if(path_json.size() == 0 && path_database.size() == 0) {
        std::cerr << "Error! The path to the database or settings file is incorrect!" << std::endl;
        return EXIT_FAILURE;
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
            std::cerr << "Error, file json cannot be opened!" << std::endl;
            return EXIT_FAILURE;
        }
        try {
            if(settings_json["path_database"] != nullptr) path_database = settings_json["path_database"];
            if(settings_json["environmental_variable"] != nullptr) environmental_variable = settings_json["environmental_variable"];
            if(settings_json["sert_file"] != nullptr) sert_file = settings_json["sert_file"];
            if(settings_json["use_day_off"] != nullptr) is_use_day_off = settings_json["use_day_off"];
        }
        catch(...) {
            std::cerr << "Error, json file does not contain necessary objects!" << std::endl;
            return EXIT_FAILURE;
        }
    }

    if(environmental_variable.size() != 0) {
        const char* env_ptr = std::getenv(environmental_variable.c_str());
        if(env_ptr == NULL) {
            std::cerr << "Error, no environment variable!" << std::endl;
            return EXIT_FAILURE;
        }
        if(path_database.size() != 0) {
            path_database = std::string(env_ptr) + "\\" + path_database;
        }
        else path_database = std::string(env_ptr);
		sert_file = std::string(env_ptr) + "\\" + sert_file;
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
    std::cout << std::boolalpha;
    std::cout << "use day off: " << is_use_day_off << std::endl;
    std::cout << "start of download..." << std::endl;

    ForexprostoolsDataStore::DataStore iDataStore(path_database);

    /* Определим начальное время загрузки данных
     * Если данные уже есть, обновим данные за последнюю неделю,
     * так как последняя неделя могла быть загружена ранее с недостающими данными
     */
    xtime::timestamp_t min_timestamp = 0, max_timestamp = 0;
    iDataStore.get_min_max_timestamp(min_timestamp, max_timestamp);
    const xtime::timestamp_t SECONDS_IN_WEEK = xtime::SECONDS_IN_DAY * xtime::DAYS_IN_WEEK;
    const xtime::timestamp_t SECONDS_IN_WEEK_X2 = SECONDS_IN_WEEK * 2;
    const xtime::timestamp_t SECONDS_IN_WEEK_X2_1 = SECONDS_IN_WEEK_X2 + xtime::SECONDS_IN_DAY;

    if(max_timestamp > SECONDS_IN_WEEK_X2_1) max_timestamp -= SECONDS_IN_WEEK_X2_1;
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

    xtime::timestamp_t stop_timestamp = xtime::get_first_timestamp_day(xtime::get_timestamp());
    stop_timestamp += SECONDS_IN_WEEK_X2;
    int err = xquotes_common::NO_INIT;
    ForexprostoolsApi api(sert_file);
    /* начинаем згрузку данных  через API */
    int err_download = api.download_and_save_all_data(
                max_timestamp,
                stop_timestamp,
                is_use_day_off,
                [&](
            const std::vector<ForexprostoolsApiEasy::News> &list_news,
            const xtime::timestamp_t timestamp) {
        /* запишем полученне данные в хранилище  */
        err = iDataStore.write_news(list_news, timestamp);
        if(err != xquotes_common::OK) {
            std::cerr << "write error, code: " << err << "\r";
            return;
        }
        std::cout
            << "downloaded data from https://sslecal2.forexprostools.com, date: "
            << xtime::get_str_date(timestamp)
            << "\r";
        iDataStore.save();
    });
    std::cout << std::endl;
    if(err == xquotes_common::OK && err_download == ForexprostoolsApi::OK) {
        std::cout << "data download completed" << std::endl;
        return EXIT_SUCCESS;
    }
    std::cerr << "error downloading data" << std::endl;
    std::cerr << "write error, code: " << err << std::endl;
    std::cerr << "download error, code: " << err_download << std::endl;
    return EXIT_FAILURE;
}

