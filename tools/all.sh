#!/bin/bash

g++ -std=c++14 -lcurl main.cpp `pkg-config --cflags --libs gumbo`