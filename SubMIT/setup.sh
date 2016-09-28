#!/bin/bash

export SUBMIT_CFG="all_trans"
#export SUBMIT_CFG="test"
export SUBMIT_LOGDIR="/work/sidn/logs/v5/"
export SUBMIT_WORKDIR="/work/sidn/submit/"
export SUBMIT_OUTDIR="/mnt/hadoop/cms/store/user/snarayan/scramjet/v5/batch/"

mkdir -p $SUBMIT_LOGDIR
mkdir -p $SUBMIT_WORKDIR
sed "s?XXXX?${SUBMIT_OUTDIR}?g" skim_tmpl.py > skim.py
chmod u+x skim.py
