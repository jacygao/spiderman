#ifndef UTILS_H
#define UTILS_H

using namespace std;

class utils {
	private:

	public:
		static string reformatUrl(string url, string prefix, string host);
		static string reformatHost(string url);
		static bool isUrlValid(string url, string host);
		static bool isExternalUrl(string url, string host);
		static map<string,size_t> countUniqueWords(string text);
		static map<string,size_t> countTotalUniqueWord(map<string,size_t> map1, map<string,size_t> map2);
};

#endif