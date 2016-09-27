#include <iostream>
#include <string>
#include <map>
#include <queue>
#include "gumbo.h"
#include "utils.h"
#include "simple_curl.h"
#include "crawler.h"

/*
*	Finds the URLs of all links in the page and push to queue
*	Validate if URL is valid
*	Validate if URL belongs to host
*	Strip out params and hashes from URLs
*/
void Crawler::searchLinks(GumboNode* node, string host) {
  if (node->type != GUMBO_NODE_ELEMENT) {
    return;
  }
  GumboAttribute* href;
  if (node->v.element.tag == GUMBO_TAG_A &&
    (href = gumbo_get_attribute(&node->v.element.attributes, "href"))) {
    if(Utils::isUrlValid(string(href->value), host)){
      string url = Utils::reformatUrl(string(href->value), host, host);
      if(!Utils::isExternalUrl(url, host)){
        linkStore.push(url);
      }
    }
  }

  GumboVector* children = &node->v.element.children;
  for (unsigned int i = 0; i < children->length; ++i) {
    searchLinks(static_cast<GumboNode*>(children->data[i]), host);
  }
}

/*
*  Get the cleantext of a page
*/
string Crawler::cleanText(GumboNode* node) {
  if (node->type == GUMBO_NODE_TEXT) {
    return string(node->v.text.text);
  } else if (node->type == GUMBO_NODE_ELEMENT &&
    node->v.element.tag != GUMBO_TAG_SCRIPT &&
    node->v.element.tag != GUMBO_TAG_STYLE) {
    string contents = "";
    GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
      const string text = cleanText((GumboNode*) children->data[i]);
      if (i != 0 && !text.empty()) {
        contents.append(" ");
      }
      contents.append(text);
    }
    //Remove all punctuations
    for (int i = 0, len = contents.size(); i < len; i++)
    {
      if (ispunct(contents[i]))
      {
       contents.erase(i--, 1);
      }
    }
    return contents;
  } else {
    return "";
  }
}

/*
*  Crawling Algorithm
*  When the process starts a new depth, reset distance based on queue size
*  Distance - 1 after every link gets processed
*  Depth + 1 after distance reaches 0
*  Stop the while loop if current depth is greater than defined depth or queue is empty
*/
void Crawler::crawl(string host, int depth) {

  int current_depth = 0;
  int distance = 1;

  linkStore.push(host);

  while(current_depth <= depth && !linkStore.empty()){
    string url = linkStore.front();

    // check if url has been processed
    if ( uniqueWordsPerLink.find(url) == uniqueWordsPerLink.end() ) {

      // Curl
      string readBuffer = Simple_curl::curl(url);

      // Get Output string
      GumboOutput* output = gumbo_parse(readBuffer.c_str());

      cout << "Currently Depth : " <<  current_depth << endl;
      cout << "Distance to the next level : " <<  distance << endl;

      // Find all links in the html
      // GetLinks, insert to queue
      searchLinks(output->root, host);

      // reset distance and increase depth after all links of current depth have been processed
      if(distance <= 1){
        distance = linkStore.size();
        current_depth++;
      }

      // Get count per word for one page
      map<string,size_t> totalWords = Utils::countUniqueWords(cleanText(output->root));

      // Get unique words count per url
      uniqueWordsPerLink[url] = totalWords.size();

      // Get unique words count in total
      TotaluniqueWordsCount = Utils::countTotalUniqueWord(TotaluniqueWordsCount, totalWords);

      // Print count per url
      cout << url << " : " << uniqueWordsPerLink[url] << endl;
      cout << "Total unique words : " << TotaluniqueWordsCount.size() << endl;
      gumbo_destroy_output(&kGumboDefaultOptions, output);
    }

    // Pop processed link off the queue and decrease distance
    linkStore.pop();
    distance--;
  }

}