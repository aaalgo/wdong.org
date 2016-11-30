#!/bin/bash

if [ -z "$1" ]
then
    exit
fi

DATE=`date +%Y-%m-%d`

F=content/$1/$1.md

if [ ! -f $F ]
then

mkdir -p content/$1
cat > content/$1/$1.md <<FOO
Title: 
Date: $DATE
Category: 

FOO
fi

vi content/$1/$1.md
