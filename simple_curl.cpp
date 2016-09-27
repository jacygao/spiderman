#include <string>
#include <curl/curl.h>
#include "simple_curl.h"
/*
*	 Simple CURL functions to retrieve HTML
*/
size_t Simple_curl::writeCallback(void *contents, size_t size, size_t nmemb, void *userp) {
  ((string*)userp)->append((char*)contents, size * nmemb);
  return size * nmemb;
}

string Simple_curl::curl(string host){
  CURL *curl;
  CURLcode res;
  string readBuffer;

  curl = curl_easy_init();
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, host.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
  }
  return readBuffer;
}