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
#ifndef FOREXPROSTOOLS_DATA_STOR_HPP_INCLUDED
#define FOREXPROSTOOLS_DATA_STOR_HPP_INCLUDED
//------------------------------------------------------------------------------
#include <ForexprostoolsApiEasy.hpp>
#include "xquotes_json_storage.hpp"
//------------------------------------------------------------------------------
namespace ForexprostoolsDataStor {
    using namespace ForexprostoolsApiEasy;

    /** \brief Хранилище новостей
     */
    class DataStore {
    private:
        xquotes_json_storage::JsonStorage iStorage;
        std::string path;                       /**< Путь к базе данных */
        unsigned long long timestamp_beg = 0;   /**< Метка времени начала исторических данных */
        unsigned long long timestamp_end = 0;   /**< Метка времени конца исторических данных */
        NewsList hist;                          /**< Исторические данные */

        /** \brief Разбить имя валютной пары на составляющие валюты
         * \param pair_name имя валютной пары
         * \param currency_1 первая валюта валютной пары
         * \param currency_2 вторая валюта валютной пары
         * \return вернет 0 в случае успеха
         */
        inline int get_currencies(std::string pair_name, std::string &currency_1, std::string &currency_2) {
            const size_t NAME_LEN = 6;
            if(pair_name.size() < NAME_LEN) return INVALID_PARAMETER;
            pair_name.erase(std::remove_if(pair_name.begin(), pair_name.end(), [](int c) {
                return !std::isalpha(c);
            }), pair_name.end());
            const std::string str_del = "frx";
            std::string::size_type pos = pair_name.find(str_del);
            while(pos != std::string::npos) {
                pair_name.erase(pos, str_del.size());
                pos = pair_name.find(str_del, pos + 1);
            }
            std::transform(pair_name.begin(), pair_name.end(),pair_name.begin(), ::toupper);
            if(pair_name.size() != NAME_LEN) return INVALID_PARAMETER;
            currency_1 = pair_name.substr(0, 3);
            currency_2 = pair_name.substr(3, 3);
            return OK;
        }
    public:

        /** \brief Инициализировать базу данных новостей
         * \param _path путь к базе данных
         */
        DataStore(const std::string &path) : iStorage(path) {};

        /** \brief Проверить наличие новостей за торговый день по метке времени
         * \param timestamp метка времени
         * \return вернет true если файл есть
         */
        bool check_timestamp(const xtime::timestamp_t timestamp) {
            return iStorage.check_timestamp(timestamp);
        }

        /** \brief Узнать максимальную и минимальную метку времени подфайлов
         * \param min_timestamp Метка времени в начале дня начала исторических данных
         * \param max_timestamp Метка времени в начале дня конца исторических данных
         * \return Вернет 0 в случае успеха, иначе см. код ошибок в xquotes_common.hpp
         */
        int get_min_max_timestamp(xtime::timestamp_t &min_timestamp, xtime::timestamp_t &max_timestamp) {
            return iStorage.get_min_max_timestamp(min_timestamp, max_timestamp);
        }

        /** \brief Установить отступ данных от дня загрузки
         *
         * Отсутп позволяет загрузить используемую область данных заранее.
         * Это позволит получать данные в пределах области без повторной загрузки подфайлов.
         * При этом, цесли данные выйдут за пределы области, то область сместится, произойдет подзагрузка
         * недостающих данных.
         * \param indent_timestamp_past Отступ от даты загрузки в днях к началу исторических данных
         * \param indent_timestamp_future Отступ от даты загрузки в днях к концу исторических данных
         */
        void set_indent(
                const uint32_t indent_timestamp_past,
                const uint32_t indent_timestamp_future) {
            iStorage.set_indent(indent_timestamp_past, indent_timestamp_future);
        }

        /** \brief Сохранить данные
         *
         * Метод  принудительно сохраняет все данные, которые еще не записаны в файл а находятся только в буфере.
         */
        void save() {
            iStorage.save();
        }

