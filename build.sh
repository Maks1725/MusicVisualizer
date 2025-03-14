#!/bin/bash

gcc -o musicvisualizer main.c -O3 -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
