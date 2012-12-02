#!/bin/bash

set -ex

pids=($(ps ax |  grep "cat ../chunes" | awk '{print $1}'))

kill ${pids[0]}

