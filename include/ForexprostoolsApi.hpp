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
#ifndef FOREXPROSTOOLSAPI_HPP_INCLUDED
#define FOREXPROSTOOLSAPI_HPP_INCLUDED
//------------------------------------------------------------------------------
#include <curl/curl.h>
#include <gzip/decompress.hpp>
#include <nlohmann/json.hpp>
#include <xtime.hpp>
#include <thread>
#include <string>
#include <vector>
#include <ForexprostoolsApiEasy.hpp>
//------------------------------------------------------------------------------
class ForexprostoolsApi
{
public:
//------------------------------------------------------------------------------
        enum ErrorType {
                OK = 0,
                NO_INIT = -1,
                INIT_ERROR = -2,
                DECOMPRESSION_ERROR = -3,
                PARSER_ERROR = -4,
        };
//------------------------------------------------------------------------------
private:
        bool is_curl_global_init_error_ = false;
        const int MAX_NUM_ATTEMPT = 10;
        const std::string sert_file_;                           /**< Имя файла сертефиката */
        std::vector<ForexprostoolsApiEasy::News> list_news_;    /**< Список новостей */
        std::mutex list_news_mutex_;
//------------------------------------------------------------------------------
        std::string get_request_body(unsigned long long beg_timestamp, unsigned long long end_timestamp, std::vector<std::string> countrys = std::vector<std::string>)
        {
                xtime::DateTime start_time(beg_timestamp), stop_time(end_timestamp);
                std::string request_body =
                "dateFrom=" + std::to_string(start_time.year) + "-" + std::to_string(start_time.month) + "-" + std::to_string(start_time.day) + "&"
                "dateTo=" + std::to_string(stop_time.year) + "-" + std::to_string(stop_time.month) + "-" + std::to_string(stop_time.day) + "&"
                "timeframe=&"
                "columns[]=exc_flags&"
                "columns[]=exc_currency&"
                "columns[]=exc_importance&"
                "columns[]=exc_actual&"
                "columns[]=exc_forecast&"
                "columns[]=exc_previous&"
                "timeZone=55&" // время GMT
                "quotes_search_text=&";
                if(countrys.size() == 0) {
                        request_body +=
                                "country[]=4&country[]=5&country[]=6&country[]=7&"
                                "country[]=9&country[]=10&country[]=11&country[]=12&"
                                "country[]=14&country[]=15&country[]=17&country[]=20&"
                                "country[]=21&country[]=22&country[]=23&country[]=24&"
                                "country[]=25&country[]=26&country[]=27&country[]=29&"
                                "country[]=32&country[]=33&country[]=34&country[]=35&"
                                "country[]=36&country[]=37&country[]=38&country[]=39&"
                                "country[]=41&country[]=42&country[]=43&country[]=44&"
                                "country[]=45&country[]=46&country[]=47&country[]=48&"
                                "country[]=51&country[]=52&country[]=53&country[]=54&"
                                "country[]=55&country[]=56&country[]=57&country[]=59&"
                                "country[]=60&country[]=61&country[]=63&country[]=66&"
                                "country[]=68&country[]=70&country[]=71&country[]=72&"
                                "country[]=75&country[]=78&country[]=80&country[]=84&"
                                "country[]=85&country[]=87&country[]=89&country[]=90&"
                                "country[]=92&country[]=93&country[]=94&country[]=96&"
                                "country[]=97&country[]=100&country[]=102&country[]=103&"
                                "country[]=105&country[]=106&country[]=107&country[]=109&"
                                "country[]=110&country[]=111&country[]=112&country[]=113&"
                                "country[]=119&country[]=121&country[]=122&country[]=123&"
                                "country[]=125&country[]=138&country[]=139&country[]=143&"
                                "country[]=145&country[]=162&country[]=163&country[]=170&"
                                "country[]=172&country[]=174&country[]=178&country[]=188&"
                                "country[]=193&country[]=202&country[]=238&country[]=247&";
                } else {
                        for(size_t i = 0; i < countrys.size(); ++i) {
                                request_body += "country[]=" + std::to_string(countrys[i]) + "&";
                        }
                }
                request_body +=
                        "timeFilter=timeOnly&" // фильтр, только время
                        "action=filter&"
                        "lang=1"; // язык EN
                return request_body;
        }
//------------------------------------------------------------------------------
        int parse_response(std::string response, std::vector<ForexprostoolsApiEasy::News> &list_news)
        {
                using json = nlohmann::json;
                try {
                        json j;
                        j = json::parse(response);
                        std::string text = j["renderedFilteredEvents"];
                        const std::string header_beg = "<tr";
                        const std::string header_end = "</tr>";
                        std::size_t start_data_pos = 0;
                        while(true) {
                                ForexprostoolsApiEasy::News one_news;
                                const int STATE_TIME = 0x01;
                                const int STATE_NAME = 0x02;
                                const int STATE_VOL = 0x04;
                                const int STATE_DATA = 0x08;
                                const int STATE_OK = 0x0F;
                                int state = 0;
                                std::size_t beg_pos = text.find(header_beg, start_data_pos);
                                std::size_t end_pos = text.find(header_end, start_data_pos);
                                if(beg_pos != std::string::npos &&
                                        end_pos != std::string::npos) {
                                        std::string part = text.substr(beg_pos, end_pos - beg_pos);
                                        start_data_pos = end_pos + header_end.size();
                                        // парсим part
                                        const std::string str_event_timestamp = "event_timestamp=";
                                        std::size_t event_timestamp_pos = part.find(str_event_timestamp, 0);
                                        if(event_timestamp_pos != std::string::npos) {
                                                const std::string str_div_timestamp = "\"";
                                                std::size_t beg_pos = part.find(str_div_timestamp, event_timestamp_pos);
                                                if(beg_pos != std::string::npos) {
                                                        std::size_t end_pos = part.find(str_div_timestamp, beg_pos + 1);
                                                        if(end_pos != std::string::npos) {
                                                                std::string str_time = part.substr(beg_pos + 1, end_pos - beg_pos - 1);
                                                                xtime::convert_str_to_timestamp(str_time, one_news.timestamp);
                                                                state |= STATE_TIME;
                                                                //std::cout << xtime::get_str_unix_date_time(timestamp) << std::endl;
                                                        } // if
                                                } // if
                                        } else {
                                                continue;
                                        }
                                        // ищем значения
                                        const std::string str_event_actual = "eventActual_";
                                        const std::string str_event_forecast = "eventForecast_";
                                        const std::string str_event_previous = "eventPrevious_";

                                        std::size_t event_actual_pos = part.find(str_event_actual, 0);
                                        std::size_t event_forecast_pos = part.find(str_event_forecast, 0);
                                        std::size_t event_previous_pos = part.find(str_event_previous, 0);

                                        const std::string str_div_beg = ">";
                                        const std::string str_div_end = "<";

                                        if(event_previous_pos != std::string::npos) {
                                                std::size_t beg_pos = part.find(str_div_beg, event_previous_pos);
                                                if(beg_pos != std::string::npos) {
                                                        std::size_t end_pos = part.find(str_div_end, beg_pos + 1);
                                                        if(end_pos != std::string::npos) {
                                                                std::string str_previous = part.substr(beg_pos + 1, end_pos - beg_pos - 1);
                                                                //
                                                                const std::string str_nbsp = "&nbsp;";
                                                                std::size_t nbsp_pos = str_previous.find(str_nbsp, 0);
                                                                if(nbsp_pos == std::string::npos) {
                                                                        one_news.previous = atof(str_previous.c_str());
                                                                        one_news.is_previous = true;
                                                                        state |= STATE_DATA;
                                                                }
                                                                //std::cout << "previous " << str_previous << std::endl;
                                                        } // if
                                                } // if
                                        }
                                        if(event_actual_pos != std::string::npos) {
                                                std::size_t beg_pos = part.find(str_div_beg, event_actual_pos);
                                                if(beg_pos != std::string::npos) {
                                                        std::size_t end_pos = part.find(str_div_end, beg_pos + 1);
                                                        if(end_pos != std::string::npos) {
                                                                std::string str_actual = part.substr(beg_pos + 1, end_pos - beg_pos - 1);
                                                                //
                                                                const std::string str_nbsp = "&nbsp;";
                                                                std::size_t nbsp_pos = str_actual.find(str_nbsp, 0);
                                                                if(nbsp_pos == std::string::npos) {
                                                                        one_news.actual = atof(str_actual.c_str());
                                                                        one_news.is_actual = true;
                                                                        state |= STATE_DATA;
                                                                }
                                                                //std::cout << "actual " << str_actual << std::endl;
                                                        } // if
                                                } // if
                                        }
                                        if(event_forecast_pos != std::string::npos) {
                                                std::size_t beg_pos = part.find(str_div_beg, event_forecast_pos);
                                                if(beg_pos != std::string::npos) {
                                                        std::size_t end_pos = part.find(str_div_end, beg_pos + 1);
                                                        if(end_pos != std::string::npos) {
                                                                std::string str_forecast = part.substr(beg_pos + 1, end_pos - beg_pos - 1);
                                                                //
                                                                const std::string str_nbsp = "&nbsp;";
                                                                std::size_t nbsp_pos = str_forecast.find(str_nbsp, 0);
                                                                if(nbsp_pos == std::string::npos) {
                                                                        one_news.forecast = atof(str_forecast.c_str());
                                                                        one_news.is_forecast = true;
                                                                        state |= STATE_DATA;
                                                                }
                                                                //std::cout << "forecast " << str_forecast << std::endl;
                                                        } // if
                                                } // if
                                        }
                                        // определяем волатильность новости
                                        const std::string str_sentiment_div_beg = "<td class=\"left textNum sentiment noWrap\" title=\"";
                                        const std::string str_sentiment_div_end = "\"";
                                        std::size_t sentiment_beg_pos = part.find(str_sentiment_div_beg, 0);
                                        if(sentiment_beg_pos != std::string::npos) {
                                                std::size_t sentiment_end_pos = part.find(str_sentiment_div_end, sentiment_beg_pos + str_sentiment_div_beg.size());
                                                if(sentiment_end_pos != std::string::npos) {
                                                        std::string str_sentiment = part.substr(sentiment_beg_pos + str_sentiment_div_beg.size(), sentiment_end_pos - sentiment_beg_pos - str_sentiment_div_beg.size());
                                                        const std::string str_low = "Low";
                                                        const std::string str_moderate = "Moderate";
                                                        const std::string str_high = "High";
                                                        if(str_sentiment.find(str_low, 0) != std::string::npos) {
                                                                one_news.level_volatility = ForexprostoolsApiEasy::LOW;
                                                                state |= STATE_VOL;
                                                                //std::cout << "sentiment " << str_low << std::endl;
                                                        } else
                                                        if(str_sentiment.find(str_moderate, 0) != std::string::npos) {
                                                                one_news.level_volatility = ForexprostoolsApiEasy::MODERATE;
                                                                state |= STATE_VOL;
                                                                //std::cout << "sentiment " << str_moderate << std::endl;
                                                        } else
                                                        if(str_sentiment.find(str_high, 0) != std::string::npos) {
                                                                one_news.level_volatility = ForexprostoolsApiEasy::HIGH;
                                                                state |= STATE_VOL;
                                                                //std::cout << "sentiment " << str_high << std::endl;
                                                        }
                                                } // if
                                        } // if

                                        // определяем имя новости
                                        const std::string str_left_event_div_beg = "<td class=\"left event\">";
                                        const std::string str_left_event_div_end = "<";
                                        std::size_t left_event_beg_pos = part.find(str_left_event_div_beg, 0);
                                        if(left_event_beg_pos != std::string::npos) {
                                                std::size_t left_event_end_pos = part.find(str_left_event_div_end, left_event_beg_pos + str_left_event_div_beg.size());
                                                if(left_event_end_pos != std::string::npos) {
                                                        std::string str_left_event = part.substr(left_event_beg_pos + str_left_event_div_beg.size(), left_event_end_pos - left_event_beg_pos - str_left_event_div_beg.size());
                                                        // удаляем слово &nbsp; если оно есть
                                                        const std::string str_nbsp = "&nbsp;";
                                                        std::size_t nbsp_pos = str_left_event.find(str_nbsp, 0);
                                                        if(nbsp_pos != std::string::npos) {
                                                                str_left_event.erase(nbsp_pos, str_nbsp.length());
                                                        }
                                                        // удаляем лишние пробелы и прочее
                                                        str_left_event.erase(std::remove_if(str_left_event.begin(), str_left_event.end(), [](char c){
                                                                return c == '\t' || c == '\v' || c == '\n' || c == '\r';
                                                                }), str_left_event.end());

                                                        while(str_left_event.size() > 0 && std::isspace(str_left_event[0])) {
                                                                str_left_event.erase(str_left_event.begin());
                                                        }
                                                        while(str_left_event.size() > 0 && std::isspace(str_left_event.back())) {
                                                                str_left_event.erase(str_left_event.end() - 1);
                                                        }
                                                        str_left_event.erase(std::unique_copy(str_left_event.begin(), str_left_event.end(), str_left_event.begin(),
                                                                [](char c1, char c2){
                                                                        return std::isspace(c1) && std::isspace(c2);
                                                                }),
                                                                str_left_event.end());
                                                        one_news.name = str_left_event;
                                                        state |= STATE_NAME;
                                                        //std::cout << "left_event " << str_left_event << std::endl;
                                                }
                                        }

                                        // определяем страну валюты
                                        const std::string str_flag_div_beg = "<td class=\"left flagCur noWrap\">";
                                        const std::string str_flag_div_end = "</td>";
                                        std::size_t flag_beg_pos = part.find(str_flag_div_beg, 0);
                                        if(flag_beg_pos != std::string::npos) {
                                                std::size_t flag_end_pos = part.find(str_flag_div_end, flag_beg_pos + str_flag_div_beg.size());
                                                if(flag_end_pos != std::string::npos) {
                                                        const std::string str_title = "title=";
                                                        const std::string str_currency_beg = "</span>";

                                                        std::size_t title_pos = part.find(str_title, flag_beg_pos + str_flag_div_beg.size());
                                                        if(title_pos != std::string::npos) {
                                                                const std::string str_div_timestamp = "\"";
                                                                std::size_t beg_pos = part.find(str_div_timestamp, title_pos);
                                                                if(beg_pos != std::string::npos) {
                                                                        std::size_t end_pos = part.find(str_div_timestamp, beg_pos + 1);
                                                                        if(end_pos != std::string::npos) {
                                                                                // имя страны
                                                                                one_news.country = part.substr(beg_pos + 1, end_pos - beg_pos - 1);
                                                                                //std::cout << str_title << std::endl;
                                                                        } // if
                                                                } // if
                                                        }

                                                        std::size_t currency_pos = part.find(str_currency_beg, flag_beg_pos + str_flag_div_beg.size());
                                                        if(currency_pos != std::string::npos) {
                                                                // имя валюты
                                                                std::string str_currency = part.substr(currency_pos + str_currency_beg.size(), flag_end_pos - currency_pos - str_currency_beg.size());
                                                                str_currency.erase(std::remove_if(str_currency.begin(), str_currency.end(), ::isspace), str_currency.end());
                                                                one_news.currency = str_currency;
                                                                //std::cout << str_currency << std::endl;
                                                        }
                                                } // if
                                        } // if
                                        if(state == STATE_OK) {
                                                list_news.push_back(one_news);
                                        }
                                } else {
                                        break;
                                }
                        } // while
                } //
                catch(...) {
                        return PARSER_ERROR;
                }
                return OK;
        }
//------------------------------------------------------------------------------
        inline int writer(char *data, size_t size, size_t nmemb, std::string *buffer)
        {
                int result = 0;
                if (buffer != NULL) {
                        buffer->append(data, size * nmemb);
                        result = size * nmemb;
                }
                return result;
        }
//------------------------------------------------------------------------------
        int do_post_request(std::string request_body, std::string &out, std::string sert_file)
        {
                CURL *curl;
                curl = curl_easy_init();
                if(!curl) {
                        return INIT_ERROR;
                }
                const std::string url = ("https://sslecal2.forexprostools.com/ajax.php");
                char error_buffer[CURL_ERROR_SIZE];
                const int TIME_OUT = 60;
                std::string buffer;

                curl_easy_setopt(curl, CURLOPT_POST, 1); // делаем пост запрос
                curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                curl_easy_setopt(curl, CURLOPT_CAINFO, sert_file.c_str());
                curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error_buffer);
                curl_easy_setopt(curl, CURLOPT_HEADER, 0); // отключаем заголовок в ответе
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

