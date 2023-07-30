#! /bin/bash

dir="./invalid_test"
for file in $(ls $dir)
do
	path="./invalid_test/$file"
	echo $path
	../server $path
done
