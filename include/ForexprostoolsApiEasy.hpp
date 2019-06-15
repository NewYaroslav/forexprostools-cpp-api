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

#include <fstream>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <vector>
#include <string>
//------------------------------------------------------------------------------
namespace ForexprostoolsApiEasy
{
//------------------------------------------------------------------------------
        /// Набор возможных состояний ошибки
        enum ErrorType {
                OK = 0,                 ///< Ошибок нет, все в порядке
                NO_ACCESS_DATA = -1,    ///< Нет доступа к данным
                PARSER_ERROR = -4,      ///< Ошибка парсера
        };
//------------------------------------------------------------------------------
        /// Уровни волатильности
        enum VolatilityType {
                NOT_INIT = -1,
                LOW = 0,
                MODERATE = 1,
                HIGH = 2,
        };
//------------------------------------------------------------------------------
        /** \brief Класс Новостей
         */
        class News
        {
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
                unsigned long long timestamp = 0;       /**< Временная метка новости */

                News() {};
        };
//------------------------------------------------------------------------------
        /** \brief Список новостей
         */
        class NewsList
        {
        private:
                std::vector<News> list_news_;
        public:
//------------------------------------------------------------------------------
                NewsList() {};
//------------------------------------------------------------------------------
                /** \brief Добавить новости
                 * \param list_news список новостей
                 */
                void add_news(std::vector<News> &list_news)
                {
                        list_news_.insert(list_news_.end(), list_news.begin(), list_news.end());
                        std::sort(list_news_.begin(), list_news_.end(), [](const News &lhs, const News &rhs) {
                                return lhs.timestamp < rhs.timestamp;
                        });
                }
//------------------------------------------------------------------------------
                /** \brief Инициализировать список новостей
                 * \param list_news список новостей
                 */
                NewsList(std::vector<News> &list_news)
                {
                        add_news(list_news);
                }
//------------------------------------------------------------------------------
                /** \brief Получить новости по временной метке
                 * \param timestamp временная метка
                 * \param time_indent_dn максимальный отступ до временной метки
                 * \param time_indent_up максимальный отступ после временной метки
                 * \param list_news список новостей
                 * \return вернет 0 в случае успеха
                 */
                int get_news(
                        unsigned long long timestamp,
                        unsigned long long time_indent_dn,
                        unsigned long long time_indent_up,
                        std::vector<News> &list_news)
                {
                        if(list_news_.size() == 0) return NO_ACCESS_DATA;
                        unsigned long long start_time = timestamp - time_indent_dn;
                        unsigned long long stop_time = timestamp + time_indent_up;
                        auto lower = std::lower_bound(list_news_.begin(), list_news_.end(), start_time, [](const News &lhs, unsigned long long rhs) {
                                return lhs.timestamp < rhs;
                        });
                        auto upper = std::upper_bound(list_news_.begin(), list_news_.end(), stop_time, [](unsigned long long lhs, const News &rhs) {
                                return lhs < rhs.timestamp;
                        });

                        if(lower == list_news_.begin()) {
                                std::cout << "lower == list_news_.begin()" << std::endl;
                        }
                        if(upper == list_news_.begin()) {
                                std::cout << "upper == list_news_.begin()" << std::endl;
                        }
                        if(lower == list_news_.end()) {
                                std::cout << "lower == list_news_.end()" << std::endl;
                        }
                        if(upper == list_news_.end()) {
                                std::cout << "upper == list_news_.end()" << std::endl;
                        }


                        if(lower == list_news_.end() && upper == list_news_.end()) {
                                return NO_ACCESS_DATA;
                        }
                        if(lower == list_news_.begin() && upper == list_news_.begin()) {
                                return NO_ACCESS_DATA;
                        }
                        list_news.clear();
                        list_news.insert(list_news.begin(), lower, upper);
                        return OK;
                }
//------------------------------------------------------------------------------
                /** \brief Получить новости по временной метке
                 * \param timestamp временная метка
                 * \param time_indent_dn максимальный отступ до временной метки
                 * \param time_indent_up максимальный отступ после временной метки
                 * \param list_news список новостей
                 * \param time_diff разница во времени между временной меткой и началом новости
                 * \return вернет 0 в случае успеха
                 */
                int get_news(
                        unsigned long long timestamp,
                        unsigned long long time_indent_dn,
                        unsigned long long time_indent_up,
                        std::vector<News> &list_news,
                        std::vector<long> &time_diff)
                {
                        int err = get_news(timestamp, time_indent_dn, time_indent_up, list_news);
                        if(err != OK)
                                return err;
                        time_diff.resize(list_news.size());
                        for(size_t i = 0; i < list_news.size(); ++i) {
                                time_diff[i] = (long long)list_news[i].timestamp - (long long)timestamp;
                        }
                        return OK;
                }
//------------------------------------------------------------------------------
        };
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
}

#endif // FOREXPROSTOOLSAPIEASY_HPP_INCLUDED
