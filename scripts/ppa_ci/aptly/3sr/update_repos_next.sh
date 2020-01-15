#!/bin/bash

# All the next uploads of the packages


set -e

#for i in bionic
for i in bionic buster stretch xenial bullseye
do
	cd /home/anton/deb
	wget https://gitlab.com/api/v4/projects/10133144/jobs/artifacts/master/download?job=deb_$i -O yade.zip
	unzip yade.zip
	aptly repo remove yadedaily-$i 'yadedaily'
	aptly repo remove yadedaily-$i 'libyadedaily'
	aptly repo remove yadedaily-$i 'yadedaily-doc'
	aptly repo remove yadedaily-$i 'python3-yadedaily'
	aptly repo add yadedaily-$i deb/*.deb
	aptly repo add yadedaily-$i deb/*.dsc
	rm -rf /home/anton/deb/*
	aptly publish update $i filesystem:yadedaily:
done
