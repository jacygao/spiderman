#ifndef SIMPLECURL_H
#define SIMPLECURL_H

using namespace std;

class simple_curl {
	private:
		static size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp);
	public:
		static string curl(string host);
};

#endif