                struct curl_slist *http_headers = NULL;
                http_headers = curl_slist_append(http_headers, "Host: sslecal2.forexprostools.com");
                http_headers = curl_slist_append(http_headers, "Accept: application/json, text/javascript, */*; q=0.01");
                http_headers = curl_slist_append(http_headers, "Accept-Language: ru-RU,ru;q=0.8,en-US;q=0.5,en;q=0.3");
                http_headers = curl_slist_append(http_headers, "Accept-Encoding: gzip, deflate, br");
                http_headers = curl_slist_append(http_headers, "Content-Type: application/x-www-form-urlencoded; charset=UTF-8");
                http_headers = curl_slist_append(http_headers, "X-Requested-With: XMLHttpRequest");
                http_headers = curl_slist_append(http_headers, "Connection: keep-alive");
                http_headers = curl_slist_append(http_headers, "Cache-Control: no-cache");
                http_headers = curl_slist_append(http_headers, "Pragma: no-cache");
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, http_headers);
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_body.c_str());
                curl_easy_setopt(curl, CURLOPT_TIMEOUT, TIME_OUT);

                CURLcode result;
                result = curl_easy_perform(curl);
                curl_slist_free_all(http_headers);
                http_headers = NULL;

                curl_easy_cleanup(curl);
                if (result == CURLE_OK) {
# if (0)
                        std::cout << buffer << std::endl;
                        std::cout << "size: " << buffer.size();
# endif
                        try {
                                const char *compressed_pointer = buffer.data();
                                out = gzip::decompress(compressed_pointer, buffer.size());
                        }
                        catch(...) {
                                return DECOMPRESSION_ERROR;
                        }
                        return OK;
                } else {
                        std::cout << "Error: [" << result << "] - " << error_buffer;
                        return result;
                }
        }
