#!/bin/bash

# https://github.com/beautify-web/js-beautify

ROOT="https://app.storz-bickel.com"
FILES=`curl -s $ROOT | grep "text/javascript" | grep -v "jquery" | grep -v "bootstrap" | sed 's/.*src="//g' | sed 's*"></script>**g'`

for F in $FILES
do
    N=`echo $F | sed 's*/*_*g'`
    O=`echo $N | sed 's/.js/.min.js/g'`
    curl -o $O $ROOT/$F
    js-beautify $O > $N
done
