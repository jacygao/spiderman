// main file
#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <queue>
#include <algorithm>
#include "gumbo.h"
#include "utils.h"
#include "crawler.h"

using namespace std;

int main(int argc, char *argv[]){

  cout << "Spiderman wakes up..." << endl;

  //the url that will be entered
  string host = argv[1];
  //ensure host is in valid format
  host = Utils::reformatHost(host);

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

  // Start crawling
  Crawler c;
  c.crawl(host, depth);

  cout << "Spiderman goes to sleep..." << endl;
  return 0;
}