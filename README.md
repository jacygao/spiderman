# Spiderman
A simple web crawler that crawls a website n-links deep and calculate the number of  unique rendered words found on each page and in total.

Please be aware that I'm not a C++ developer and I don't have professional experience with C++. I would feel more comfortable coding it in a different language such as PHP/Javascript/Golang. Sorry for any rookie mistakes.

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

      tools/all.sh
      
To run Spiderman, do:

      ./a.out {url} {depth}
      
For example

      ./a.out http://www.ea.com 1
