#!/bin/bash

export SCRAMJET=${CMSSW_BASE}/src/SCRAMJet
export TOP=${SCRAMJET}/TopTagging

#location of production 
histDir=scramjet
export SCRAMJETHIST=${EOS}/$histDir

#location of flat trees
export SCRAMJETFLAT=${HOME}/home000/scramjet_v3/
mkdir -p $SCRAMJETFLAT/batch
