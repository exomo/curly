/**************************************************************************
 *  Copyright 2016 Kai Bublitz (exomo)                                    *
 *                                                                        *
 *  This file is part of the Curly library.                                           *
 *                                                                        *
 *  Curly is free software: you can redistribute it and/or modify         *
 *  it under the terms of the GNU General Public License as published by  *
 *  the Free Software Foundation, either version 3 of the License, or     *
 *  (at your option) any later version.                                   *
 *                                                                        *
 *  Curly is distributed in the hope that it will be useful,              *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *  GNU General Public License for more details.                          *
 *                                                                        *
 *  You should have received a copy of the GNU General Public License     *
 *  along with Curly.  If not, see <http://www.gnu.org/licenses/>.        *
 *                                                                        *
 **************************************************************************/

#ifndef CURLY_H_INCLUDED
#define CURLY_H_INCLUDED

#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

// forward declaration of CURL class, definition not required for this interface
typedef void CURL;

namespace curly
{

class CurlyException
{
    public:
        std::string Message;
        CurlyException(std::string Msg) : Message(Msg)
        {

        }
};

enum UserAgent
{
    DEFAULT = 0,
    FIREFOX,
    INTERNET_EXPLORER,
    OPERA
};

class CurlyObserver
{
public:
	virtual void download_update( double dltotal, double dlnow ) = 0;
};

class Curly
{
public:
    Curly();
    ~Curly();

    // delete copy constructor and assignment
    Curly( const Curly &c ) = delete;
    Curly & operator=(const Curly &c ) = delete;

    void setBaseUrl(std::string Url);

    std::string getPage(std::string url);
    std::string getPage(std::string url, std::string postdata);
    bool getFile(std::string url, std::string filename);
    bool getFile(std::string url, std::string filename, std::string postdata);

    void setUserAgent(UserAgent agent);
    void setUserAgent(std::string agent);

    void setLocalPort(long port);

    void refreshReferer();

    void setCookie(std::string name, std::string value);
    std::string getCookie(std::string name);
    std::vector<std::string> getCookieNames();
    void clearCookies();
    void saveCookies(std::string filename);
    void loadCookies(std::string filename, bool keepExisting=true);

    void setObserver( CurlyObserver *obs );

    static std::string urlEncode(std::string);


private:
    std::string baseUrl;
    std::string userAgent;
    std::string referer;
    long localPort;
    bool refRefresh;

    CURL* curl;
    //std::string page;
    std::ofstream fileout;
    std::ostringstream stringout;

    std::ostream *out;
    CurlyObserver *observer = nullptr;

    static size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp);
    static int download_progress(void *userp, double dltotal, double dlnow, double ultotal, double ulnow);

    std::map<std::string, std::string> cookies;
    std::string getCookieString();
// TODO (exomo#1#): Cookie Management

};

} // end namespace

#endif // CURLY_H_INCLUDED
