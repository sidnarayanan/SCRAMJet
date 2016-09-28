#!/bin/bash

export SUBMIT_CFG="all_trans"
#export SUBMIT_CFG="test"
export SUBMIT_LOGDIR="/work/sidn/logs/v5/"
export SUBMIT_WORKDIR="/work/sidn/submit/"

mkdir -p $SUBMIT_LOGDIR
mkdir -p $SUBMIT_WORKDIR
rm -rf $SUBMIT_WORKDIR/*
