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
#ifndef FOREXPROSTOOLSAPIEASY_HPP_INCLUDED
#define FOREXPROSTOOLSAPIEASY_HPP_INCLUDED

#include "banana_filesystem.hpp"
#include <xtime.hpp>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <vector>
#include <string>
#include <cctype>
#include <iomanip>

namespace ForexprostoolsApiEasy {

        /// Набор возможных состояний ошибки
        enum ErrorType {
            OK = 0,                 ///< Ошибок нет, все в порядке
            NO_DATA_ACCESS = -1,    ///< Нет доступа к данным
            UNKNOWN_ERROR = -3,     ///< Неопределенная ошибка
            PARSER_ERROR = -4,      ///< Ошибка парсера
            INVALID_PARAMETER = -6, ///< Один из параметров неверно указан
        };

        /// Уровни волатильности
        enum VolatilityType {
            NOT_INIT = -1,
            LOW = 0,        ///< Новости слабой силы
            MODERATE = 1,   ///< Новости средней силы
            HIGH = 2,       ///< Сильные новости
        };

        /// Состояния фильтра
        enum FilterState {
            NEWS_FOUND = 0, ///< Есть новость или новости
            NO_NEWS = 1,    ///< Новости не найдены
        };

        /** \brief Класс Новостей
         */
        class News {
        public:
            std::string name;                       /**< Имя новости */
            std::string currency;                   /**< Валюта новости */
            std::string country;                    /**< Страна новости */
            int level_volatility = NOT_INIT;        /**< Уровень волатильности (-1 не инициализировано, 0,1,2) */
            double previous = 0.0;                  /**< Предыдущее значение */
            double actual = 0.0;                    /**< Актуальное значение */
            double forecast = 0.0;                  /**< Предсказанное значение */
            bool is_previous = false;               /**< Наличие предыдущего значения */
            bool is_actual = false;                 /**< Наличие актуального значения */
            bool is_forecast = false;               /**< Наличие предсказанного значения */
            xtime::timestamp_t timestamp = 0;       /**< Временная метка новости */

            News() {};
        };

        /** \brief Список новостей
         *
         * Данный класс хранит в себе массив новостей и позволяет получать к нему удобный доступ черех методы класса
         */
        class NewsList {
        private:
            std::vector<News> list_news_;
        public:
            NewsList() {};

        /** \brief Добавить новости
         * \param list_news список новостей
         */
        void add_news(const std::vector<News> &list_news) {
            list_news_.insert(list_news_.end(), list_news.begin(), list_news.end());
            std::sort(list_news_.begin(), list_news_.end(), [](const News &lhs, const News &rhs) {
                return lhs.timestamp < rhs.timestamp;
            });
        }

        /** \brief Инициализировать список новостей
         * \param list_news список новостей
         */
        NewsList(const std::vector<News> &list_news) {
            add_news(list_news);
        }

        /** \brief Получить новости по метке времени
         *
         * \param timestamp Метка времени
         * \param indent_timestamp_past Максимальный отступ до метки времени
         * \param indent_timestamp_future Максимальный отступ после метки времени
         * \param list_news Список новостей
         * \return Ыернет 0 в случае успеха
         */
        int get_news(
                const xtime::timestamp_t timestamp,
                const xtime::timestamp_t indent_timestamp_past,
                const xtime::timestamp_t indent_timestamp_future,
                std::vector<News> &list_news) {
            if(list_news_.size() == 0) return NO_DATA_ACCESS;
            const xtime::timestamp_t start_time = timestamp - indent_timestamp_past;
            const xtime::timestamp_t stop_time = timestamp + indent_timestamp_future;
            auto lower = std::lower_bound(list_news_.begin(), list_news_.end(), start_time, [](const News &lhs, unsigned long long rhs) {
                return lhs.timestamp < rhs;
            });
            auto upper = std::upper_bound(list_news_.begin(), list_news_.end(), stop_time, [](unsigned long long lhs, const News &rhs) {
                return lhs < rhs.timestamp;
            });

            if(lower == list_news_.end() && upper == list_news_.end()) {
                return NO_DATA_ACCESS;
            } else
            if(lower == list_news_.begin() && upper == list_news_.begin()) {
                return NO_DATA_ACCESS;
            } else
            if(lower == list_news_.end() && upper == list_news_.begin()) {
                return NO_DATA_ACCESS;//*
            }
            list_news.clear();
            list_news.insert(list_news.begin(), lower, upper);
            return OK;
        }

