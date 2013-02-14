#!/bin/bash

ls *.mp3 | while read file; do mv -v "$file" $(echo "$file" | tr ' ' '_' | tr -d '[{}(),\!]' | tr -d "\'" | tr '[A-Z]' '[a-z]' | sed 's/_-_/_/g'); done;
