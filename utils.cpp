#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include "utils.h"

using namespace std;

/*
*  reformat invalid URLs
*/
string Utils::reformatUrl(string url, string prefix, string host) {
  if (url.substr(0, prefix.size()) != prefix) {
    if (url.substr(0, 1) == "/" && url.length() > 1) {
      return host += url;
    }
  }
  return url;
}

/*
*  reformat invalid Host
*  valid host sample: https://www.ea.com
*/
string Utils::reformatHost(string url) {
  if(url.substr(0, 4) != "http"){
    url = "http://"+url;
  }
  if(url.substr(url.length() - 1) == "/"){
    url.pop_back();
  }
  return url;
}

/*
*  Check if URL is in valid format
*/
bool Utils::isUrlValid(string url, string host) {
  // url variable is empty
  if(url.length() == 0) {
    return false;
  }  
  // url variable is hash trigger
  if(url.substr(0,1) == "#") {
    return false;
  }
  // url variable is adding param
  if(url.substr(0,1) == "?") {
    return false;
  }
  if(url.length() > 1) {
    // url variable is adding param
    if(url.substr(0,2) == "/?") {
      return false;
    }
    // url variable is hash trigger
    if(url.substr(0,2) == "/#") {
      return false;
    }
    if(url.length() >= host.size()) {
      if (url.substr(0, host.size()+1) == host+"?") {
        return false;
      }
      if (url.substr(0, host.size()+1) == host+"#") {
        return false;
      }
      if (url.substr(0, host.size()+2) == host+"/?") {
        return false;
      }
      if (url.substr(0, host.size()+2) == host+"/#") {
        return false;
      }
    }
  }
  return true;
}

/*
*  Check if URL is external
*/
bool Utils::isExternalUrl(string url, string host) {
  if (url.substr(0, host.size()) != host) {
    return true;
  }
  return false;
}

/*
*  Count total number of unique words in a string.
*/
map<string,size_t> Utils::countUniqueWords(string text) {
  map<string,size_t> wordcount;
  stringstream is(text);
  string word;
  int number_of_words = 0;
  while (is >> word)
    if(wordcount.find(word) == wordcount.end()){
      // Unique word
      wordcount[word] = 1;
    } 
    else {
      // Duplicate word
      wordcount[word] += 1;
    }

    return wordcount;
  }

/*
*  Add two maps together with the following behavior:
*  If key exists add two key values together.
*  If key does not exist. Insert pair to map.
*  This Method is used for calculating total as 
*  We'd like to filter out duplicate words across pages
*/
map<string,size_t> Utils::countTotalUniqueWord(map<string,size_t> map1, map<string,size_t> map2) {
  for(auto it = map2.begin(); it != map2.end(); ++it) {
    map1[it->first] += it->second;
  }
  return map1;
}
