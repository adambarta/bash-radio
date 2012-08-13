#!/bin/bash

if [[ $# -lt 1 ]];
then
  echo "usage: "$0" directory"
  exit 1
fi

if [ ! -d $1 ];
then
  echo "directory not valid"
  exit 1
fi

DIR=$1

LAST=0

while (true) 
do
  LIST=($(ls $DIR))
  SIZE=${#LIST[@]}
  N=$[$RANDOM%$SIZE]

  until [ $N -ne $LAST ];
  do
    N=$[$RANDOM%$SIZE]
  done

  echo "${LIST[$N]}" >> $DIR/../track

#  sleep 1

  cat "$DIR/${LIST[$N]}"

  LAST=$N

done

