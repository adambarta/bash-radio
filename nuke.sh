#!/bin/bash

set -ex

track=$(tail -n 1 /srv/beats/track | awk '{print $2}')

mv /srv/beats/chunes/$trac /srv/beats/old/

