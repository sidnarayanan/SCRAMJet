#!/bin/bash

cfgname=all_trans

WD=$PWD

cd $CMSSW_BASE/
tar -chvzf 8011.tgz biglib bin lib objs test external # h = --dereference symlinks
mv 8011.tgz $WD

cd ${CMSSW_BASE}/src/SCRAMJet/SubMIT
cp skim.py $WD
cp config/${cfgname}.cfg $WD/local.cfg

cd $WD 
tar -chzvf scramjet.tgz 8011.tgz local.cfg skim.py