        /** \brief Записать новости за один торговый день
         * \param list_news Список новостей
         * \param timestamp Метка времени
         * \return Вернет 0 в случае успеха
         */
        int write_news(const std::vector<News> &list_news, const xtime::timestamp_t timestamp) {
            nlohmann::json j;
            for(size_t i = 0; i < list_news.size(); ++i) {
                j[i]["name"] = list_news[i].name;
                j[i]["currency"] = list_news[i].currency;
                j[i]["country"] = list_news[i].country;
                j[i]["volatility"] = list_news[i].level_volatility;
                j[i]["timestamp"] = list_news[i].timestamp;
                if(list_news[i].is_previous) j[i]["previous"] = list_news[i].previous;
                if(list_news[i].is_actual) j[i]["actual"] = list_news[i].actual;
                if(list_news[i].is_forecast) j[i]["forecast"] = list_news[i].forecast;
            }
            return iStorage.write_json(j, xtime::get_first_timestamp_day(timestamp));
        }

        /** \brief Прочитать новости за торговый день
         * \param list_news Список новостей
         * \param timestamp Метка времени
         * \return Вернет 0 в случае успеха
         */
        int read_news(std::vector<News> &list_news, const xtime::timestamp_t timestamp) {
            list_news.clear();
            nlohmann::json j;
            try {
                int err = iStorage.get_json(j, xtime::get_first_timestamp_day(timestamp));
                if(err != xquotes_common::OK) return err;
                list_news.resize(j.size());
                for(size_t i = 0; i < list_news.size(); ++i) {
                    list_news[i].name = j[i]["name"];
                    list_news[i].currency = j[i]["currency"];
                    list_news[i].country = j[i]["country"];
                    list_news[i].level_volatility = j[i]["volatility"];
                    list_news[i].timestamp = j[i]["timestamp"];
                    auto it_previous = j[i].find("previous");
                    auto it_actual = j[i].find("actual");
                    auto it_forecast = j[i].find("forecast");
                    if(it_previous != j[i].end()) list_news[i].previous = *it_previous;
                    if(it_actual != j[i].end()) list_news[i].actual = *it_actual;
                    if(it_forecast != j[i].end()) list_news[i].forecast = *it_forecast;
                }
            }
            catch(...) {
                return PARSER_ERROR;
            }
            return OK;
        }


        /** \brief Получить новости
         * \param timestamp Метка времени
         * \param indent_timestamp_past Максимальный отступ до метки времени
         * \param indent_timestamp_future Максимальный отступ после метки времени
         * \param news_data Список новостей
         * \return Вернет 0 в случае отсутствия ошибок
         */
        int get(
                const xtime::timestamp_t timestamp,
                const xtime::timestamp_t indent_timestamp_past,
                const xtime::timestamp_t indent_timestamp_future,
                std::vector<News> &news_data) {
            const xtime::timestamp_t start_timestamp = xtime::get_first_timestamp_day(timestamp - indent_timestamp_past);
            const xtime::timestamp_t stop_timestamp = xtime::get_first_timestamp_day(timestamp + indent_timestamp_future);

            /* проверяем доступность данных */
            if(start_timestamp < timestamp_beg || stop_timestamp > timestamp_end) {
                hist.clear();
                for(xtime::timestamp_t t = start_timestamp; t <= stop_timestamp; t += xtime::SECONDS_IN_DAY) {
                    std::vector<News> list_news;
                    int err = read_news(list_news, t);
                    if(err == OK) hist.add_news(list_news);
                }
                timestamp_beg = start_timestamp;
                timestamp_end = stop_timestamp;
            } else return NO_DATA_ACCESS;
            return hist.get_news(timestamp, indent_timestamp_past, indent_timestamp_future, news_data);
        }

        /** \brief Фильтр новостей
         *
         * Данный метод поместит в state NEWS_FOUND, если есть новости, или NO_NEWS, если новостей нет
         * \param pair_name имя валютной пары
         * \param timestamp Текущее время (Метка времени)
         * \param indent_timestamp_past Максимальный отступ до метки времени
         * \param indent_timestamp_future Максимальный отступ после метки времени
         * \param min_level_volatility Минимальный уровень силы новости (от 0 до 2)
         * \param state состояние фильтра (NEWS_FOUND или NO_NEWS)
         * \return вернет 0 в случае успеха
         */
        int filter(
                const std::string &pair_name,
                const xtime::timestamp_t timestamp,
                const xtime::timestamp_t indent_timestamp_past,
                const xtime::timestamp_t indent_timestamp_future,
                const int min_level_volatility,
                int &state) {
            state = NO_NEWS;
            std::string currency_1, currency_2;
            int err = get_currencies(pair_name, currency_1, currency_2);
            if(err != OK) return err;

            std::vector<News> news_data;
            err = get(timestamp, indent_timestamp_past, indent_timestamp_future, news_data);
            if(err != OK) return err;

            for(size_t i = 0; i < news_data.size(); ++i) {
                if((news_data[i].currency == currency_1 || news_data[i].currency == currency_2) &&
                (news_data[i].level_volatility >= min_level_volatility)) {
                    state = NEWS_FOUND;
                    return OK;
                }
            }
            return OK;
        }

