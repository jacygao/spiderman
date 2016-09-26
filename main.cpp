// main file
#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <queue>
#include <curl/curl.h>
#include <algorithm>
#include "gumbo.h"

using namespace std;

// Ideally we use an exteral queue servive (sqs/rabbitMQ) instead of global variable
queue<string> linkStore;

string reformatUrl(string url, string prefix, string host) {
   if (url.substr(0, prefix.size()) != prefix) {
      if (url.substr(0, 1) == "/" && url.length() > 1) {
        return host += url;
      }
   }
   return url;
}

static bool isUrlValid(string url, string host) {
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

static bool isExternalUrl(string url, string host) {
   if (url.substr(0, host.size()) != host) {
      return true;
   }
   return false;
}

static void search_for_links(GumboNode* node, string host) {
  if (node->type != GUMBO_NODE_ELEMENT) {
    return;
  }
  GumboAttribute* href;
  if (node->v.element.tag == GUMBO_TAG_A &&
      (href = gumbo_get_attribute(&node->v.element.attributes, "href"))) {
    if(isUrlValid(string(href->value), host)){
      string url = reformatUrl(string(href->value), host, host);
      if(!isExternalUrl(url, host)){
        linkStore.push(url);
      }
    }
  }

  GumboVector* children = &node->v.element.children;
  for (unsigned int i = 0; i < children->length; ++i) {
    search_for_links(static_cast<GumboNode*>(children->data[i]), host);
  }
}

static string cleantext(GumboNode* node) {
  if (node->type == GUMBO_NODE_TEXT) {
    return string(node->v.text.text);
  } else if (node->type == GUMBO_NODE_ELEMENT &&
             node->v.element.tag != GUMBO_TAG_SCRIPT &&
             node->v.element.tag != GUMBO_TAG_STYLE) {
    string contents = "";
    GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
      const string text = cleantext((GumboNode*) children->data[i]);
      if (i != 0 && !text.empty()) {
        contents.append(" ");
      }
      contents.append(text);
    }
    return contents;
  } else {
    return "";
  }
}

static string cleanPunct(string text) {
    for (int i = 0, len = text.size(); i < len; i++)
    {
        if (ispunct(text[i]))
        {
            text.erase(i--, 1);
        }
    }
 	return text;
}

/*
 *  Count total number of unique words in a string.
 */

static map<string,size_t> countUniqueWords(string text) {
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
 */
static map<string,size_t> countTotalUniqueWord(map<string,size_t> map1, map<string,size_t> map2) {
	for(auto it = map2.begin(); it != map2.end(); ++it) {
		map1[it->first] += it->second;
	}
	return map1;
}

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

static string curl(string host){
	CURL *curl;
	CURLcode res;
	string readBuffer;

	curl = curl_easy_init();
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL, host.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	}
	return readBuffer;
}

int main(int argc, char *argv[]){
  	cout << "Spiderman woke up..." << endl;

	string host = argv[1]; //the url that will be entered
	int depth;

	istringstream iss(argv[2]); //sstream object
	
	if(!(iss >> depth)) //valid number checking
	{
		cout << "Invalid number: " << argv[2] << "\n";
		return 0;
	}
	else
	{
		iss >> depth; //if valid save to int
	}

	cout << host << endl;
    int current_depth = 0;
    int distance = 1;

    linkStore.push(host);

    map<string,size_t> uniqueWordsPerLink;

    while(current_depth <= depth && !linkStore.empty()){
        string url = linkStore.front();
        // TODO: Convert invalid / incomplete urls
        //url = reformatUrl(url, host, host);
        // check if url has been processed
        if ( uniqueWordsPerLink.find(url) == uniqueWordsPerLink.end() ) {
          // TODOD: load string from user input
          // Curl
          string readBuffer = curl(url);

          GumboOutput* output = gumbo_parse(readBuffer.c_str());
          // Find all links in the html
          // getLinks, insert to queue
          cout << "Currently Depth : " <<  current_depth << endl;
          cout << "Distance to the next level : " <<  distance << endl;
          search_for_links(output->root, host);

          if(distance <= 1){
            distance = linkStore.size();
            current_depth++;
          }

          // Get count per word
          // TODO: Only body content should be included
          map<string,size_t> totalWords = countUniqueWords(cleanPunct(cleantext(output->root)));

          // Get count per url
          uniqueWordsPerLink[url] = totalWords.size();

          // Print count per url
          cout << url << " : " << uniqueWordsPerLink[url] << endl;
          gumbo_destroy_output(&kGumboDefaultOptions, output);
        }

        linkStore.pop();
        distance--;
    }
  	return 0;
}