#!/bin/bash

count=`ls -l patches/*.patch | wc -l`
echo "$count patches are needed to apply, please confirm:(y/n)"
read answer

if [ "$answer" = "y" ]
then
    for index in  `seq 1 $count`
    do
        prefix=000$index
        if [ $index -ge 10 ]; then
            prefix=00$index
        elif [ $index -ge 100 ]; then
            prefix=0$index
        fi
        git am patches/$prefix*.patch
    done
    sudo service apache2 restart
else
    echo "Abort!"
    exit 0
fi
