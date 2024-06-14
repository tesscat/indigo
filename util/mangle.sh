#!/bin/bash

# echo -e "$1 {} " | clang++ -x c++ -S - -o- -Wno-everything | grep "# @" | sed -e 's/^.*\@//'

IFS='::' read -a array <<< "$1"

indexes=("${!array[@]}")

prefix=""
middle=""
suffix=""
rettype=""
if [ -z "$2" ]; then
    rettype="void"
else
    rettype="$2"
fi


for index in "${indexes[@]}"
do
    #echo "$index ${array[index]}"
    if [ $index == ${indexes[-1]} ]; then
    #echo "last"
    middle="$rettype ${array[index]};"
    elif [ -n "${array[index]}" ]; then
    #echo "not empty"
    prefix="${prefix}namespace ${array[index]}{"
    suffix="${suffix}};"
    fi
done

echo "$prefix$middle$suffix $rettype $1{}"
echo "$prefix$middle$suffix $rettype $1{}" | clang++ -x c++ -S - -o- -Wno-everything | grep "# @" | sed -e 's/^.*\@//' #| grep "^_.*:$" | sed -e 's/:$//'
