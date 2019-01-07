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

#include <unordered_map>
#include <vector>
#include <string>
//------------------------------------------------------------------------------
namespace ForexprostoolsApiEasy
{
//------------------------------------------------------------------------------
        enum ErrorType {
                OK = 0,
                NO_ACCESS_DATA = -1,
        };
//------------------------------------------------------------------------------
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
                double previous;                        /**< Предыдущее значение */
                double actual;                          /**< Актуальное значение */
                double forecast;                        /**< Предсказанное значение */
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
                 * \param min_time_diff_dn отступ до начала новостей
                 * \param min_time_diff_up отступ после начала новостей
                 * \param list_news список новостей
                 * \return вернет 0 в случае успеха
                 */
                int get_news(
                        unsigned long long timestamp,
                        unsigned long long min_time_diff_dn,
                        unsigned long long min_time_diff_up,
                        std::vector<News> &list_news)
                {
                        auto lower = std::lower_bound(list_news_.begin(), list_news_.end(), timestamp - min_time_diff_dn, [](const News &lhs, double rhs) {
                                return lhs.timestamp < rhs;
                        });
                        auto upper = std::upper_bound(list_news_.begin(), list_news_.end(), timestamp + min_time_diff_up, [](double lhs, const News &rhs) {
                                return lhs < rhs.timestamp;
                        });
                        if(lower == list_news_.end() || upper == list_news_.end()) {
                                return NO_ACCESS_DATA;
                        }
                        list_news.clear();
                        list_news.insert(list_news.begin(), lower, upper);
                        return OK;
                }

//------------------------------------------------------------------------------
                /** \brief Получить новости по временной метке
                 * \param timestamp временная метка
                 * \param min_time_diff_dn отступ до начала новостей
                 * \param min_time_diff_up отступ после начала новостей
                 * \param list_news список новостей
                 * \param time_diff разница во времени между временной меткой и началом новости
                 * \return вернет 0 в случае успеха
                 */
                int get_news(
                        unsigned long long timestamp,
                        unsigned long long min_time_diff_dn,
                        unsigned long long min_time_diff_up,
                        std::vector<News> &list_news,
                        std::vector<long> &time_diff)
                {
                        int err = get_news(timestamp, min_time_diff_dn, min_time_diff_up, list_news);
                        if(err != OK)
                                return err;
                        time_diff.resize(list_news.size());
                        for(size_t i = 0; i < list_news.size(); ++i) {
                                time_diff[i] = (long long)list_news[i].timestamp - (long long)timestamp;
                        }
                }
//------------------------------------------------------------------------------
        };
//------------------------------------------------------------------------------
}

#endif // FOREXPROSTOOLSAPIEASY_HPP_INCLUDED
