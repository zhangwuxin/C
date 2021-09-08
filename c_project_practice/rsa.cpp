#include <iostream>
#include <sstream>
#include <json/json.h>
#include <curl/curl.h>
#include <exception>
#include <string>
#include <iostream>
#include <stdlib.h>

int writer(char *data, size_t size, size_t nmemb, string *writerData)
{
　　unsigned long sizes = size * nmemb;
　　if (writerData == NULL)
　　return -1;

　　writerData->append(data, sizes);
　　return sizes;
}

string parseJsonResponse_question(string input)
{
    Json::Value root;
    Json::Reader reader;
    bool parsingSuccessful = reader.parse(input, root);
    if(!parsingSuccessful)
    {
        std::cout<<"!!! Failed to parse the response data"<< std::endl;
        return "";
    }
    const Json::Value text = root["obj"]["question"];
    string result = text.asString(); 
    return result;
}

string HttpsGetRequest_question(string input)
{
    string buffer, ling_result;

    // 对请求参数中的中文和特殊字符（如空格等）进行处理，方可使用
    char * escape_control = curl_escape(input.c_str(), input.size());
    input = escape_control;
    curl_free(escape_control);

    string str_url= "https://*.*.*.*/question?question=" + input; // alter *.*.*.* by your server address
    
    try
    {
        CURL *pCurl = NULL;
        CURLcode res;

        // In windows, this will init the winsock stuff
        curl_global_init(CURL_GLOBAL_ALL); 
        // get a curl handle
        pCurl = curl_easy_init();
        if (NULL != pCurl)
        {
            // 设置超时时间为8秒
            curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 8); 
            curl_easy_setopt(pCurl, CURLOPT_URL, str_url.c_str());

            // 下面两个为验证对方和验证主机名，若为0，则跳过验证，我这个服务器必须验证才能得到请求数据
            curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYPEER, 1L); 
            curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYHOST, 1L);

            // 配置 https 请求所需证书
            curl_easy_setopt(pCurl,CURLOPT_CAINFO,"/etc/msc/ca_info.pem"); 
            curl_easy_setopt(pCurl, CURLOPT_SSLCERT, "/etc/msc/client.pem");
            curl_easy_setopt(pCurl, CURLOPT_SSLKEY, "/etc/msc/client_key.pem");
            curl_easy_setopt(pCurl, CURLOPT_KEYPASSWD, "your_key_password");

            curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, writer); 
            curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &buffer);

            // Perform the request, res will get the return code
            res = curl_easy_perform(pCurl);
            // Check for errors
            if (res != CURLE_OK)
            {
                printf("curl_easy_perform() failed:%s\n", curl_easy_strerror(res));
            }
            curl_easy_cleanup(pCurl);
        }
        curl_global_cleanup();
    }
    catch (std::exception &ex)
    {
        printf("curl exception %s.\n", ex.what());
    }
    if(buffer.empty())
    {
        std::cout<< "!!! ERROR The sever response NULL" << std::endl;
    }
    else
    {
        ling_result = parseJsonResponse_question(buffer);
    }
    return ling_result;
}