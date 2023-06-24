#!/bin/bash

gcc lrustack.c cachesim.c -o cachesim
./cachesim $1 $2 $3 $4
