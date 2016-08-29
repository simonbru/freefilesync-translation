// *****************************************************************************
// * This file is part of the FreeFileSync project. It is distributed under    *
// * GNU General Public License: http://www.gnu.org/licenses/gpl-3.0           *
// * Copyright (C) Zenju (zenju AT freefilesync DOT org) - All Rights Reserved *
// *****************************************************************************

#include "http.h"
#ifdef ZEN_WIN
    #include <zen/win.h> //tame wininet.h include
    #include <wininet.h>
#endif

#if defined ZEN_LINUX || defined ZEN_MAC
    #include <zen/thread.h> //std::thread::id
    #include <wx/protocol/http.h>
#endif

using namespace zen;


namespace
{
#ifdef ZEN_WIN
    #if defined NDEBUG && defined __WXWINDOWS__
        #error don not use wxWidgets for this component!
    #endif
#else
    #ifndef NDEBUG
        const std::thread::id mainThreadId = std::this_thread::get_id();
    #endif
#endif


std::string sendHttpRequestImpl(const std::wstring& url, //throw FileError
                                const std::wstring& userAgent,
                                const std::string* postParams, //issue POST if bound, GET otherwise
                                int level = 0)
{
    assert(!startsWith(makeUpperCopy(url), L"HTTPS:")); //not supported by wxHTTP!
    const std::wstring urlFmt = startsWith(makeUpperCopy(url), L"HTTP://") ? afterFirst(url, L"://", IF_MISSING_RETURN_NONE) : url;
    const std::wstring server =       beforeFirst(urlFmt, L'/', IF_MISSING_RETURN_ALL);
    const std::wstring page   = L'/' + afterFirst(urlFmt, L'/', IF_MISSING_RETURN_NONE);

#ifdef ZEN_WIN
    //WinInet: 1. uses IE proxy settings! :) 2. follows HTTP redirects by default 3. swallows HTTPS if needed
    HINTERNET hInternet = ::InternetOpen(userAgent.c_str(),            //_In_  LPCTSTR lpszAgent,
                                         INTERNET_OPEN_TYPE_PRECONFIG, //_In_  DWORD dwAccessType,
                                         nullptr, //_In_  LPCTSTR lpszProxyName,
                                         nullptr, //_In_  LPCTSTR lpszProxyBypass,
                                         0);      //_In_  DWORD dwFlags
    if (!hInternet)
        THROW_LAST_FILE_ERROR(_("Internet access failed."), L"InternetOpen");
    ZEN_ON_SCOPE_EXIT(::InternetCloseHandle(hInternet));

    HINTERNET hSession = ::InternetConnect(hInternet,      //_In_ HINTERNET     hInternet,
                                           server.c_str(), //_In_ LPCTSTR       lpszServerName,
                                           INTERNET_DEFAULT_HTTP_PORT, //_In_ INTERNET_PORT nServerPort,
                                           nullptr,        //_In_ LPCTSTR       lpszUsername,
                                           nullptr,        //_In_ LPCTSTR       lpszPassword,
                                           INTERNET_SERVICE_HTTP, //_In_ DWORD         dwService,
                                           0,              //_In_ DWORD         dwFlags,
                                           0);             //_In_ DWORD_PTR     dwContext
    if (!hSession)
        THROW_LAST_FILE_ERROR(_("Internet access failed."), L"InternetConnect");
    ZEN_ON_SCOPE_EXIT(::InternetCloseHandle(hSession));

    const wchar_t* acceptTypes[] = { L"*/*", nullptr };
    DWORD requestFlags = INTERNET_FLAG_KEEP_CONNECTION |
                         INTERNET_FLAG_NO_UI  |
                         INTERNET_FLAG_RELOAD |                  //
                         INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS; //relevant for GET only
    if (postParams)
        requestFlags |= INTERNET_FLAG_NO_AUTO_REDIRECT; //POST would be re-issued as GET during auto-redirect => handle ourselves!

    HINTERNET hRequest = ::HttpOpenRequest(hSession,     //_In_ HINTERNET hConnect,
                                           postParams ? L"POST" : L"GET", //_In_ LPCTSTR   lpszVerb,
                                           page.c_str(), //_In_ LPCTSTR   lpszObjectName,
                                           nullptr,      //_In_ LPCTSTR   lpszVersion,
                                           nullptr,      //_In_ LPCTSTR   lpszReferer,
                                           acceptTypes,  //_In_ LPCTSTR   *lplpszAcceptTypes,
                                           requestFlags, //_In_ DWORD     dwFlags,
                                           0); //_In_ DWORD_PTR dwContext
    if (!hRequest)
        THROW_LAST_FILE_ERROR(_("Internet access failed."), L"HttpOpenRequest");
    ZEN_ON_SCOPE_EXIT(::InternetCloseHandle(hRequest));

    const std::wstring headers = postParams ? L"Content-type: application/x-www-form-urlencoded" : L"";
    std::string postParamsTmp = postParams ? *postParams : "";
    char* postParamBuf = postParamsTmp.empty() ? nullptr : &*postParamsTmp.begin();
    if (!::HttpSendRequest(hRequest,              //_In_ HINTERNET hRequest,
                           headers.c_str(),       //_In_ LPCTSTR   lpszHeaders,
                           static_cast<DWORD>(headers.size()),        //_In_ DWORD     dwHeadersLength,
                           postParamBuf,                              //_In_ LPVOID    lpOptional,
                           static_cast<DWORD>(postParamsTmp.size()))) //_In_ DWORD     dwOptionalLength
        THROW_LAST_FILE_ERROR(_("Internet access failed."), L"HttpSendRequest");

    DWORD sc = 0;
    {
        DWORD bufLen = sizeof(sc);
        if (!::HttpQueryInfo(hRequest, //_In_     HINTERNET hRequest,
                             HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, //_In_     DWORD dwInfoLevel,
                             &sc,      //_Inout_  LPVOID lpvBuffer,
                             &bufLen,  //_Inout_  LPDWORD lpdwBufferLength,
                             nullptr)) //_Inout_  LPDWORD lpdwIndex
            THROW_LAST_FILE_ERROR(_("Internet access failed."), L"HttpQueryInfo: HTTP_QUERY_STATUS_CODE");
    }

    //http://en.wikipedia.org/wiki/List_of_HTTP_status_codes#3xx_Redirection
    if (sc / 100 == 3) //e.g. 301, 302, 303, 307... we're not too greedy since we check location, too!
    {
        if (level < 5) //"A user agent should not automatically redirect a request more than five times, since such redirections usually indicate an infinite loop."
        {
            DWORD bufLen = 10000;
            std::wstring location(bufLen, L'\0');
            if (!::HttpQueryInfo(hRequest, HTTP_QUERY_LOCATION, &*location.begin(), &bufLen, nullptr))
                THROW_LAST_FILE_ERROR(_("Internet access failed."), L"HttpQueryInfo: HTTP_QUERY_LOCATION");
            if (bufLen >= location.size()) //HttpQueryInfo expected to write terminating zero
                throw FileError(_("Internet access failed."), L"HttpQueryInfo: HTTP_QUERY_LOCATION, buffer overflow");
            location.resize(bufLen);

            if (!location.empty())
                return sendHttpRequestImpl(location, userAgent, postParams, level + 1);
        }
        throw FileError(_("Internet access failed."), L"Unresolvable redirect.");
    }

    if (sc != HTTP_STATUS_OK) //200
        throw FileError(_("Internet access failed."), replaceCpy<std::wstring>(L"HTTP status code %x.", L"%x", numberTo<std::wstring>(sc)));
    //e.g. 404 - HTTP_STATUS_NOT_FOUND

    std::string buffer;
    const DWORD blockSize = 64 * 1024;
    //internet says "HttpQueryInfo() + HTTP_QUERY_CONTENT_LENGTH" not supported by all http servers...
    for (;;)
    {
        buffer.resize(buffer.size() + blockSize);

        DWORD bytesRead = 0;
        if (!::InternetReadFile(hRequest,    //_In_   HINTERNET hFile,
                                &*(buffer.begin() + buffer.size() - blockSize),  //_Out_  LPVOID lpBuffer,
                                blockSize,   //_In_   DWORD dwNumberOfBytesToRead,
                                &bytesRead)) //_Out_  LPDWORD lpdwNumberOfBytesRead
            THROW_LAST_FILE_ERROR(_("Internet access failed."), L"InternetReadFile");

        if (bytesRead > blockSize) //better safe than sorry
            throw FileError(_("Internet access failed."), L"InternetReadFile: buffer overflow.");

        if (bytesRead < blockSize)
            buffer.resize(buffer.size() - (blockSize - bytesRead)); //caveat: unsigned arithmetics

        if (bytesRead == 0)
            return buffer;
    }

#else
    assert(std::this_thread::get_id() == mainThreadId);
    assert(wxApp::IsMainLoopRunning());

    wxHTTP webAccess;
    webAccess.SetHeader(L"User-Agent", userAgent);
    webAccess.SetTimeout(10 /*[s]*/); //default: 10 minutes: WTF are these wxWidgets people thinking???

    if (!webAccess.Connect(server)) //will *not* fail for non-reachable url here!
        throw FileError(_("Internet access failed."), L"wxHTTP::Connect");

    if (postParams)
        if (!webAccess.SetPostText(L"application/x-www-form-urlencoded", utfCvrtTo<wxString>(*postParams)))
            throw FileError(_("Internet access failed."), L"wxHTTP::SetPostText");

    std::unique_ptr<wxInputStream> httpStream(webAccess.GetInputStream(page)); //must be deleted BEFORE webAccess is closed
    const int sc = webAccess.GetResponse();

    //http://en.wikipedia.org/wiki/List_of_HTTP_status_codes#3xx_Redirection
    if (sc / 100 == 3) //e.g. 301, 302, 303, 307... we're not too greedy since we check location, too!
    {
        if (level < 5) //"A user agent should not automatically redirect a request more than five times, since such redirections usually indicate an infinite loop."
        {
            const std::wstring newUrl(webAccess.GetHeader(L"Location"));
            if (!newUrl.empty())
                return sendHttpRequestImpl(newUrl, userAgent, postParams, level + 1);
        }
        throw FileError(_("Internet access failed."), L"Unresolvable redirect.");
    }

    if (sc != 200) //HTTP_STATUS_OK
        throw FileError(_("Internet access failed."), replaceCpy<std::wstring>(L"HTTP status code %x.", L"%x", numberTo<std::wstring>(sc)));

    if (!httpStream || webAccess.GetError() != wxPROTO_NOERR)
        throw FileError(_("Internet access failed."), L"wxHTTP::GetError");

    std::string buffer;
    int newValue = 0;
    while ((newValue = httpStream->GetC()) != wxEOF)
        buffer.push_back(static_cast<char>(newValue));
    return buffer;
#endif
}


//encode into "application/x-www-form-urlencoded"
std::string urlencode(const std::string& str)
{
    std::string out;
    for (const char c : str) //follow PHP spec: https://github.com/php/php-src/blob/master/ext/standard/url.c#L500
        if (c == ' ')
            out += '+';
        else if (('0' <= c && c <= '9') ||
                 ('A' <= c && c <= 'Z') ||
                 ('a' <= c && c <= 'z') ||
                 c == '-' || c == '.' || c == '_') //note: "~" is encoded by PHP!
            out += c;
        else
        {
            const char hexDigits[] = "0123456789ABCDEF";
            out += '%';
            out += hexDigits[static_cast<unsigned char>(c) / 16];
            out += hexDigits[static_cast<unsigned char>(c) % 16];
        }
    return out;
}
}


