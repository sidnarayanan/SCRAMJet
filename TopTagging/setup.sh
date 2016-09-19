#!/bin/bash

export SCRAMJET=${CMSSW_BASE}/src/SCRAMJet
export TOP=${SCRAMJET}/TopTagging

#location of production 
histDir=scramjet
export SCRAMJETHIST=${EOS}/$histDir

#location of flat trees
export SCRAMJETFLAT=${HOME}/home000/scramjet_v4/
#export SCRAMJETFLAT=${HOME}/hadoop/scramjet_v4/
mkdir -p $SCRAMJETFLAT/batch
chmod 777 $SCRAMJETFLAT/batch
mkdir -p $SCRAMJETFLAT/training
