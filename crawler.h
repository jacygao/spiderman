#ifndef CRAWLER_H
#define CRAWLER_H

using namespace std;

class Crawler {
	private:
		queue<string> linkStore;
	  	map<string,size_t> uniqueWordsPerLink;
  		map<string,size_t> TotaluniqueWordsCount;
		void searchLinks(GumboNode* node, string host);
		static string cleanText(GumboNode* node);
	public:
		void crawl(string host, int depth);
};

#endif