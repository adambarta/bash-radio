#!/bin/bash



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

  cat "$DIR/${LIST[$N]}"
  sleep 1;
  LAST=$N

done