        /** \brief Получить новости по метке времени
         * \param timestamp Метка времени
         * \param indent_timestamp_past Максимальный отступ до метки времени
         * \param indent_timestamp_future Максимальный отступ после метки времени
         * \param list_news Список новостей
         * \param time_diff Разница во времени между меткой врепмени и началом новости
         * \return Вернет 0 в случае успеха
         */
        int get_news(
                const xtime::timestamp_t timestamp,
                const xtime::timestamp_t indent_timestamp_past,
                const xtime::timestamp_t indent_timestamp_future,
                std::vector<News> &list_news,
                std::vector<long> &time_diff) {
            int err = get_news(timestamp, indent_timestamp_past, indent_timestamp_future, list_news);
            if(err != OK)
                return err;
            time_diff.resize(list_news.size());
            for(size_t i = 0; i < list_news.size(); ++i) {
                time_diff[i] = (long long)list_news[i].timestamp - (long long)timestamp;
            }
            return OK;
        }

        /** \brief Очистить список новостей
         */
        void clear() {
            list_news_.clear();
        }
    };
//------------------------------------------------------------------------------
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!!!!!!!!!!!ДАЛЬШЕ УСТАРЕВШИЙ КОД! НЕ РЕКОМЕНДУЕТСЯ ИСПОЛЬЗОВАТЬ!!!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//                               ¯\_(=_=)_/¯
//------------------------------------------------------------------------------
        /** \brief Получить имя файла из даты
         * Выбрана последовательность ГОД МЕСЯЦ ДЕНЬ чтобы файлы были
         * в алфавитном порядке
         * \param timestamp временная метка
         * \return имя файла
         */
        std::string get_file_name_from_date(unsigned long long timestamp)
        {
                xtime::DateTime iTime(timestamp);
                std::string file_name =
                        std::to_string(iTime.year) + "_" +
                        std::to_string(iTime.month) + "_" +
                        std::to_string(iTime.day);
                return file_name;
        }
//------------------------------------------------------------------------------
        /** \brief Записать файл новостей
         * \param file_name имя файла
         * \param list_news список новостей
         */
        void write_news_file(std::string file_name, std::vector<News> &list_news)
        {
                std::ofstream file(file_name);
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
                file << std::setw(4) << j << std::endl;
                file.close();
        }
//------------------------------------------------------------------------------
        /** \brief Прочитать файл новостей
         * \param file_name имя файла
         * \param list_news список новостей
         * \return вернет 0 в случае успеха
         */
        int read_news_file(std::string file_name, std::vector<News> &list_news)
        {
                std::ifstream file(file_name);
                list_news.clear();
                nlohmann::json j;
                try {
                        file >> j;
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
                        file.close();
                        return PARSER_ERROR;
                }
                file.close();
                return OK;
        }
//------------------------------------------------------------------------------
        /** \brief Найти первую и последнюю дату файлов
         * \param file_list список файлов
         * \param file_extension расширение файла (например .json)
         * \param beg_timestamp первая дата, встречающееся среди файлов
         * \param end_timestamp последняя дата, встречающееся среди файлов
         * \return вернет 0 в случае успеха
         */
        int get_beg_end_timestamp(
                std::vector<std::string> &file_list,
                std::string file_extension,
                unsigned long long &beg_timestamp,
                unsigned long long &end_timestamp) {
                if(file_list.size() == 0)
                        return INVALID_PARAMETER;
                beg_timestamp = std::numeric_limits<unsigned long long>::max();
                end_timestamp = std::numeric_limits<unsigned long long>::min();
                for(size_t i = 0; i < file_list.size(); i++) {
                        std::vector<std::string> path_file;
                        bf::parse_path(file_list[i], path_file);
                        if(path_file.size() == 0)
                                continue;
                        std::string file_name = path_file.back();
                        // очищаем слово от расширения
                        std::size_t first_pos = file_name.find(file_extension);
                        if(first_pos == std::string::npos)
                                continue;
                        unsigned long long time;
                        std::string word = file_name.substr(0, first_pos);
                        if(xtime::convert_str_to_timestamp(file_name.substr(0, first_pos), time)) {
                                if(beg_timestamp > time)
                                        beg_timestamp = time;
                                if(end_timestamp < time)
                                        end_timestamp = time;
                        }
                }
                if(beg_timestamp == std::numeric_limits<unsigned long long>::max() ||
                        end_timestamp == std::numeric_limits<unsigned long long>::min())
                        return UNKNOWN_ERROR;
                return OK;
        }
//------------------------------------------------------------------------------
        /** \brief Найти первую и последнюю дату файлов
         * \param path директория с файлами исторических данных
         * \param file_extension расширение файла (например .hex или .zstd)
         * \param beg_timestamp первая дата, встречающееся среди файлов
         * \param end_timestamp последняя дата, встречающееся среди файлов
         * \return вернет 0 в случае успеха
         */
        int get_beg_end_timestamp_for_path(
                std::string path,
                std::string file_extension,
                unsigned long long &beg_timestamp,
                unsigned long long &end_timestamp) {
                std::vector<std::string> file_list;
                bf::get_list_files(path, file_list, true);
                return get_beg_end_timestamp(file_list, file_extension, beg_timestamp, end_timestamp);
        }
//------------------------------------------------------------------------------
        /** \brief База данных новостей
         */
        class DataBase {
        private:
                std::string path;                       /**< Путь к базе данных */
                unsigned long long timestamp_beg = 0;   /**< Временная метка исторических данных */
                unsigned long long timestamp_end = 0;   /**< Временная метка исторических данных */
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
                        while (pos != std::string::npos) {
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
                DataBase() {};

                /** \brief Инициализировать базу данных новостей
                 * \param _path путь к базе данных
                 */
                DataBase(std::string _path) {
                        path = _path;
                }


                /** \brief Получить новости
                 * \param _timestamp временная метка
                 * \param _time_indent_dn максимальный отступ до временной метки
                 * \param _time_indent_up максимальный отступ после временной метки
                 * \param news_data список новостей
                 * \return вернет 0 в случае отсутствия ошибок
                 */
                int get(
                        unsigned long long _timestamp,
                        unsigned long long _time_indent_dn,
                        unsigned long long _time_indent_up,
                        std::vector<News> &news_data) {
                        unsigned long long start_time = _timestamp - _time_indent_dn;
                        unsigned long long stop_time = _timestamp + _time_indent_up;

                        xtime::DateTime iStartTime(start_time); iStartTime.set_beg_day();
                        xtime::DateTime iStopTime(stop_time); iStopTime.set_beg_day();

                        unsigned long long start_timestamp = iStartTime.get_timestamp();
                        unsigned long long stop_timestamp = iStopTime.get_timestamp();

                        // проверяем доступность данных
                        if(start_timestamp < timestamp_beg || stop_timestamp > timestamp_end) {
                                hist.clear();
                                for(unsigned long long t = start_timestamp; t <= stop_timestamp; t += xtime::SECONDS_IN_DAY) {
                                        std::vector<News> list_news;
                                        read_news_file(path + "//" + get_file_name_from_date(t) + ".json", list_news);
                                        hist.add_news(list_news);
                                }
                                timestamp_beg = start_timestamp;
                                timestamp_end = stop_timestamp;
                        }
                        return hist.get_news(_timestamp, _time_indent_dn, _time_indent_up, news_data);
                }

                /** \brief Фильтр новостей
                 * Данный метод поместит в state NEWS_FOUND, если есть новости, или NO_NEWS, если новостей нет
                 * \param _pair_name имя валютной пары
                 * \param _timestamp текущее время (временная метка)
                 * \param _time_indent_dn максимальный отступ до временной метки
                 * \param _time_indent_up максимальный отступ после временной метки
                 * \param min_level_volatility минимальый уровень силы новости (от 0 до 2)
                 * \param state состояние фильтра (NEWS_FOUND или NO_NEWS)
                 * \return вернет 0 в случае успеха
                 */
                int filter(
                        std::string _pair_name,
                        unsigned long long _timestamp,
                        unsigned long long _time_indent_dn,
                        unsigned long long _time_indent_up,
                        int min_level_volatility,
                        int &state) {

                        std::string currency_1, currency_2;
                        int err = get_currencies(_pair_name, currency_1, currency_2);
                        if(err != OK) return err;

                        std::vector<News> news_data;
                        err = get(_timestamp, _time_indent_dn, _time_indent_up, news_data);
                        if(err != OK) return err;

                        state = NO_NEWS;
                        for(size_t i = 0; i < news_data.size(); ++i) {
                                if((news_data[i].currency == currency_1 || news_data[i].currency == currency_2) &&
                                (news_data[i].level_volatility >= min_level_volatility)) {
                                        state = NEWS_FOUND;
                                        return OK;
                                }
                        }
                        return OK;
                }
        };
//------------------------------------------------------------------------------
}

#endif // FOREXPROSTOOLSAPIEASY_HPP_INCLUDED
