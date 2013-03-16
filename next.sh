#!/bin/bash

set -ex

pids=($(ps ax |  grep "cat /srv/beats/chunes/live" | awk '{print $1}'))

kill ${pids[0]}
#killall -s SIGHUP pickrnd.sh

