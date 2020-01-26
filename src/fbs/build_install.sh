#!/bin/bash

# build python topics
flatc -p -o ./python/ *.fbs

# build c++ topics
flatc -c -o ./cpp/topics/ *.fbs

# 'install' to python directory
cp -r ./python/topics/ ../python/

# 'install' to cpp directory
cp -r ./cpp/topics/ ../cpp/
