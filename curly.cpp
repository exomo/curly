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

#include "curly.h"

#include <curl/curl.h>
#include <ostream>

namespace curly
{

Curly::Curly()
    : localPort(0)
{
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

    curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, download_progress);
    curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, this);

    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

    curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "-");

    //curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");
    //curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

    refRefresh = true;
}

//Curly::Curly( Curly&& oc )
//{
//	curl = oc.curl;
//	out = nullptr;
//	cookies = std::move( oc.cookies );
//}

Curly::~Curly()
{
    curl_easy_cleanup(curl);
}

size_t Curly::write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
    Curly* inst = (Curly*) userp;

    char *cbuffer = (char*) buffer;

    inst->out->write( cbuffer, size * nmemb );

    return size * nmemb;
}

int Curly::download_progress(void *userp, double dltotal, double dlnow, double ultotal, double ulnow)
{
	Curly* inst = (Curly*) userp;
	if( inst->observer != nullptr ) inst->observer->download_update( dltotal, dlnow );

	return 0;
}

void Curly::setBaseUrl(std::string Url)
{
    baseUrl = Url;
}

std::string Curly::getPage(std::string url)
{
    //page.clear();
    stringout.str("");

    out = &stringout;

    //std::cout << "curly get page " << url <<std::endl;


    curl_slist *header = NULL;
    // curl_slist_append(header, "Referer: http://www.meinemafia.de/Blue-Gangs");
    if(userAgent != "") curl_slist_append(header, ("User-Agent: " + userAgent).c_str());
    if(referer != "") curl_slist_append(header, ("Referer: " + referer).c_str() );
    std::string cookieString = getCookieString();
    curl_easy_setopt(curl, CURLOPT_COOKIE, cookieString.c_str());
    curl_easy_setopt(curl, CURLOPT_LOCALPORT, localPort);
    // std::cout << "angefordert: " << (baseUrl+url) << std::endl;
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);

    curl_easy_setopt(curl, CURLOPT_URL, (baseUrl+url).c_str());
    curl_easy_perform(curl);

    curl_slist_free_all(header);

    if(refRefresh)
    {
        referer = url;
        refRefresh = false;
    }

    return stringout.str();
}

std::string Curly::getPage(std::string url, std::string postdata)
{
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postdata.c_str());
    std::string response = getPage(url);
    curl_easy_setopt(curl, CURLOPT_HTTPGET, (long)1);
    return response;
}

bool Curly::getFile(std::string url, std::string filename)
{
	fileout.open(filename.c_str(), std::ios::binary);
	if(!fileout)
	{
		return false;
	}
	out = &fileout;

    curl_slist *header = NULL;
    // curl_slist_append(header, "Referer: http://www.meinemafia.de/Blue-Gangs");
    if(userAgent != "") curl_slist_append(header, ("User-Agent: " + userAgent).c_str());
    if(referer != "") curl_slist_append(header, ("Referer: " + referer).c_str() );
    std::string cookieString = getCookieString();
    curl_easy_setopt(curl, CURLOPT_COOKIE, cookieString.c_str());

    // std::cout << "angefordert: " << (baseUrl+url) << std::endl;
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
    curl_easy_setopt(curl, CURLOPT_URL, (baseUrl+url).c_str());

    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0 );

    curl_easy_perform(curl);

    fileout.close();

    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1 );
    curl_slist_free_all(header);


    return true;
}

bool Curly::getFile(std::string url, std::string filename, std::string postdata)
{
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postdata.c_str());
    bool result = getFile(url, filename);
    curl_easy_setopt(curl, CURLOPT_HTTPGET, (long)1);
    return result;
}

std::string Curly::urlEncode(std::string parameter)
{
    char* encoded = curl_easy_escape(0, parameter.c_str(), parameter.length());

    std::string result = encoded;
    curl_free(encoded);
    return result;
}

void Curly::refreshReferer()
{
    refRefresh = true;
}
void Curly::setUserAgent(UserAgent agent)
{
    switch(agent)
    {
        case DEFAULT:
            setUserAgent("");
            break;

        case FIREFOX:
            setUserAgent("Mozilla/5.0 (Windows; U; Windows NT 6.1; de; rv:1.9.2.10) Gecko/20100914 Firefox/3.6.10");
            break;

        default:
            setUserAgent("");
            break;
    }
}

void Curly::setUserAgent(std::string agent)
{
    userAgent = agent;
}

void Curly::setLocalPort(long port)
{
    localPort = port;
}

void Curly::setObserver( CurlyObserver *obs )
{
	observer = obs;
}

void Curly::setCookie(std::string name, std::string value)
{
    cookies[name] = value;
}

std::string Curly::getCookie(std::string name)
{
    return cookies[name];
}

std::vector<std::string> Curly::getCookieNames()
{
	std::vector<std::string> cookieNames;
	return cookieNames;
}

void Curly::clearCookies()
{
	cookies.clear();
}


void Curly::saveCookies(std::string filename)
{


}

void Curly::loadCookies(std::string filename, bool keepExisting)
{

}

std::string Curly::getCookieString()
{
    // TODO (kai#1#): look at value encoding
    std::string str = "";

    bool first = true;
    for(std::map<std::string, std::string>::iterator it = cookies.begin(); it != cookies.end(); ++it)
    {
        if(!first) str+="; ";
        str+= it->first + "=" + it->second;
        first = false;
    }
    return str;
}

} // end namespace
