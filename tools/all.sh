#!/bin/bash

g++ -std=c++14 -lcurl main.cpp utils.cpp simple_curl.cpp `pkg-config --cflags --libs gumbo`