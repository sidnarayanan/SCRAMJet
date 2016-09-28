#!/bin/bash

cfgname=$SUBMIT_CFG

WD=$PWD

cd $CMSSW_BASE/
echo "Tarring up CMSSW..."
tar -chzf 8011.tgz src python biglib bin lib objs test external # h = --dereference symlinks
mv 8011.tgz $WD

cd ${CMSSW_BASE}/src/SCRAMJet/SubMIT
cp skim.py $WD

voms-proxy-init -voms cms
cp /tmp/x509up_u$UID $WD/x509up

cd $WD 

inputs="8011.tgz skim.py x509up"

echo "Tarring up everything..."
tar -chzvf scramjet.tgz $inputs

rm $inputs

cp ${CMSSW_BASE}/src/SCRAMJet/SubMIT/exec.sh .
