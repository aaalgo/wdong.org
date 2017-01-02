Title: Tips for Cross Compiling Libraries for Android
Date: 2013-03-19
Category: Technology
Tags: C++, Android

Assume we want to install the libraries to "/opt/arm-tools".

1. Boost

Edit the file "tools/build/v2/user-config.jam" under the boost source directory and add the following line:

```bash
using gcc : arm : arm-none-linux-gnueabi-g++ ;
```

Then build and install boost with the following command

```bash
./b2 toolset=gcc-arm  target-os=linux threading=multi link=static runtime-link=static variant=release optimization=space --prefix=/opt/arm-tools
```

2. Packages with Autotools

Run ./configure with the following script

```bash
#!/bin/bash

export CC=arm-none-linux-gnueabi-gcc
export CXX=arm-none-linux-gnueabi-g++
export AR=arm-none-linux-gnueabi-ar
export CFLAGS=-I/opt/arm-tools/include
export CXXFLAGS=-I/opt/arm-tools/include
export LDFLAGS=-L/opt/arm-tools/lib
./configure --host=x86_64-unknown-linux-gnu --target=arm-none-linux-gnueabi --enable-static --disable-shared --prefix=/opt/arm-tools

```
