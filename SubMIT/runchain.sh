#!/bin/bash

WD=$PWD

cd $SUBMIT_WORKDIR
sh ${WD}/makeTar.sh
cp ${WD}/exec.sh .
