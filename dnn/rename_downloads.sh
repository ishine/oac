#!/bin/sh

echo changing files
for i in `find dnn | grep "_data\.[ch]" | grep -v dump_data`
do
	cat $i | sed -e 's/OPUS/OAC/g' -e 's/Opus/Oac/g' -e 's/opus_/oac_/g' -e 's/opus/oac/g' > tmp.c
	cp tmp.c $i; rm tmp.c
done
