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
#SOFTWAREDIR=/home/adam/live_audio_streaming
SOFTWAREDIR=/srv/beats/bash-radio

while (true)
do
   LIST=( $($SOFTWAREDIR/ar $1) )

   for N in ${LIST[@]} 
   do
     echo $(date +%F-%T)" $(basename $N .mp3)" >> $DIR/../track
     cat $N
   done
   
done

