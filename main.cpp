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

/*
 *  reformat invalid URLs
 */
string reformatUrl(string url, string prefix, string host) {
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
string reformatHost(string url) {
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

/*
 *  Check if URL is external
 */
static bool isExternalUrl(string url, string host) {
   if (url.substr(0, host.size()) != host) {
      return true;
   }
   return false;
}

/*
 *  Finds the URLs of all links in the page
 *  Validate if URL is valid
 *	Validate if URL belongs to host
 *	strip out params and hashes from URLs
 */
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

/*
 *  Get the cleantext of a page
 */

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

/*
 *  Remove all punctuation from HTML
 */
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
 *  This Method is used for calculating total as 
 *	We'd like to filter out duplicate words across pages
 */
static map<string,size_t> countTotalUniqueWord(map<string,size_t> map1, map<string,size_t> map2) {
	for(auto it = map2.begin(); it != map2.end(); ++it) {
		map1[it->first] += it->second;
	}
	return map1;
}

/*
 *	Simple CURL functions to retrieve HTML
 */
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
  	cout << "Spiderman wakes up..." << endl;

  	//the url that will be entered
	string host = argv[1];
	host = reformatHost(host);

	int depth;
	//sstream object
	istringstream iss(argv[2]);
	//valid number checking
	if(!(iss >> depth))
	{
		cout << "Invalid number: " << argv[2] << "\n";
		return 0;
	}
	else
	{
		//if valid save to int
		iss >> depth;
	}

	cout << "Starting from :" << host << endl;
    int current_depth = 0;
    int distance = 1;

    linkStore.push(host);

    map<string,size_t> uniqueWordsPerLink;
    map<string,size_t> TotaluniqueWordsCount;

    /*
     *  Crawling Algorithm
     *  When the process starts a new depth, reset distance based on queue size
     *	Distance - 1 after every link gets processed
     *	Depth + 1 after distance reaches 0
     *	Stop the while loop if current depth is greater than defined depth or queue is empty
     */
    while(current_depth <= depth && !linkStore.empty()){
        string url = linkStore.front();

        // check if url has been processed
        if ( uniqueWordsPerLink.find(url) == uniqueWordsPerLink.end() ) {

          // Curl
          string readBuffer = curl(url);

          // Get Output string
          GumboOutput* output = gumbo_parse(readBuffer.c_str());

          cout << "Currently Depth : " <<  current_depth << endl;
          cout << "Distance to the next level : " <<  distance << endl;

          // Find all links in the html
          // GetLinks, insert to queue
          search_for_links(output->root, host);

          // reset distance and increase depth after all links of current depth have been processed
          if(distance <= 1){
            distance = linkStore.size();
            current_depth++;
          }

          // Get count per word for one page
          map<string,size_t> totalWords = countUniqueWords(cleanPunct(cleantext(output->root)));

          // Get unique words count per url
          uniqueWordsPerLink[url] = totalWords.size();

          // Get unique words count in total
          TotaluniqueWordsCount = countTotalUniqueWord(TotaluniqueWordsCount, totalWords);

          // Print count per url
          cout << url << " : " << uniqueWordsPerLink[url] << endl;
          cout << "Total unique words : " << TotaluniqueWordsCount.size() << endl;
          gumbo_destroy_output(&kGumboDefaultOptions, output);
        }

        // Pop processed link off the queue and decrease distance
        linkStore.pop();
        distance--;
    }
    cout << "Spiderman goes to sleep..." << endl;
  	return 0;
}