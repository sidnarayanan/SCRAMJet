#!/bin/bash

cfgname=$SUBMIT_CFG

WD=$PWD

cd $CMSSW_BASE/
tar -chzf 8011.tgz src python biglib bin lib objs test external # h = --dereference symlinks
mv 8011.tgz $WD

cd ${CMSSW_BASE}/src/SCRAMJet/SubMIT
cp skim.py $WD
cp config/${cfgname}.cfg $WD/local.cfg

voms-proxy-init -voms cms
cp /tmp/x509up_u$UID $WD/x509up

cd $WD 

inputs="8011.tgz local.cfg skim.py x509up"

tar -chzvf scramjet.tgz $inputs

rm $inputs

cp ${CMSSW_BASE}/src/SCRAMJet/SubMIT/exec.sh .
