#!/bin/bash
label=$1
cfgName=$2
nPerJob=$3
WD=${PWD}
pwd

executable=skimEfficient.py
#executable=skimBatch.py

scramdir=/afs/cern.ch/user/s/snarayan/work/private/scramjet/CMSSW_8_0_11/
cd ${scramdir}/src/
eval `scramv1 runtime -sh`
source SCRAMJet/TopTagging/setup.sh
cd $WD

cp ${scramdir}/src/SCRAMJet/TopTagging/config/${cfgName}.cfg local.cfg
cp ${scramdir}/src/SCRAMJet/TopTagging/skimming/$executable .

python $executable $label $nPerJob ${SCRAMJETFLAT}/batch/  

rm -rf *root *.py local.cfg data

exit 0