//------------------------------------------------------------------------------
public:
//------------------------------------------------------------------------------
        enum CounryCode {
                Italy = 10,
                South_Korea = 11,
                Germany = 17,
                France = 22,
                Australia = 25,
                Spain = 26,
                Brazil = 32,
                Japan = 35,
                Singapore = 36,
                China = 37,
                Austria = 54,
                Russia = 56,
                Euro_Zone = 72,
                United_Kingdom = 4,
                United_States = 5,
                Canada = 6,
                South_Africa = 110,
                Hong_Kong = 39,
                New_Zealand = 43,
                India = 14,
                Switzerland = 12,
        };
//------------------------------------------------------------------------------
        ForexprostoolsApi(std::string sert_file = "curl-ca-bundle.crt")
        {
                /* Должен инициализировать libcurl до запуска любых потоков */
                if(curl_global_init(CURL_GLOBAL_ALL) !=0) {
                        is_curl_global_init_error_ = true;
                }
                std::string sert_file_ = sert_file;
        }
//------------------------------------------------------------------------------
        /** \brief
         *
         * \param beg_timestamp начальная дата новостей
         * \param end_timestamp конечная дата новостей
         * \param is_wait ждать ответа сервера
         * \return
         *
         */
        int download_all_news(unsigned long long beg_timestamp, unsigned long long end_timestamp, bool is_wait = true)
        {
                if(is_curl_global_init_error_)
                        return NO_INIT;
                std::thread curl_thread([&, beg_timestamp, end_timestamp]() {
                        std::string request_body = get_request_body(beg_timestamp, end_timestamp);
                        std::string response;
                        if(do_post_request(request_body, response, sert_file_) == OK) {

                                if(parse_response(response, list_news_) == OK) {

                                }

                        }
                });
                if(is_wait) {
                        curl_thread.join();
                } else {
                        curl_thread.detach();
                }
        }
