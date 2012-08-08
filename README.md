# BASH - Radio

  ## make 
    creates fan and streamer

  ## fan 
    used to split the mp3 file into ogg and mp3 streams

  ## streamer
    used to connect to icecast2 server
    requires libshout

  ## usefull commands
    ```madplay -b 16 -R 44100 -S -o raw:- -```
      convert mp3 from stdin to raw pcm on stdout

    ```lame --preset cbr 128 -r -s 44.1 --bitwidth 16 - -```
      convert stdin to mp3 on stdout

    ```oggdec -R -b 16 -e 0 -s 1 -o -```
      convet ogg on stdin to raw on stdout

    ```oggenc -r -b 128 -B 16 -C 2 -R 44100 --raw-endianness 0 -```
      convert raw pcm from stdin to ogg vorbis on stdout

    
# example usage

  this will take an mp3 file, reencode it to ogg and mp3 
  streams at 128kbs and stream them to an icecast server. 
```cat myfile.mp3 | 
  fan "madplay -b 16 -R 44100 -S -o raw:- - 
       | oggenc -b 128 -r -B 16 -C 2 -R 44100 --raw-endianness 0 - 
       | streamer 0 host port pass mount" 
      "lame --mp3input --preset cbr 128 -s 44.1 --bitwidth 16 - - 
       | streamer 1 host port pass mount"```

w0rd

  pshr.co

