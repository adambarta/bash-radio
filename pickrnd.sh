#!/bin/bash
trap "echo chaning track; ps uax | grep cat $DIR" SIGHUP

set -x

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
  LIST=($(ls $DIR/*.mp3))
  SIZE=${#LIST[@]}
 # N=$[$RANDOM%$SIZE]
  N=$(( $(cat /dev/urandom | od -N3 -An -i) % $SIZE ))

  until [ $N -ne $LAST ];
  do
    #N=$[$RANDOM%$SIZE]
    N=$(( $(cat /dev/urandom | od -N3 -An -i) % $SIZE ))
  done

  echo $(date +%F-%T)" ${LIST[$N]}" >> $DIR/../track

#  sleep 1

  #(cat "$DIR/${LIST[$N]}")
  MP3=${LIST[$N]}
  OGG=${MP3%%.mp3}.ogg

  #(~/live_audio_streaming/fan "cat '$MP3' > /srv/mp3" "cat '$OGG' > /srv/ogg")
  #coproc cat '$MP3' > /srv/mp3

  #coproc cat '$OGG' > /srv/ogg

  (cat $MP3 > /srv/mp3) &
  (cat $OGG > /srv/ogg)

#  echo -e $MP3"\n"$OGG
#notes (mkfifo /srv/beats/{mp3,ogg}) (while true; do cat /srv/beats/mp3) | streamer

  LAST=$N

done