        /** \brief Фильтр новостей
         *
         * Данный метод поместит в state NEWS_FOUND, если есть новости, или NO_NEWS, если новостей нет
         * \param pair_name имя валютной пары
         * \param timestamp Текущее время (Метка времени)
         * \param indent_timestamp_past Максимальный отступ до метки времени
         * \param indent_timestamp_future Максимальный отступ после метки времени
         * \param is_only_select Использовать только выбранные уровни силы новости.
         * Если есть новость с другим уровнем силы, функция поместит в state NEWS_FOUND.
         * \param is_low Использовать слабые новости.
         * \param is_moderate Использовать новости средней силы.
         * \param is_high Использовать сильные новости.
         * \param state состояние фильтра (NEWS_FOUND или NO_NEWS)
         * \return вернет 0 в случае успеха
         */
        int filter(
                const std::string &pair_name,
                const xtime::timestamp_t timestamp,
                const xtime::timestamp_t indent_timestamp_past,
                const xtime::timestamp_t indent_timestamp_future,
                const bool is_only_select,
                const bool is_low,
                const bool is_moderate,
                const bool is_high,
                int &state) {
            state = NO_NEWS;
            std::string currency_1, currency_2;
            int err = get_currencies(pair_name, currency_1, currency_2);
            if(err != OK) return err;

            std::vector<News> news_data;
            err = get(timestamp, indent_timestamp_past, indent_timestamp_future, news_data);
            if(err != OK) return err;
            for(size_t i = 0; i < news_data.size(); ++i) {
                if(news_data[i].currency != currency_1 &&
                    news_data[i].currency != currency_2) continue;
                if(news_data[i].level_volatility == ForexprostoolsApiEasy::LOW) {
                    if(is_low) state = NEWS_FOUND;
                    else if(is_only_select) {
                        state = NO_NEWS;
                        return OK;
                    }
                } else
                if(news_data[i].level_volatility == ForexprostoolsApiEasy::MODERATE) {
                    if(is_moderate) state = NEWS_FOUND;
                    else if(is_only_select) {
                        state = NO_NEWS;
                        return OK;
                    }
                } else
                if(news_data[i].level_volatility == ForexprostoolsApiEasy::HIGH) {
                    if(is_high) state = NEWS_FOUND;
                    else if(is_only_select) {
                        state = NO_NEWS;
                        return OK;
                    }
                }
            }
            return OK;
        }

        /** \brief Проверить новости
         *
         * Данный метод вернет true если есть новости или новость по указанным параметрам.
         * \param pair_name имя валютной пары
         * \param timestamp Текущее время (Метка времени)
         * \param indent_timestamp_past Максимальный отступ до метки времени
         * \param indent_timestamp_future Максимальный отступ после метки времени
         * \param is_only_select Использовать только выбранные уровни силы новости.
         * Если есть новость с другим уровнем силы, функция поместит в state NEWS_FOUND.
         * \param is_low Использовать слабые новости.
         * \param is_moderate Использовать новости средней силы.
         * \param is_high Использовать сильные новости.
         * \return вернет true если есть новость, подходящая по указанным параметрам
         */
        bool is_news(
                const std::string &pair_name,
                const xtime::timestamp_t timestamp,
                const xtime::timestamp_t indent_timestamp_past,
                const xtime::timestamp_t indent_timestamp_future,
                const bool is_only_select,
                const bool is_low,
                const bool is_moderate,
                const bool is_high) {
            int state = NO_NEWS;
            int err = filter(
                pair_name,
                timestamp,
                indent_timestamp_past,
                indent_timestamp_future,
                is_only_select,
                is_low,
                is_moderate,
                is_high,
                state);
            return (err == OK && state == NEWS_FOUND);
        }
    };
}
#endif // FOREXPROSTOOLS_DATA_STOR_HPP_INCLUDED
