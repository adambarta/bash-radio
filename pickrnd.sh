#!/bin/bash

#set -x

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

FIFO=/srv/beats/mp3
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

  echo $(date +%F-%T)" $(basename ${LIST[$N]} .mp3)" >> $DIR/../track


  #(cat "$DIR/${LIST[$N]}")
  MP3=${LIST[$N]}

  cat $MP3

# OGG=${MP3%%.mp3}.ogg
# coproc PROC_OGG ( cat $OGG > /srv/ogg )
  #coproc PROC_MP3 ( cat $MP3 )

  #PIDS="$PROC_MP3_PID" #$PROC_OGG_PID"

  #trap "echo changing track; kill ${PIDS[0]}; kill ${PIDS[1]}" SIGHUP
  #trap "echo changing track; kill ${PIDS[0]}" SIGHUP

  #echo "PIDs: "$PIDS" TRAc: "$MP3

#  for PID in $PIDS
#    do
#      #echo $PID
#      wait $PID
#    done

  #sleep 5

  
  #(~/live_audio_streaming/fan "cat '$MP3' > /srv/mp3" "cat '$OGG' > /srv/ogg")
  #coproc cat '$MP3' > /srv/mp3

  #coproc cat '$OGG' > /srv/ogg

#  (cat $MP3 > /srv/mp3) &
#  (cat $OGG > /srv/ogg)

#  echo -e $MP3"\n"$OGG
#notes (mkfifo /srv/beats/{mp3,ogg}) (while true; do cat /srv/beats/mp3) | streamer

  LAST=$N

done

