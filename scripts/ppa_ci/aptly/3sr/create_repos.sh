#!/bin/bash

set -e
#for i in bionic
for i in bionic buster stretch xenial bullseye
do
    aptly repo create -distribution=$i -component=main yadedaily-$i
done
