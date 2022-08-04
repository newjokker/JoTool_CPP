#ifndef __LOAD_IMG_HPP_
#define _LOAD_IMG_HPP_

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <iostream>
#include <unistd.h>
 
// using namespace curlpp::options;

// 根据 url 路径下载

void ImageDownloader(std::string image_url, std::string save_address)
{
    CURL* curl;
    CURLcode res;

    curl = curl_easy_init();

    FILE* fp = fopen(save_address.c_str(),"wb");
    res = curl_easy_setopt(curl, CURLOPT_URL, image_url.c_str());
    if(res != CURLE_OK)
    {
        curl_easy_cleanup(curl);
        return;
    }

    res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
    res = curl_easy_perform(curl);
    fclose(fp);

    curl_easy_cleanup(curl);
}


#endif