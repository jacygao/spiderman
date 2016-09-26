# Spiderman
A simple web crawler that crawls a website n-links deep and calculate the number of  unique rendered words found on each page and in total.

<b>One-time setup</b>

Install Gumbo (https://github.com/google/gumbo-parser)

      git clone https://github.com/google/gumbo-parser.git
      
      $ ./autogen.sh
      $ ./configure
      $ make
      $ sudo make install
      
For Mac with Homebrew, do:
      
      brew install gumbo-parser
      
Clone Spiderman repo

      git clone https://github.com/JacyGao/spiderman.git
      
To compile Spiderman, do:

      g++ -std=c++14 -lcurl main.cpp `pkg-config --cflags --libs gumbo`
      
To run Spiderman, do:

      ./a.out {url} {depth}
      
For example

      ./a.out http://www.ea.com 1
