#!/bin/bash

if [[ -d _build ]];
  then
  rm -rf _build
fi;

if [[ -d bin ]];
  then
  rm -rf bin
fi;

if [[ -d lib ]];
  then
  rm -rf lib
fi;

if [[ -d libbin ]];
  then
  rm -rf libbin
fi;

if [[ -d include ]];
  then
  rm -rf include
fi;

echo "Project clean"