std::string zen::sendHttpPost(const std::wstring& url, const std::wstring& userAgent, const std::vector<std::pair<std::string, std::string>>& postParams) //throw FileError
{
    //convert post parameters into "application/x-www-form-urlencoded"
    std::string flatParams;
    for (const auto& pair : postParams)
        flatParams += urlencode(pair.first) + '=' + urlencode(pair.second) + '&';
    //encode both key and value: https://www.w3.org/TR/html401/interact/forms.html#h-17.13.4.1
    if (!flatParams.empty())
        flatParams.pop_back();

    return sendHttpRequestImpl(url, userAgent, &flatParams); //throw FileError
}


std::string zen::sendHttpGet(const std::wstring& url, const std::wstring& userAgent) //throw FileError
{
    return sendHttpRequestImpl(url, userAgent, nullptr); //throw FileError
}


bool zen::internetIsAlive() //noexcept
{
#ifdef ZEN_WIN
    //::InternetAttemptConnect(0) -> not working as expected: succeeds even when there is no internet connection!

    HINTERNET hInternet = ::InternetOpen(L"FreeFileSync", //_In_  LPCTSTR lpszAgent,
                                         INTERNET_OPEN_TYPE_PRECONFIG, //_In_  DWORD dwAccessType,
                                         nullptr, //_In_  LPCTSTR lpszProxyName,
                                         nullptr, //_In_  LPCTSTR lpszProxyBypass,
                                         0);      //_In_  DWORD dwFlags
    if (!hInternet)
        return false;
    ZEN_ON_SCOPE_EXIT(::InternetCloseHandle(hInternet));

    //InternetOpenUrl is shortcut for HTTP:GET with InternetConnect + HttpOpenRequest + HttpSendRequest:
    HINTERNET hRequest = ::InternetOpenUrl(hInternet,   //_In_  HINTERNET hInternet,
                                           L"http://www.google.com/", //_In_  LPCTSTR lpszUrl,
                                           nullptr,     //_In_  LPCTSTR lpszHeaders,
                                           0,           //_In_  DWORD dwHeadersLength,
                                           INTERNET_FLAG_KEEP_CONNECTION |
                                           INTERNET_FLAG_NO_UI  |
                                           INTERNET_FLAG_RELOAD |
                                           INTERNET_FLAG_NO_AUTO_REDIRECT, //_In_  DWORD dwFlags,
                                           0);          //_In_  DWORD_PTR dwContext
    //fails with ERROR_INTERNET_NAME_NOT_RESOLVED if server not found => the server-relative part is checked by HTTP_QUERY_STATUS_CODE!!!
    if (!hRequest)
        return false;
    ZEN_ON_SCOPE_EXIT(::InternetCloseHandle(hRequest));

    DWORD sc = 0;
    {
        DWORD bufLen = sizeof(sc);
        if (!::HttpQueryInfo(hRequest, //_In_     HINTERNET hRequest,
                             HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, //_In_     DWORD dwInfoLevel,
                             &sc,      //_Inout_  LPVOID lpvBuffer,
                             &bufLen,  //_Inout_  LPDWORD lpdwBufferLength,
                             nullptr)) //_Inout_  LPDWORD lpdwIndex
            return false;
    }

#else
    assert(std::this_thread::get_id() == mainThreadId);

    const wxString server = L"www.google.com";
    const wxString page   = L"/";

    wxHTTP webAccess;
    webAccess.SetTimeout(10 /*[s]*/); //default: 10 minutes: WTF are these wxWidgets people thinking???

    if (!webAccess.Connect(server)) //will *not* fail for non-reachable url here!
        return false;

    std::unique_ptr<wxInputStream> httpStream(webAccess.GetInputStream(page)); //call before checking wxHTTP::GetResponse()
    const int sc = webAccess.GetResponse();
#endif
    //attention: http://www.google.com/ might redirect to "https" => don't follow, just return "true"!!!
    return sc / 100 == 2 || //e.g. 200
           sc / 100 == 3;   //e.g. 301, 302, 303, 307... when in doubt, consider internet alive!
}
