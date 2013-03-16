#!/bin/bash

set -ex

WATCHDIR=/srv/beats/chunes/upload
OUTPUT=/srv/beats/chunes/encode
ORIG=/srv/beats/chunes/original
BPS=160
BPSK=160k

inotifywait -e moved_to -qm --format "%f" $WATCHDIR | while read f; do

  FNAME=$(echo "$f" | tr ' ' '_' | tr -d '[{}(),\!]' | tr -d "\'" | tr '[A-Z]' '[a-z]' | sed 's/_-_/_/g')
  
  #FNAME=${FNAME%%.mp3}
  MP3=$FNAME
#  OGG=$FNAME.ogg

  if [[ ! -f $OUTPUT/"$MP3" ]] 
  then
    echo -e "mp3: "$MP3
    (lame --mp3input --preset cbr $BPS -s 44.1 --bitwidth 16 $WATCHDIR/"$f" $OUTPUT/"$MP3")
  fi
 
#  if [[ ! -f $OUTPUT/"$OGG" ]]
#  then
#    echo -e "ogg: "$OGG
#    (avconv -i $WATCHDIR/"$f"  -map 0:a -c:a vorbis -q:a 7 -minrate $BPSK -maxrate $BPSK -b:a $BPSK -strict experimental $OUTPUT/"$OGG")
#    #(madplay -b 16 -R 44100 -S -o raw:- $WATCHDIR/"$f" | oggenc -b $BPS -r -B 16 -C 2 -R 44100 --raw-endianness 0 - -o $OUTPUT/"$OGG")
#  fi

  mv -v $WATCHDIR/"$f" $ORIG/ 

done