//------------------------------------------------------------------------------
        int get_news(unsigned long long beg_timestamp, unsigned long long end_timestamp)
        {
                if(is_curl_global_init_error_)
                        return NO_INIT;
                std::thread curl_thread([&, beg_timestamp, end_timestamp]() {
                        int num_attempt = MAX_NUM_ATTEMPT;
                        while(num_attempt > 0) {
                                num_attempt--;
                                CURL *curl;
                                curl = curl_easy_init();
                                CURLcode result;
                                if(!curl) {
                                        std::cout << "cant init curl. exit" << std::endl;
                                        continue;
                                }
                                curl_easy_setopt(curl, CURLOPT_CAINFO, sert_file_.c_str());
                                curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);
                                curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                                curl_easy_setopt(curl, CURLOPT_POST, 1);

                                curl_easy_setopt(curl, CURLOPT_HEADER, 0);
                                //curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
                                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);
                                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
                                // Headers
                                struct curl_slist *httpHeaders = NULL;
                                httpHeaders = curl_slist_append(httpHeaders, "Host: sslecal2.forexprostools.com");
                                //httpHeaders = curl_slist_append(httpHeaders, "User-Agent: Mozilla/5.0 (Windows NT 6.3; Win64; x64; rv:63.0) Gecko/20100101 Firefox/63.0");
                                httpHeaders = curl_slist_append(httpHeaders, "Accept: application/json, text/javascript, */*; q=0.01");
                                httpHeaders = curl_slist_append(httpHeaders, "Accept-Language: ru-RU,ru;q=0.8,en-US;q=0.5,en;q=0.3");
                                httpHeaders = curl_slist_append(httpHeaders, "Accept-Encoding: gzip, deflate, br");
                                httpHeaders = curl_slist_append(httpHeaders, "Referer: https://sslecal2.forexprostools.com/?columns=exc_flags,exc_currency,exc_importance,exc_actual,exc_forecast,exc_previous&category=_employment,_economicActivity,_inflation,_credit,_centralBanks,_confidenceIndex,_balance,_Bonds&importance=3&features=datepicker,timezone,timeselector,filters&countries=25,32,6,37,72,22,17,39,14,10,35,43,56,36,110,11,26,12,4,5&calType=week&timeZone=18&lang=7");
                                httpHeaders = curl_slist_append(httpHeaders, "Content-Type: application/x-www-form-urlencoded; charset=UTF-8");
                                httpHeaders = curl_slist_append(httpHeaders, "X-Requested-With: XMLHttpRequest");
                                //httpHeaders = curl_slist_append(httpHeaders, content_length.c_str());
                                httpHeaders = curl_slist_append(httpHeaders, "Content-Length: 620");
                                httpHeaders = curl_slist_append(httpHeaders, "Connection: keep-alive");
                                httpHeaders = curl_slist_append(httpHeaders, "Cache-Control: no-cache");
                                httpHeaders = curl_slist_append(httpHeaders, "Pragma: no-cache");
                        }
                });
                curl_thread.detach();
        }
}

#endif // FOREXPROSTOOLSAPI_HPP_INCLUDED
